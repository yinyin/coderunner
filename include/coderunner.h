#ifndef __X_CODERUNNER_CODERUNNER_H__
#define __X_CODERUNNER_CODERUNNER_H__ 1

#include <unistd.h>
#include <stdint.h>

typedef struct _T_CodeRunInstance {
	time_t tstamp_start;
	time_t tstamp_bound;
	time_t tstamp_finish;

	time_t tstamp_lastcheck;

	uint32_t overtime_sigint_second;
	uint32_t overtime_sigterm_second;

	pid_t child_pid;

	int return_code;
	int stop_signal;

	int last_errno;

	int _life_cycle_status;
} CodeRunInstance;



int run_program(CodeRunInstance *instance, const char *filename, char *const argv[], char *const envp[], const char *working_directory, const char *run_as_user, const char *datafilename_stdin, const char *logfilename_stdout, const char *logfilename_stderr, uint32_t max_running_second,  uint32_t overtime_sigint_second, uint32_t overtime_sigterm_second);



#endif	/* __X_CODERUNNER_CODERUNNER_H__ */

/*
vim: ts=4 sw=4 ai nowrap
*/
