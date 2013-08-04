#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <limits.h>
#include <dirent.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>

#if UNIT_TESTING
#include <setjmp.h>
#include <google/cmockery.h>
#endif	/* UNIT_TESTING */

#include "coderunner.h"



#define RECORD_ERR(msg, srcfile, srcline) {	\
	int errnum;	\
	errnum = errno;	\
	instance->last_errno = errnum;	\
	fprintf(stderr, "ERR: %s: %s @[%s:%d]\n", msg, strerror(errnum), srcfile, srcline);	\
}


#define PROGRAM_LIFECYCLE_NORMAL 0
#define PROGRAM_LIFECYCLE_SIGINTED 1
#define PROGRAM_LIFECYCLE_SIGTERMED 2
#define PROGRAM_LIFECYCLE_SIGKILLED 3



static void release_allocated_memory(char *fullpath_working_directory, char *fullpath_datafile_stdin, char *fullpath_logfile_stdout, char *fullpath_logfile_stderr)
{
#define FREE_ALLOCATED_MEMORY(p) { if(NULL != p) { free(p); } }
	FREE_ALLOCATED_MEMORY(fullpath_working_directory);
	FREE_ALLOCATED_MEMORY(fullpath_datafile_stdin);
	FREE_ALLOCATED_MEMORY(fullpath_logfile_stdout);
	FREE_ALLOCATED_MEMORY(fullpath_logfile_stderr);
#undef FREE_ALLOCATED_MEMORY

	return;
}


static char *x_realpath(const char *path)
{
	char resolved_path[PATH_MAX];
	char *result;

	if(NULL == realpath(path, resolved_path))
	{ return NULL; }

	resolved_path[PATH_MAX-1] = '\0';

	if(NULL == (result = strdup(resolved_path)))
	{ return NULL; }

	return result;
}


static time_t get_current_tstamp(CodeRunInstance *instance)
{
	time_t current_tstamp;

	if( ((time_t)(-1)) == time(&current_tstamp) )
	{
		RECORD_ERR("cannot get current timestamp", __FILE__, __LINE__);
		return (time_t)(0);
	}

	return current_tstamp;
}


static int check_working_directory(CodeRunInstance *instance, const char *working_directory, char **p_fullpath_working_directory)
{
	char *p;
	if(NULL == (p = x_realpath(working_directory)))
	{ return 1; }

	*p_fullpath_working_directory = p;

	return 0;
}


static int prepare_log_files(CodeRunInstance *instance, const char *datafilename_stdin, const char *logfilename_stdout, const char *logfilename_stderr, char **p_fullpath_datafile_stdin, char **p_fullpath_logfile_stdout, char **p_fullpath_logfile_stderr)
{
	int fd;
	char *p;

	if(NULL != datafilename_stdin)
	{
		if(-1 == (fd = open(datafilename_stdin, O_RDONLY)))
		{
			RECORD_ERR("failed on attempting to open STDIN file", __FILE__, __LINE__);
			return 1;
		}
		if(-1 == close(fd))
		{
			RECORD_ERR("failed on attempting to close STDIN file", __FILE__, __LINE__);
			return 2;
		}
		if(NULL == (p = x_realpath(datafilename_stdin)))
		{
			RECORD_ERR("failed on getting path of STDIN file", __FILE__, __LINE__);
			return 3;
		}
		*p_fullpath_datafile_stdin = p;
	}
	else
	{ *p_fullpath_datafile_stdin = NULL; }

	if(NULL != logfilename_stdout)
	{
		if(-1 == (fd = open(logfilename_stdout, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)))
		{
			RECORD_ERR("failed on attempting to open STDOUT file", __FILE__, __LINE__);
			return 11;
		}
		if(-1 == close(fd))
		{
			RECORD_ERR("failed on attempting to close STDOUT file", __FILE__, __LINE__);
			return 12;
		}
		if(NULL == (p = x_realpath(logfilename_stdout)))
		{
			RECORD_ERR("failed on getting path of STDOUT file", __FILE__, __LINE__);
			return 13;
		}
		*p_fullpath_logfile_stdout = p;
	}
	else
	{ *p_fullpath_logfile_stdout = NULL; }

	if(NULL != logfilename_stderr)
	{
		if( (NULL == logfilename_stdout) || (0 != strcmp(logfilename_stdout, logfilename_stderr)) )
		{
			if(-1 == (fd = open(logfilename_stderr, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)))
			{
				RECORD_ERR("failed on attempting to open STDERR file", __FILE__, __LINE__);
				return 21;
			}
			if(-1 == close(fd))
			{
				RECORD_ERR("failed on attempting to close STDERR file", __FILE__, __LINE__);
				return 22;
			}
		}
		if(NULL == (p = x_realpath(logfilename_stderr)))
		{
			RECORD_ERR("failed on getting path of STDERR file", __FILE__, __LINE__);
			return 23;
		}
		*p_fullpath_logfile_stderr = p;
	}
	else
	{ *p_fullpath_logfile_stderr = NULL; }

	return 0;
}


