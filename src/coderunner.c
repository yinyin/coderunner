#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>



#define RECORD_ERR(msg) {	\
	int errnum;	\
	errnum = errno;	\
	instance->last_errno = errnum;	\
	fprintf(stderr, "ERR: %s: %s", msg, strerror(errnum));	\
}


static int prepare_log_files(const char *datafilename_stdin, const char *logfilename_stdout, const char *logfilename_stderr)
{
	int fd;

	if(NULL != datafilename_stdin)
	{
		if(-1 == (fd = open(datafilename_stdin, O_RDONLY)))
		{
			RECORD_ERR("failed on attempting to open STDIN file");
			return 1;
		}
		if(-1 == close(fd))
		{
			RECORD_ERR("failed on attempting to close STDIN file");
			return 2;
		}
	}

	if(NULL != logfilename_stdout)
	{
		if(-1 == (fd = open(logfilename_stdout, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)))
		{
			RECORD_ERR("failed on attempting to open STDOUT file");
			return 11;
		}
		if(-1 == close(fd))
		{
			RECORD_ERR("failed on attempting to close STDOUT file");
			return 12;
		}
	}

	if( (NULL != logfilename_stderr) && ( (NULL == logfilename_stdout) || (0 != strcmp(logfilename_stdout, logfilename_stderr)) ) )
	{
		if(-1 == (fd = open(logfilename_stderr, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP)))
		{
			RECORD_ERR("failed on attempting to open STDERR file");
			return 21;
		}
		if(-1 == close(fd))
		{
			RECORD_ERR("failed on attempting to close STDERR file");
			return 22;
		}
	}

	return 0;
}

int run_program(CodeRunInstance *instance, const char *filename, char *const argv[], char *const envp[], const char *working_directory, const char *run_as_user, const char *datafilename_stdin, const char *logfilename_stdout, const char *logfilename_stderr, uint32_t max_running_second,  uint32_t overtime_sigint_second, uint32_t overtime_sigterm_second, uint32_t error_skip)
{
	memset(instance, 0, sizeof(CodeRunInstance));

	if(0 != working_directory(working_directory))
	{
		RECORD_ERR("failed on changing work directory");
		return 1;
	}

	if(0 != prepare_log_files(datafilename_stdin, logfilename_stdout, logfilename_stderr))
	{ return 2; }

	return -1;
}



/*
vim: ts=4 sw=4 ai nowrap
*/
