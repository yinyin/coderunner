#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <google/cmockery.h>

#include <string.h>

#include "coderunner.h"


char *const STRARRAY_SAMPLE_1[] = {
	"sample-1", NULL};

void test_clear_instance(void **state)
{
	CodeRunInstance instance;
	int ret;

	memset(&instance, 7, sizeof(CodeRunInstance));

	ret = run_program(&instance, NULL, STRARRAY_SAMPLE_1, STRARRAY_SAMPLE_1, "/", NULL, NULL, NULL, NULL, 10, 10, 10, 0);

	assert_int_equal(ret, 0);
	assert_int_equal(instance.return_code, 0);
	assert_int_equal(instance.return_code, 0);
	assert_int_equal(instance.stop_signal, 0);
}



int main(int argc, char* argv[])
{
	const UnitTest tests[] = {
		unit_test(test_clear_instance),
	};
	return run_tests(tests);
}



/*
vim: ts=4 sw=4 ai nowrap
*/