static int lookup_runner_account(CodeRunInstance *instance, const char *run_as_user, uid_t *p_runner_uid, gid_t *p_runner_gid)
{
	struct passwd *p;

	if(NULL == run_as_user)
	{
		*p_runner_uid = geteuid();
		*p_runner_gid = getegid();
		return 0;
	}

	if(NULL == (p = getpwnam(run_as_user)))
	{
		RECORD_ERR("cannot found subject runner account", __FILE__, __LINE__);
		return 1;
	}

	*p_runner_uid = p->pw_uid;
	*p_runner_gid = p->pw_gid;
	return 0;
}


static void fill_instance_structure(CodeRunInstance *instance, uint32_t max_running_second,  uint32_t overtime_sigint_second, uint32_t overtime_sigterm_second, pid_t child_pid)
{
	time_t current_tstamp;

	current_tstamp = get_current_tstamp(instance);

	instance->tstamp_start = current_tstamp;
	instance->tstamp_bound = current_tstamp + ((time_t)(max_running_second));
	instance->tstamp_finish = (time_t)(0);

	instance->tstamp_lastcheck = current_tstamp;

	instance->tstamp_onstop_sigint = current_tstamp + ((time_t)(overtime_sigint_second));
	instance->tstamp_onstop_sigterm = current_tstamp + ((time_t)(overtime_sigterm_second));

	instance->child_pid = child_pid;

	return;
}

static int set_file_owner(CodeRunInstance *instance, int fd, uid_t runner_uid, gid_t runner_gid, const char *errmsg)
{
	if( (runner_uid == geteuid()) && (runner_gid == getegid()) )
	{ return 0; }

	if(-1 == fchown(fd, runner_uid, runner_gid))
	{
		RECORD_ERR(errmsg, __FILE__, __LINE__);
	}

	return 0;
}

static int open_log_files(CodeRunInstance *instance, const char *fullpath_datafilename_stdin, const char *fullpath_logfilename_stdout, const char *fullpath_logfilename_stderr, uid_t runner_uid, gid_t runner_gid)
{
	int fd;

	if(NULL != fullpath_datafilename_stdin)
	{
		if(-1 == (fd = open(fullpath_datafilename_stdin, O_RDONLY)))
		{
			RECORD_ERR("failed on open STDIN file", __FILE__, __LINE__);
			return 1;
		}
		if(STDIN_FILENO != fd)
		{
			if(STDIN_FILENO != dup2(fd, STDIN_FILENO))
			{
				RECORD_ERR("failed on dup file descriptor to STDIN", __FILE__, __LINE__);
				return 2;
			}
			if(-1 == close(fd))
			{
				RECORD_ERR("failed on close user STDIN file descriptor", __FILE__, __LINE__);
				return 3;
			}
		}
	}

	if(NULL != fullpath_logfilename_stdout)
	{
		if(-1 == (fd = open(fullpath_logfilename_stdout, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)))
		{
			RECORD_ERR("failed on open STDOUT file", __FILE__, __LINE__);
			return 11;
		}
		if(0 != set_file_owner(instance, fd, runner_uid, runner_gid, "cannot change file owner for STDOUT file"))
		{ return 15; }
		if(STDOUT_FILENO != fd)
		{
			if(STDOUT_FILENO != dup2(fd, STDOUT_FILENO))
			{
				RECORD_ERR("failed on dup file descriptor to STDOUT", __FILE__, __LINE__);
				return 12;
			}
			if(-1 == close(fd))
			{
				RECORD_ERR("failed on close user STDOUT file descriptor", __FILE__, __LINE__);
				return 13;
			}
		}
	}

	if(NULL != fullpath_logfilename_stderr)
	{
		if( (NULL != fullpath_logfilename_stdout) && (0 == strcmp(fullpath_logfilename_stdout, fullpath_logfilename_stderr)) )
		{
			if(STDERR_FILENO != dup2(STDOUT_FILENO, STDERR_FILENO))
			{
				RECORD_ERR("failed on dup STDOUT to STDERR", __FILE__, __LINE__);
				return 24;
			}
		}
		else
		{
			if(-1 == (fd = open(fullpath_logfilename_stderr, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)))
			{
				RECORD_ERR("failed on open STDERR file", __FILE__, __LINE__);
				return 21;
			}
			if(0 != set_file_owner(instance, fd, runner_uid, runner_gid, "cannot change file owner for STDERR file"))
			{ return 25; }
			if(STDERR_FILENO != fd)
			{
				if(STDERR_FILENO != dup2(fd, STDERR_FILENO))
				{
					RECORD_ERR("failed on dup file descriptor to STDERR", __FILE__, __LINE__);
					return 22;
				}
				if(-1 == close(fd))
				{
					RECORD_ERR("failed on close user STDERR file descriptor", __FILE__, __LINE__);
					return 23;
				}
			}
		}
	}

	return 0;
}


