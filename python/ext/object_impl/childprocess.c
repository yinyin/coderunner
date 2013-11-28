/** implementation of methods belong to ChildProcess
 *
 * this code is intend to build with pycoderunner.c, should not build alone.
 * */
#ifndef ____CODE_INDICATOR_05a04832113c7891f7e5dffd4ef5a9a396a25058
	#error "This code is not intent to build alone.\n"
#endif

#define MAX_ARGUMENT_BUFFER_SIZE 8192
#define MAX_ARGUMENT_COUNT (511+1)
#define MAX_ENVIRONMENT_BUFFER_SIZE 8192
#define MAX_ENVIRONMENT_COUNT (511+1)
#define EXTERNAL_STRING_LENGTH_GUARD_MASK 0x7FFF
/* EXTERNAL_STRING_LENGTH_GUARD_MASK will limit the length of every arguments from command line and
 * length of name and value of environment variable. 0x7FFF = 32767 should large enough.
** */

#define CHILDPROCESS_BUFFER_ALLIGN 4
#define CHILDPROCESS_BUFFER_ALLIGN_MASK (CHILDPROCESS_BUFFER_ALLIGN - 1)


/* {{{ forward declares */
static PyObject * ChildProcess_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static void ChildProcess_dealloc(coderunner_ChildProcessObject *self);
static int ChildProcess_init(coderunner_ChildProcessObject *self, PyObject *args, PyObject *kwds);

static PyObject * ChildProcess_wait_blocking(coderunner_ChildProcessObject *self);
static PyObject * ChildProcess_wait_nonblocking(coderunner_ChildProcessObject *self);
static PyObject * ChildProcess_stop(coderunner_ChildProcessObject *self);
static PyObject * ChildProcess_get_result(coderunner_ChildProcessObject *self);
/* }}} forward declares */


static int _ChildProcess_prepare_arg(char *arg_buffer, char *arg_v[], PyObject *arg_obj)
{
	PyObject *iterator;
	PyObject *item;
	char *v_current_ptr;
	int v_count;
	int v_remain;

	if(NULL == arg_obj)
	{
		_report_error(PyExc_TypeError, "ERR: given \"args\" parameter is empty. @[%s:%d]", arg_obj, __FILE__, __LINE__);
		return -1;
	}

	if(NULL == (iterator = PyObject_GetIter(arg_obj)))
	{
		_report_error(PyExc_TypeError, "ERR: failed on getting iterator from \"args\". @[%s:%d]", __FILE__, __LINE__);
		return -2;
	}

	v_current_ptr = arg_buffer;
	v_count = 0;
	v_remain = MAX_ARGUMENT_BUFFER_SIZE;
	while( (v_remain > 0) && (v_count < (MAX_ARGUMENT_COUNT-1)) && (item = PyIter_Next(iterator)) ) {
		char *p;
		if(NULL == (p = PyString_AsString(item)))
		{
			_report_error(NULL, "ERR: cannot get string content from %d-th item of \"args\". @[%s:%d]", v_count, __FILE__, __LINE__);
			Py_DECREF(item);
			Py_DECREF(iterator);
			return -3;
		}

		{
			int l_arg;
			int l_total;

			l_arg = (int)(strlen(p) & EXTERNAL_STRING_LENGTH_GUARD_MASK);
			l_total = l_arg + 1;

			if(v_remain < l_total)
			{
				_report_error(PyExc_MemoryError, "ERR: insufficient memory for loading \"args\". @[%s:%d]", __FILE__, __LINE__);
				Py_DECREF(item);
				Py_DECREF(iterator);
				return -4;
			}

			strncpy(v_current_ptr, p, l_total);
			arg_v[v_count] = v_current_ptr;

			l_total = (0 != (l_total & CHILDPROCESS_BUFFER_ALLIGN_MASK)) ? (
					l_total + (CHILDPROCESS_BUFFER_ALLIGN - (l_total & CHILDPROCESS_BUFFER_ALLIGN_MASK))
				) : l_total;

			v_current_ptr += l_total;
			v_remain -= l_total;
		}

		v_count++;
		Py_DECREF(item);
	}

	arg_v[v_count] = NULL;

	Py_DECREF(iterator);

	return 0;
}

