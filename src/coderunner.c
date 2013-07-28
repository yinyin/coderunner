#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>


#define RECORD_ERR(msg) {	\
	int errnum;	\
	errnum = errno;	\
	instance->last_errno = errnum;	\
	fprintf(stderr, "ERR: %s: %s", msg, strerror(errnum));	\
}

int run_program(CodeRunInstance *instance, const char *filename, char *const argv[], char *const envp[], const char *working_directory, const char *run_as_user, const char *datafilename_stdin, const char *logfilename_stdout, const char *logfilename_stderr, uint32_t max_running_second,  uint32_t overtime_sigint_second, uint32_t overtime_sigterm_second)
{
	memset(instance, 0, sizeof(CodeRunInstance));

	if(0 != working_directory(working_directory))
	{
		RECORD_ERR("failed on changing work directory");
		return 1;
	}

	return -1;
}



/*
vim: ts=4 sw=4 ai nowrap
*/
