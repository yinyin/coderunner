#include <Python.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "coderunner.h"

#define DUMP_DEBUG_MSG 1

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
	#define PyMODINIT_FUNC void
#endif


#define ____CODE_INDICATOR_05a04832113c7891f7e5dffd4ef5a9a396a25058 1


PyMODINIT_FUNC initcoderunner(void);


static void _report_error(PyObject *errtype, const char *format, ...)
{
	char message_buf[1024];
	va_list arg;

	va_start(arg, format);
	PyOS_vsnprintf(message_buf, 1024, format, arg);
	va_end(arg);

	#if DUMP_DEBUG_MSG
		fputs(message_buf, stderr);
	#endif	/* DUMP_DEBUG_MSG */

	if(NULL != errtype)
	{
		PyErr_SetString(errtype, message_buf);
	}
}

/* {{{ object data structure */
typedef struct {
	PyObject_HEAD

	/* Type-specific fields */
	CodeRunInstance childprocess_instance;
} coderunner_ChildProcessObject;
/* }}} object data structure */


#include "childprocess.c"


/** method table for ChildProcess class */
static PyMethodDef coderunner_ChildProcess_methods[] = {
	{"wait_blocking", (PyCFunction)ChildProcess_wait_blocking, METH_NOARGS,
		"Blocking wait for program stop. Return True on program stopped." },
	{"wait_nonblocking", (PyCFunction)ChildProcess_wait_nonblocking, METH_NOARGS,
		"Blocking wait for program stop. Return True on program stopped, False if program still running." },
	{"stop", (PyCFunction)ChildProcess_stop, METH_NOARGS,
		"Stop running program. Need call wait_* method after invoke this method to collect exit code." },
	{"stop", (PyCFunction)ChildProcess_get_result, METH_NOARGS,
		"Receiving result of program. A two element tuple would be returned." },
	{NULL}  /* Sentinel */
};

static PyTypeObject coderunner_ChildProcessType = {
	PyObject_HEAD_INIT(NULL)
	0,							/* ob_size; */
	"coderunner.ChildProcess",	/* const char	*tp_name; # For printing, in format "<module>.<name>" */

	/* For allocation */
	sizeof(coderunner_ChildProcessObject),	/* Py_ssize_t tp_basicsize; */
	0,							/* Py_ssize_t	tp_itemsize; */

	/* Methods to implement standard operations */
	(destructor)(ChildProcess_dealloc),	/* destructor	tp_dealloc; */
	0,							/* printfunc	tp_print; */
	0,							/* getattrfunc	tp_getattr; */
	0,							/* setattrfunc	tp_setattr; */
	0,							/* cmpfunc		tp_compare; */
	0,							/* reprfunc		tp_repr; */

	/* Method suites for standard classes */
	0,							/* PyNumberMethods		*tp_as_number; */
	0,							/* PySequenceMethods	*tp_as_sequence */
	0,							/* PyMappingMethods		*tp_as_mapping */

	/* More standard operations (here for binary compatibility) */
	0,							/* hashfunc		tp_hash; */
	0,							/* ternaryfunc	tp_call; */
	0,							/* reprfunc		tp_str; */
	0,							/* getattrofunc	tp_getattro; */
	0,							/* setattrofunc	tp_setattro; */

	/* Functions to access object as input/output buffer */
	0,							/* PyBufferProcs	*tp_as_buffer; */

	( Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE ),	/* long	tp_flags; # Flags to define presence of optional/expanded features */
	"Child process class for managing process status",	/* const char *tp_doc;	# Documentation string */

	0,							/* traverseproc tp_traverse; # call function for all accessible objects */
	0,							/* inquiry tp_clear; # delete references to contained objects */

	0,							/* richcmpfunc tp_richcompare; # rich comparisons */
	0,							/* Py_ssize_t tp_weaklistoffset; # weak reference enabler */

	/* Iterators */
	0,							/* getiterfunc tp_iter; */
	0,							/* iternextfunc tp_iternext; */

	/* Attribute descriptor and subclassing stuff */
	coderunner_ChildProcess_methods,	/* struct PyMethodDef *tp_methods; */
	0,							/* struct PyMemberDef *tp_members; */
	0,							/* struct PyGetSetDef *tp_getset; */
	0,							/* struct _typeobject *tp_base; */
	0,							/* PyObject *tp_dict; */
	0,							/* descrgetfunc tp_descr_get; */
	0,							/* descrsetfunc tp_descr_set; */
	0,							/* Py_ssize_t tp_dictoffset; */
	(initproc)(ChildProcess_init),				/* initproc tp_init; */
	0,							/* allocfunc tp_alloc; */
	ChildProcess_new,				/* newfunc tp_new; */
	0,							/* freefunc tp_free; # Low-level free-memory routine */
	0,							/* inquiry tp_is_gc; # For PyObject_IS_GC */
	0,							/* PyObject *tp_bases; */
	0,							/* PyObject *tp_mro; # method resolution order */
	0,							/* PyObject *tp_cache; */
	0,							/* PyObject *tp_subclasses; */
	0,							/* PyObject *tp_weaklist; */
	0,							/* destructor tp_del; */
#if 0	/* *** */
	0,							/* unsigned int tp_version_tag; # Type attribute cache version tag. Added in version 2.6 */
#endif
};




/** function table for module-level functions */
static PyMethodDef coderunner_functions[] = {
	{NULL}  /* Sentinel */
};


PyMODINIT_FUNC initcoderunner(void)
{
	PyObject* m;

	if(PyType_Ready(&coderunner_ChildProcessType) < 0)
	{ return; }

	m = Py_InitModule3("coderunner", coderunner_functions, "Code runner module.");

	Py_INCREF(&coderunner_ChildProcessType);
	PyModule_AddObject(m, "ChildProcess", (PyObject *)&coderunner_ChildProcessType);
}


/*
vim: ts=4 sw=4 ai nowrap
*/