static int _ChildProcess_prepare_env(char *env_buffer, char *env_p[], PyObject *env_obj)
{
	PyObject *mapped_items;
	PyObject *iterator;
	PyObject *item;
	char *v_current_ptr;
	int v_count;
	int v_remain;

	if( (NULL == env_obj) || (!PyMapping_Check(env_obj)) )
	{
		strncpy(env_buffer, "EMPTY_ENV=1", 16);
		env_p[0] = env_buffer;
		env_p[1] = NULL;
		#if DUMP_DEBUG_MSG
			fprintf(stderr, "INFO: with empty environment variables. @[%s:%d]\n", __FILE__, __LINE__);
		#endif
		return 0;
	}

	if(NULL == (mapped_items = PyMapping_Items(env_obj)))
	{
		_report_error(PyExc_TypeError, "ERR: failed on getting mapped item from \"env\". @[%s:%d]", __FILE__, __LINE__);
		return -2;
	}

	if(NULL == (iterator = PyObject_GetIter(mapped_items)))
	{
		_report_error(PyExc_TypeError, "ERR: failed on getting iterator from \"env\". @[%s:%d]", __FILE__, __LINE__);
		Py_DECREF(mapped_items);
		return -3;
	}

	v_current_ptr = env_buffer;
	v_count = 0;
	v_remain = MAX_ENVIRONMENT_BUFFER_SIZE;
	while( (v_remain > 0) && (v_count < (MAX_ENVIRONMENT_COUNT-1)) && (item = PyIter_Next(iterator)) ) {
		PyObject *env_name_obj;
		PyObject *env_value_obj;
		char *env_name;
		char *env_value;

		if(NULL != (env_name_obj = PyTuple_GetItem(item, 0)))
		{
			if(NULL == (env_name = PyString_AsString(env_name_obj)))
			{
				_report_error(NULL, "ERR: cannot get environment name content from %d-th pair of \"env\". @[%s:%d]", v_count, __FILE__, __LINE__);
				Py_DECREF(item);
				Py_DECREF(iterator);
				Py_DECREF(mapped_items);
				return -5;
			}
		}
		else
		{
			_report_error(NULL, "ERR: cannot get environment name object from %d-th pair of \"env\". @[%s:%d]\n", v_count, __FILE__, __LINE__);
			Py_DECREF(item);
			Py_DECREF(iterator);
			Py_DECREF(mapped_items);
			return -4;
		}

		env_value = NULL;
		if(NULL != (env_value_obj = PyTuple_GetItem(item, 1)))
		{
			if(NULL == (env_value = PyString_AsString(env_value_obj)))
			{
				_report_error(NULL, "ERR: cannot get environment value content from %d-th pair of \"env\". @[%s:%d]\n", v_count, __FILE__, __LINE__);
				Py_DECREF(item);
				Py_DECREF(iterator);
				Py_DECREF(mapped_items);
				return -6;
			}
		}

		{
			int l_name;
			int l_value;
			int l_total;

			if(NULL == env_value)
			{
				l_name = (int)(strlen(env_name) & EXTERNAL_STRING_LENGTH_GUARD_MASK);
				l_value = 0;
				l_total = l_name + 1;
			}
			else
			{
				l_name = (int)(strlen(env_name) & EXTERNAL_STRING_LENGTH_GUARD_MASK);
				l_value = (int)(strlen(env_value) & EXTERNAL_STRING_LENGTH_GUARD_MASK);
				l_total = l_name + l_value + 2;
			}

			if(v_remain < l_total)
			{
				_report_error(PyExc_MemoryError, "ERR: insufficient memory for loading \"env\". @[%s:%d]", __FILE__, __LINE__);
				Py_DECREF(item);
				Py_DECREF(iterator);
				Py_DECREF(mapped_items);
				return -7;
			}

			if(NULL == env_value)
			{
				strncpy(v_current_ptr, env_name, l_name);
				v_current_ptr[l_name] = '\0';
			}
			else
			{
				strncpy(v_current_ptr, env_name, l_name);
				v_current_ptr[l_name] = '=';
				strncpy(v_current_ptr+l_name+1, env_value, l_value);
				v_current_ptr[l_total-1] = '\0';
			}

			env_p[v_count] = v_current_ptr;

			l_total = (0 != (l_total & CHILDPROCESS_BUFFER_ALLIGN_MASK)) ? (
					l_total + (CHILDPROCESS_BUFFER_ALLIGN - (l_total & CHILDPROCESS_BUFFER_ALLIGN_MASK))
				) : l_total;

			v_current_ptr += l_total;
			v_remain -= l_total;
		}

		v_count++;
		Py_DECREF(item);
	}

	env_p[v_count] = NULL;

	Py_DECREF(iterator);
	Py_DECREF(mapped_items);

	return 0;
}


static PyObject * ChildProcess_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	coderunner_ChildProcessObject *self;

	#if DUMP_DEBUG_MSG
		fprintf(stderr, "INFO: new ChildProcess object. @[%s:%d]\n", __FILE__, __LINE__);
	#endif

	if( NULL == (self = (coderunner_ChildProcessObject *)(type->tp_alloc(type, 0))) )
	{ return NULL; }

	memset(&(self->childprocess_instance), 0, sizeof(CodeRunInstance));

	return (PyObject *)(self);
}

static void ChildProcess_dealloc(coderunner_ChildProcessObject *self)
{
	#if DUMP_DEBUG_MSG
		fprintf(stderr, "INFO: dealloc ChildProcess object. @[%s:%d]\n", __FILE__, __LINE__);
	#endif

	self->ob_type->tp_free( (PyObject *)(self) );
}

