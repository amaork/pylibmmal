#include <Python.h>
#include "constants.h"
#include "mmal_graph.h"
#include "tv_service.h"


#define _VERSION_ "0.1"
#define _NAME_ "pylibmmal"
PyDoc_STRVAR(pylibmmal_doc, "Raspberry Multi-Media Abstraction Layer Library.\n");


static PyMethodDef pylibmmal_methods[] = {
	{NULL}
};


#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef pylibmmalmodule = {
	PyModuleDef_HEAD_INIT,
	_NAME_,		            /* Module name */
	pylibmmal_doc,	        /* Module pylibi2cMethods */
	-1,			            /* size of per-interpreter state of the module, size of per-interpreter state of the module,*/
	pylibmmal_methods,      /* Module methods */
};
#endif


#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_pylibmmal(void)
#else
PyMODINIT_FUNC initpylibmmal(void)
#endif
{

	PyObject *module;

	if (PyType_Ready(&MmalGraphObjectType) < 0) {

#if PY_MAJOR_VERSION >= 3
		return NULL;
#else
		return;
#endif
	}

	if (PyType_Ready(&TVServiceObjectType) < 0) {

#if PY_MAJOR_VERSION >= 3
		return NULL;
#else
		return;
#endif
	}

#if PY_MAJOR_VERSION >= 3
	module = PyModule_Create(&pylibmmalmodule);
	PyObject *version = PyUnicode_FromString(_VERSION_);
#else
	module = Py_InitModule3(_NAME_, pylibmmal_methods, pylibmmal_doc);
	PyObject *version = PyString_FromString(_VERSION_);
#endif

	/* Version */
	PyObject *dict = PyModule_GetDict(module);
	PyDict_SetItemString(dict, "__version__", version);
	Py_DECREF(version);

	/* Constants */
	define_constants(module);

	/* TVService */
	Py_INCREF(&TVServiceObjectType);
	PyModule_AddObject(module, TVService_name, (PyObject *)&TVServiceObjectType);

	/* MmalGraph */
	Py_INCREF(&MmalGraphObjectType);
	PyModule_AddObject(module, MmalGraph_name, (PyObject *)&MmalGraphObjectType);

#if PY_MAJOR_VERSION >= 3
	return module;
#endif
}

