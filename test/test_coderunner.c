#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <google/cmockery.h>

#include <string.h>
#include <sys/wait.h>


#include "coderunner.h"


char *const STRARRAY_SAMPLE_1[] = {
	"sample-1", NULL};


void run_program_w_empty_pointer(void **state, CodeRunInstance *instance)
{
	int ret;

	memset(instance, 7, sizeof(CodeRunInstance));

	ret = run_program(instance, NULL, STRARRAY_SAMPLE_1, STRARRAY_SAMPLE_1, "/", NULL, NULL, NULL, NULL, 10, 10, 10);

	assert_int_equal(ret, 0);
	assert_int_equal(instance->return_code, 0);
	assert_int_equal(instance->return_code, 0);
	assert_int_equal(instance->stop_signal, 0);
	assert_int_not_equal(instance->child_pid, 0);
}

void verify_program_w_empty_pointer(void **state, CodeRunInstance *instance)
{
	assert_int_equal(instance->return_code, 20);
	assert_int_equal(instance->stop_signal, -1);
	assert_int_not_equal(instance->tstamp_finish, 0);
}


void test_clear_instance(void **state)
{
	CodeRunInstance instance;

	run_program_w_empty_pointer(state, &instance);

	if(0 != instance.child_pid)
	{
		int prg_status;
		waitpid(instance.child_pid, &prg_status, 0);
	}
}

void test_clear_instance_blocking_wait(void **state)
{
	CodeRunInstance instance_obj;
	int retcode;

	run_program_w_empty_pointer(state, &instance_obj);

	retcode = wait_program(&instance_obj, 1);

	assert_int_equal(retcode, 0);

	verify_program_w_empty_pointer(state, &instance_obj);
}



int main(int argc, char* argv[])
{
	const UnitTest tests[] = {
		unit_test(test_clear_instance),
		unit_test(test_clear_instance_blocking_wait),
	};
	return run_tests(tests);
}



/*
vim: ts=4 sw=4 ai nowrap
*/
