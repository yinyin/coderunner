#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>

#include <sys/types.h>
#include <sys/stat.h>


#include "coderunner.h"



#define RECORD_ERR(msg, srcfile, srcline) {	\
	int errnum;	\
	errnum = errno;	\
	instance->last_errno = errnum;	\
	fprintf(stderr, "ERR: %s: %s @[%s:%d]\n", msg, strerror(errnum), srcfile, srcline);	\
}



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


static int check_working_directory(CodeRunInstance *instance, const char *working_directory, char **p_fullpath_working_directory)
{
	char *p;
	if(NULL == (p = realpath(working_directory, NULL)))
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
		if(NULL == (p = realpath(datafilename_stdin, NULL)))
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
		if(NULL == (p = realpath(logfilename_stdout, NULL)))
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
		if(NULL == (p = realpath(logfilename_stderr, NULL)))
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
		*p_runner_uid = getuid();
		*p_runner_gid = getgid();
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

	if( ((time_t)(-1)) == time(&current_tstamp) )
	{
		RECORD_ERR("cannot get current timestamp", __FILE__, __LINE__);
		current_tstamp = (time_t)(0);
	}

	instance->tstamp_start = current_tstamp;
	instance->tstamp_bound = current_tstamp + ((time_t)(max_running_second));
	instance->tstamp_finish = (time_t)(0);

	instance->tstamp_lastcheck = current_tstamp;

	instance->overtime_sigint_second = overtime_sigint_second;
	instance->overtime_sigterm_second = overtime_sigterm_second;

	instance->child_pid = child_pid;

	return;
}


int run_program(CodeRunInstance *instance, const char *filename, char *const argv[], char *const envp[], const char *working_directory, const char *run_as_user, const char *datafilename_stdin, const char *logfilename_stdout, const char *logfilename_stderr, uint32_t max_running_second, uint32_t overtime_sigint_second, uint32_t overtime_sigterm_second, uint32_t error_skip)
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
	else if(0 == child_pid)
	{
		fill_instance_structure(instance, max_running_second, overtime_sigint_second, overtime_sigterm_second, child_pid);
		RELEASE_ALLOCATED_RESOURCE;
		return 0;
	}


	if(0 != chdir(working_directory))
	{
		RECORD_ERR("failed on changing work directory", __FILE__, __LINE__);
		return 1;
	}

#undef RELEASE_ALLOCATED_RESOURCE

	return -1;
}



/*
vim: ts=4 sw=4 ai nowrap
*/