static int close_fd_impl_procfs_fdfolder(CodeRunInstance *instance, const char *fdfolder_path)
{
	int dirfd_val;
	DIR *dirp;
	struct dirent *p;

	if(NULL == (dirp = opendir(fdfolder_path)))
	{
		RECORD_ERR("cannot open file descriptor proc list", __FILE__, __LINE__);
		return 1;
	}

	dirfd_val = dirfd(dirp);

	while(NULL != (p = readdir(dirp))) {
		int fd_val;
		char *endp;
		fd_val = (int)(strtol(p->d_name, &endp, 10));
		if( (p->d_name != endp) && ('\0' == *endp) && (fd_val != dirfd_val) && (fd_val > 2) )
		{
			if(0 != close(fd_val))
			{
				char buf[64];
				snprintf(buf, 63, "failed on close file descriptor (fd=%d)", fd_val);
				buf[63] = '\0';
				RECORD_ERR(buf, __FILE__, __LINE__);
			}
		}
	}

	if(0 != closedir(dirp))
	{
		RECORD_ERR("failed on close procfs file descriptor folder", __FILE__, __LINE__);
	}

	return 0;
}

static int close_fd_impl_way_to_max()
{
	int max_fd;
	int fd;

	max_fd = (int)(sysconf(_SC_OPEN_MAX));
	for(fd = 3; fd < max_fd; fd++) {
		close(fd);
	}

	return 0;
}

static void close_fd(CodeRunInstance *instance)
{
	int ret;

	ret = -1;

#if __APPLE__
	ret = close_fd_impl_procfs_fdfolder(instance, "/dev/fd");
#elif __linux__ || __linux
	ret = close_fd_impl_procfs_fdfolder(instance, "/proc/self/fd");
#endif

	if(0 != ret)
	{
		close_fd_impl_way_to_max();
	}

	return;
}


static int change_account(CodeRunInstance *instance, uid_t runner_uid, gid_t runner_gid)
{
	if(0 != setgid(runner_gid))
	{
		RECORD_ERR("cannot set Group ID", __FILE__, __LINE__);
		return 1;
	}
	if(0 != setegid(runner_gid))
	{
		RECORD_ERR("cannot set Effective Group ID", __FILE__, __LINE__);
		return 2;
	}
	if(0 != setuid(runner_uid))
	{
		RECORD_ERR("cannot set User ID", __FILE__, __LINE__);
		return 3;
	}
	if(0 != seteuid(runner_uid))
	{
		RECORD_ERR("cannot set Effective User ID", __FILE__, __LINE__);
		return 4;
	}

	return 0;
}


