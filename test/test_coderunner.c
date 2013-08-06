#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <google/cmockery.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>



#include "coderunner.h"


char *const STRARRAY_SAMPLE_1[] = {
	"sample-1", NULL};

char *const STRARRAY_TD1_ARG_1[] = {
	"td1-x", "N5", "Hello", NULL};
char *const STRARRAY_TD1_ENV_1[] = {
	"ENV1=environment 1", "ENV2=environment 2", NULL};


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


void _ln_s(char *srcexe, char *targetpath)
{
	char buf[2048];

	if(NULL == getcwd(buf, 2048))
	{ perror("cannot get working directory"); }

	strncat(buf, "/", 64);
	strncat(buf, srcexe, 64);

	if(-1 == symlink(buf, targetpath))
	{ fprintf(stderr, "WARN: cannot make symlink: %s @[%s:%d]\n", strerror(errno), __FILE__, __LINE__); }
}

char * seek_stdout_content(char *p)
{
	int c;
	c = 0;
	while('\0' != *p) {
		if('-' == *p)
		{ c++; }
		else if((16 == c) && ('\n' == *p))
		{ return (p+1); }
		else
		{ c = 0; }
		p++;
	}
	return NULL;
}

void run_program_w_td1_1(void **state, CodeRunInstance *instance)
{
	int ret;

	memset(instance, 7, sizeof(CodeRunInstance));

	_ln_s("test/td1", "/tmp/td1");

	ret = run_program(instance, "td1", STRARRAY_TD1_ARG_1, STRARRAY_TD1_ENV_1, "/tmp", NULL, NULL, "/tmp/t1_stdout.txt", "/tmp/t1_stderr.txt", 10, 10, 10);

	assert_int_equal(ret, 0);
	assert_int_equal(instance->return_code, 0);
	assert_int_equal(instance->return_code, 0);
	assert_int_equal(instance->stop_signal, 0);
	assert_int_not_equal(instance->child_pid, 0);
}

void verify_program_w_td1_1(void **state, CodeRunInstance *instance)
{
	int fd;
	char buf[1024];
	int readed;

	assert_int_equal(instance->return_code, 0);
	assert_int_equal(instance->stop_signal, -1);
	assert_int_not_equal(instance->tstamp_finish, 0);

	if(-1 == (fd = open("/tmp/t1_stdout.txt", O_RDONLY)))
	{ perror("cannot open redirected STDOUT file."); }
	if(-1 == (readed = read(fd, buf, 1024)))
	{ perror("cannot read from STDOUT dump file."); }
	close(fd);
	buf[readed] = '\0';

	assert_string_equal(seek_stdout_content(buf), "A: N5\nA: Hello\nNPRT: 5\nE1: environment 1\nE2: environment 2\n");

	if(-1 == (fd = open("/tmp/t1_stderr.txt", O_RDONLY)))
	{ perror("cannot open redirected STDERR file."); }
	if(-1 == (readed = read(fd, buf, 1024)))
	{ perror("cannot read from STDERR dump file."); }
	close(fd);
	buf[readed] = '\0';

	assert_string_equal(buf, "Output to STDERR.\n");
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

void test_clear_instance_nonblocking_wait(void **state)
{
	CodeRunInstance instance_obj;
	CodeRunInstance *instance;
	int retcode;

	instance = &instance_obj;

	run_program_w_empty_pointer(state, &instance_obj);

	while(0 != (retcode = wait_program(&instance_obj, 0))) {
		assert_int_equal(retcode, 1);
		assert_int_equal(instance->return_code, 0);
		assert_int_equal(instance->stop_signal, 0);
		assert_int_equal(instance->tstamp_finish, 0);
		sleep(1);
	}

	verify_program_w_empty_pointer(state, &instance_obj);
}



void test_redirect_stdout_stderr(void **state)
{
	CodeRunInstance instance_obj;
	int retcode;

	run_program_w_td1_1(state, &instance_obj);

	retcode = wait_program(&instance_obj, 1);

	assert_int_equal(retcode, 0);

	verify_program_w_td1_1(state, &instance_obj);
}



int main(int argc, char* argv[])
{
	const UnitTest tests[] = {
		unit_test(test_clear_instance),
		unit_test(test_clear_instance_blocking_wait),
		unit_test(test_clear_instance_nonblocking_wait),
		unit_test(test_redirect_stdout_stderr),
	};
	return run_tests(tests);
}



/*
vim: ts=4 sw=4 ai nowrap
*/
