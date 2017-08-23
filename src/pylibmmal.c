#include <Python.h>
#include "mmal_graph.h"


#define LCD 4
#define HDMI 5
#define _VERSION_ "0.1"
#define _NAME_ "pylibmmal"
PyDoc_STRVAR(pylibmmal_doc, "Raspberry Multi-Media Abstraction Layer Library.\n");


static PyMethodDef pylibmmal_methods[] = {
	{NULL}
};


#if PY_MAJOR_VERSION > 2
static struct PyModuleDef pylibmmalmodule = {
	PyModuleDef_HEAD_INIT,
       	_NAME_,		/* Module name */
	pylibmmal_doc,	/* Module pylibi2cMethods */
       	-1,			/* size of per-interpreter state of the module, size of per-interpreter state of the module,*/
	pylibmmal_methods,
};
#endif


#if PY_MAJOR_VERSION >= 3
PyMODINIT_FUNC PyInit_pylibmmal(void)
#else
PyMODINIT_FUNC initpylibmmal(void)
#endif
{

	PyObject *module;

	if (PyType_Ready(&PyMmalGraphObjectType) < 0) {

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
	PyObject *dict = PyModule_GetDict(module);
	PyDict_SetItemString(dict, "__version__", version);
	Py_DECREF(version);

	PyObject *lcd = Py_BuildValue("i", LCD);
	PyModule_AddObject(module, "LCD", lcd);

	PyObject *hdmi = Py_BuildValue("i", HDMI);
	PyModule_AddObject(module, "HDMI", hdmi);

	Py_INCREF(&PyMmalGraphObjectType);
	PyModule_AddObject(module, PyMmalGraph_name, (PyObject *)&PyMmalGraphObjectType);


#if PY_MAJOR_VERSION >= 3
	return module;
#endif
}