int run_program(CodeRunInstance *instance, const char *filename, char *const argv[], char *const envp[], const char *working_directory, const char *run_as_user, const char *datafilename_stdin, const char *logfilename_stdout, const char *logfilename_stderr, uint32_t max_running_second, uint32_t overtime_sigint_second, uint32_t overtime_sigterm_second)
{
	char *fullpath_working_directory;

	char *fullpath_datafile_stdin;
	char *fullpath_logfile_stdout;
	char *fullpath_logfile_stderr;

	uid_t runner_uid;
	gid_t runner_gid;

	pid_t child_pid;


	fullpath_working_directory = NULL;
	fullpath_datafile_stdin = NULL;
	fullpath_logfile_stdout = NULL;
	fullpath_logfile_stderr = NULL;

#define RELEASE_ALLOCATED_RESOURCE {	\
	release_allocated_memory(fullpath_working_directory, fullpath_datafile_stdin, fullpath_logfile_stdout, fullpath_logfile_stderr);	\
}


	memset(instance, 0, sizeof(CodeRunInstance));

	if(0 != check_working_directory(instance, working_directory, &fullpath_working_directory))
	{
		RECORD_ERR("cannot have real path of given working directory", __FILE__, __LINE__);
		RELEASE_ALLOCATED_RESOURCE;
		return 1;
	}

	if(0 != prepare_log_files(instance, datafilename_stdin, logfilename_stdout, logfilename_stderr, &fullpath_datafile_stdin, &fullpath_logfile_stdout, &fullpath_logfile_stderr))
	{
		RELEASE_ALLOCATED_RESOURCE;
		return 2;
	}

	if(0 != lookup_runner_account(instance, run_as_user, &runner_uid, &runner_gid))
	{
		RELEASE_ALLOCATED_RESOURCE;
		return 3;
	}

	child_pid = fork();
	if(-1 == child_pid)
	{
		RECORD_ERR("failed on perform fork()", __FILE__, __LINE__);
		RELEASE_ALLOCATED_RESOURCE;
		return 4;
	}
	else if(0 != child_pid)
	{
		fill_instance_structure(instance, max_running_second, overtime_sigint_second, overtime_sigterm_second, child_pid);
		RELEASE_ALLOCATED_RESOURCE;
		return 0;
	}

	/* {{{ child process code */

	if(0 != setpgid(0, 0))
	{
		RECORD_ERR("failed on creating independent process group", __FILE__, __LINE__);
	}

	if(0 != chdir(working_directory))
	{
		RECORD_ERR("failed on changing work directory", __FILE__, __LINE__);
		exit(17);
		return 1;
	}

	if(0 != open_log_files(instance, fullpath_datafile_stdin, fullpath_logfile_stdout, fullpath_logfile_stderr, runner_uid, runner_gid))
	{
		exit(18);
		return 2;
	}

	close_fd(instance);

	if( (NULL != run_as_user) && (0 != change_account(instance, runner_uid, runner_gid)) )
	{
		exit(19);
		return 3;
	}

	fprintf(stdout, "PID: %d\n", (int)(getpid()));
	fprintf(stdout, "WorkDirectory: [%s]\n", fullpath_working_directory);
	fprintf(stdout, "Runner: UID=%d; GID=%d\n", (int)(runner_uid), (int)(runner_gid));
	fprintf(stdout, "----------------\n");

	execve(filename, argv, envp);
	RECORD_ERR("cannot execute target program", __FILE__, __LINE__);
	exit(20);

	/* }}} child process code */

#undef RELEASE_ALLOCATED_RESOURCE

	return -1;
}



static time_t update_lastcheck_tstamp(CodeRunInstance *instance)
{
	time_t current_tstamp;

	if( (time_t)(0) != (current_tstamp = get_current_tstamp(instance)) )
	{
		instance->tstamp_lastcheck = current_tstamp;
	}

	return current_tstamp;
}


static void stop_program_impl(CodeRunInstance *instance, time_t current_tstamp)
{
	if( (PROGRAM_LIFECYCLE_NORMAL == instance->_life_cycle_status) || (current_tstamp < instance->tstamp_onstop_sigint) )
	{
		instance->_life_cycle_status = PROGRAM_LIFECYCLE_SIGINTED;
		if(0 != killpg(instance->child_pid, SIGINT))
		{
			RECORD_ERR("cannot send SIGINT to child process", __FILE__, __LINE__);
			return 1;
		}
	}
	else if( (PROGRAM_LIFECYCLE_SIGINTED == instance->_life_cycle_status) || (current_tstamp < instance->tstamp_onstop_sigterm) )
	{
		instance->_life_cycle_status = PROGRAM_LIFECYCLE_SIGTERMED;
		if(0 != killpg(instance->child_pid, SIGTERM))
		{
			RECORD_ERR("cannot send SIGTERM to child process", __FILE__, __LINE__);
			return 2;
		}
	}
	else
	{
		instance->_life_cycle_status = PROGRAM_LIFECYCLE_SIGKILLED;
		if(0 != killpg(instance->child_pid, SIGKILL))
		{
			RECORD_ERR("cannot send SIGKILL to child process", __FILE__, __LINE__);
			return 3;
		}
	}

	return 0;
}

static void update_exit_code(CodeRunInstance *instance, int prg_exitcode)
{
	if(WIFEXITED(prg_exitcode))
	{
		instance->return_code = WEXITSTATUS(prg_exitcode);
		instance->stop_signal = -1;
	}
	else
	{
		instance->return_code = -1;
		instance->stop_signal = WTERMSIG(prg_exitcode);
	}
}

int wait_program(CodeRunInstance *instance, int blocking_wait)
{
	time_t current_tstamp;
	int retcode;
	int prg_status;

	current_tstamp = update_lastcheck_tstamp(instance);

	if( -1 == (retcode = waitpid(instance->child_pid, &prg_status, ((0 == blocking_wait) ? WNOHANG : 0))) )
	{
		RECORD_ERR("failed on waitpid", __FILE__, __LINE__);
		return 2;
	}

	if(0 == retcode)
	{
		if(current_tstamp > instance->tstamp_bound)
		{
			stop_program_impl(instance, current_tstamp);
		}
		return 1;
	}

	update_exit_code(instance, prg_status);

	return 0;
}



/*
vim: ts=4 sw=4 ai nowrap
*/