static int ChildProcess_init(coderunner_ChildProcessObject *self, PyObject *args, PyObject *kwds)
{
	static char *kwdlist[] = {"args", "env", "executable", "cwd", "run_as", "stdin", "stdout", "stderr", "max_run_second", "overtime_period_sigint_second", "overtime_period_sigterm_second", NULL};
	static char *arglist = "O|OzzzzzzIII";

	char arg_buffer[MAX_ARGUMENT_BUFFER_SIZE];
	char *arg_v[MAX_ARGUMENT_BUFFER_SIZE];
	char env_buffer[MAX_ENVIRONMENT_BUFFER_SIZE];
	char *env_p[MAX_ENVIRONMENT_COUNT];

	PyObject *arg_obj;
	PyObject *env_obj;
	char *filepath_executable;
	char *working_directory;
	char *runner_account;
	char *filepath_stdin;
	char *filepath_stdout;
	char *filepath_stderr;
	int max_runtime;
	int ot_period_sigint;
	int ot_period_sigterm;

	int retcode;

	arg_obj = NULL;
	env_obj = NULL;
	filepath_executable = NULL;
	working_directory = NULL;
	runner_account = NULL;
	filepath_stdin = NULL;
	filepath_stdout = NULL;
	filepath_stderr = NULL;
	max_runtime = 0;
	ot_period_sigint = 0;
	ot_period_sigterm = 0;

	#if DUMP_DEBUG_MSG
		fprintf(stderr, "INFO: init ChildProcess object. @[%s:%d]\n", __FILE__, __LINE__);
	#endif

	if( !PyArg_ParseTupleAndKeywords(args, kwds, arglist, kwdlist, &arg_obj, &env_obj, &filepath_executable, &working_directory, &runner_account, &filepath_stdin, &filepath_stdout, &filepath_stderr, &max_runtime, &ot_period_sigint, &ot_period_sigterm) )
	{ return -1; }

	if(0 != _ChildProcess_prepare_arg(arg_buffer, arg_v, arg_obj))
	{ return -1; }

	if(0 != _ChildProcess_prepare_env(env_buffer, env_p, env_obj))
	{ return -1; }

	if(NULL == filepath_executable)
	{ filepath_executable = arg_v[0]; }

	if(NULL == working_directory)
	{ working_directory = "."; }


	retcode = run_program(&(self->childprocess_instance),
		filepath_executable,
		arg_v, env_p,
		working_directory,
		runner_account,
		filepath_stdin, filepath_stdout, filepath_stderr,
		(uint32_t)(max_runtime),
		(uint32_t)(ot_period_sigint),
		(uint32_t)(ot_period_sigterm));

	if(0 == retcode)
	{ return 0; }

	_report_error(PyExc_RuntimeError, "ERR: failed on activate run_program function (ret=%d). @[%s:%d]", retcode, __FILE__, __LINE__);
	return -1;
}


static PyObject * _ChildProcess_wait_impl(coderunner_ChildProcessObject *self, int is_blocking)
{
	int retcode;

	retcode = wait_program(&(self->childprocess_instance), is_blocking);

	if(0 == retcode)
	{ Py_RETURN_TRUE; }
	else if(1 == retcode)
	{ Py_RETURN_FALSE; }

	_report_error(PyExc_RuntimeError, "ERR: failed on waiting program (errno=%d). @[%s:%d]", self->childprocess_instance.last_errno, __FILE__, __LINE__);
	return NULL;
}

static PyObject * ChildProcess_wait_blocking(coderunner_ChildProcessObject *self)
{
	return _ChildProcess_wait_impl(self, 1);
}

static PyObject * ChildProcess_wait_nonblocking(coderunner_ChildProcessObject *self)
{
	return _ChildProcess_wait_impl(self, 0);
}


static PyObject * ChildProcess_stop(coderunner_ChildProcessObject *self)
{
	int retcode;

	retcode = stop_program(&(self->childprocess_instance));

	if(0 == retcode)
	{ Py_RETURN_TRUE; }

	_report_error(PyExc_RuntimeError, "ERR: failed on stop program (retcode=%d, errno=%d). @[%s:%d]", retcode, self->childprocess_instance.last_errno, __FILE__, __LINE__);
	return NULL;
}


static PyObject * ChildProcess_get_result(coderunner_ChildProcessObject *self)
{
	if((time_t)(0) == self->childprocess_instance.tstamp_finish)
	{
		return PyTuple_Pack(2, Py_None, Py_None);
	}
	else if(-1 == self->childprocess_instance.stop_signal)
	{
		return PyTuple_Pack(2, PyInt_FromLong(self->childprocess_instance.return_code), Py_None);
	}
	else
	{
		return PyTuple_Pack(2, Py_None, PyInt_FromLong(self->childprocess_instance.stop_signal));
	}
}



/*
vim: ts=4 sw=4 ai nowrap
*/
