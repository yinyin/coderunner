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

	int _life_cycle_status;
} CodeRunInstance;



#endif	/* __X_CODERUNNER_CODERUNNER_H__ */

/*
vim: ts=4 sw=4 ai nowrap
*/
