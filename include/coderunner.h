#ifndef __X_CODERUNNER_CODERUNNER_H__
#define __X_CODERUNNER_CODERUNNER_H__ 1

#include <unistd.h>
#include <stdint.h>
#include <time.h>

typedef struct _T_CodeRunInstance {
	time_t tstamp_start;
	time_t tstamp_bound;
	time_t tstamp_finish;

	time_t tstamp_lastcheck;

	time_t tstamp_onstop_sigint;
	time_t tstamp_onstop_sigterm;

	pid_t child_pid;

	int return_code;
	int stop_signal;

	int last_errno;

	int _life_cycle_status;

#if ENABLE_RUNTIMEENV_PRESERVE
	char *fullpath_working_directory;

	char *fullpath_datafile_stdin;
	char *fullpath_logfile_stdout;
	char *fullpath_logfile_stderr;

	uid_t runner_uid;
	gid_t runner_gid;
#endif	/* ENABLE_RUNTIMEENV_PRESERVE */
} CodeRunInstance;



int run_program(CodeRunInstance *instance, const char *filename, char *const argv[], char *const envp[], const char *working_directory, const char *run_as_user, const char *datafilename_stdin, const char *logfilename_stdout, const char *logfilename_stderr, uint32_t max_running_second,  uint32_t overtime_sigint_second, uint32_t overtime_sigterm_second);

int wait_program(CodeRunInstance *instance, int blocking_wait);

int stop_program(CodeRunInstance *instance);



#if ENABLE_RUNTIMEENV_PRESERVE

int adapt_preserved_runtime_environment(CodeRunInstance *instance);

int release_runtime_environment_preservation(CodeRunInstance *instance);

#endif	/* ENABLE_RUNTIMEENV_PRESERVE */



#endif	/* __X_CODERUNNER_CODERUNNER_H__ */

/*
vim: ts=4 sw=4 ai nowrap
*/
