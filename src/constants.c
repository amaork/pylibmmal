#include <Python.h>
#include "constants.h"


void define_constants(PyObject *module) {

	PyObject *lcd = Py_BuildValue("i", LCD);
	PyModule_AddObject(module, "LCD", lcd);

	PyObject *hdmi = Py_BuildValue("i", HDMI);
	PyModule_AddObject(module, "HDMI", hdmi);

	PyObject *cea = Py_BuildValue("s", HDMI_CEA);
	PyModule_AddObject(module, "CEA", cea);

	PyObject *dmt = Py_BuildValue("s", HDMI_DMT);
	PyModule_AddObject(module, "DMT", dmt);
}
