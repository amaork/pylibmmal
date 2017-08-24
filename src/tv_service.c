#include <Python.h>
#include <stdio.h>
#include <string.h>
#include <interface/vmcs_host/vc_tvservice.h>
#include "tv_service.h"

#define MAX_MODE_ID (127)
#define CHECK_ERROR(ret, fmt, arg...) if (ret != 0) { fprintf(stderr, "[E] " fmt "\n", ##arg); goto error; }


PyDoc_STRVAR(TVServiceObject_type_doc,
             "TVService() -> Controls for both HDMI and analogue TVs.\n"
             "It allows the user to dynamically switch between HDMI and SDTV without having"
             "to worry about switch one off before turning the other on. \n"
             "It also allows the user to query the supported HDMI resolutions and audio formats and turn on/off copy protection.");


typedef struct {

    PyObject_HEAD;
    uint32_t preferred_mode;
    HDMI_RES_GROUP_T preferred_group;
    VCHI_INSTANCE_T vchi_instance;
    VCHI_CONNECTION_T *vchi_connection;
} TVServiceObject;



static PyObject *TVService_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    TVServiceObject *self;
    if ((self = (TVServiceObject *)type->tp_alloc(type, 0)) == NULL) {

        return NULL;
    }

    Py_INCREF(self);
    return (PyObject *)self;
}


static PyObject *TVService_stop(TVServiceObject *self) {

    /* Stop tvservice */
    vc_vchi_tv_stop();

    /* Disconnect the VCHI connection */
    vchi_disconnect(self->vchi_instance);

    Py_INCREF(Py_None);
    return Py_None;
}


static void TVService_free(TVServiceObject *self) {

    PyObject *ref = TVService_stop(self);
    Py_XDECREF(ref);

    Py_TYPE(self)->tp_free((PyObject *)self);
}


static int TVService_init(TVServiceObject *self, PyObject *args, PyObject *kwds) {

    int32_t ret = 0;

    /* Initialize VCOS */
    vcos_init();

    // Initialize the VCHI connection
    ret = vchi_initialise(&self->vchi_instance);
    CHECK_ERROR(ret, "Failed to initialize VCHI");

    ret = vchi_connect(NULL, 0, self->vchi_instance);
    CHECK_ERROR(ret, "Failed to create VCHI connection");

    /* Initialize the tvservice */
    vc_vchi_tv_init(self->vchi_instance, &self->vchi_connection, 1);

    return 0;

error:
    return -1;
}


static PyObject *TVService_enter(PyObject *self, PyObject *args) {

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    Py_INCREF(self);
    return self;
}


static PyObject *TVService_exit(TVServiceObject *self, PyObject *args) {

    PyObject *exc_type = 0;
    PyObject *exc_value = 0;
    PyObject *traceback = 0;

    if (!PyArg_UnpackTuple(args, "__exit__", 3, 3, &exc_type, &exc_value, &traceback)) {

        return 0;
    }

    TVService_stop(self);
    Py_RETURN_FALSE;
}


static int hdmi_set_property(HDMI_PROPERTY_T prop, uint32_t param1, uint32_t param2) {

    int ret;
    HDMI_PROPERTY_PARAM_T property;

    property.property = prop;
    property.param1 = param1;
    property.param2 = param2;

    ret = vc_tv_hdmi_set_property(&property);
    CHECK_ERROR(ret, "Failed to set property");

error:
    return ret;
}


PyDoc_STRVAR(TVService_set_preferred_doc, "set_preferred()\n\nPower on HDMI with preferred settings\n");
static PyObject* TVService_set_preferred(TVServiceObject *self, PyObject *args, PyObject *kwds) {

    int ret;

    if (hdmi_set_property(HDMI_PROPERTY_3D_STRUCTURE, HDMI_3D_FORMAT_NONE, 0) != 0) {

        goto error;
    }

    ret = vc_tv_hdmi_power_on_preferred();
    CHECK_ERROR(ret, "Failed to power on HDMI with preferred settings");

    Py_INCREF(Py_None);
    return Py_None;

error:

    /* Cleanup everything */
    return TVService_stop(self);
}


PyDoc_STRVAR(TVService_set_explicit_doc, "set_explicit(group, mode)\n\nPower on HDMI with explicit GROUP(CEA, DMT, CEA_3D_SBS, CEA_3D_TB, CEA_3D_FP, CEA_3D_FS) and MODE\n");
static PyObject* TVService_set_explicit(TVServiceObject *self, PyObject *args, PyObject *kwds) {

    int ret;
    char *group_name = NULL;
    uint32_t mode, drive = HDMI_MODE_HDMI;
    HDMI_RES_GROUP_T group = HDMI_RES_GROUP_INVALID;
    static char *kwlist[] = {"group", "mode", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|si", kwlist, &group_name, &mode)) {

        fprintf(stderr, "Get args error\n");
        goto out;
    }

    if (vcos_strcasecmp(HDMI_RES_GROUP_NAME(HDMI_RES_GROUP_CEA), group_name) == 0) {

        group = HDMI_RES_GROUP_CEA;
    }
    else if (vcos_strcasecmp(HDMI_RES_GROUP_NAME(HDMI_RES_GROUP_DMT), group_name) == 0) {

        group = HDMI_RES_GROUP_DMT;
    }
    else {

        fprintf(stderr, "Invalid group: %s\n", group_name);
        goto out;
    }

    if (hdmi_set_property(HDMI_PROPERTY_3D_STRUCTURE, HDMI_3D_FORMAT_NONE, 0) != 0) {

        goto error;
    }

    if (hdmi_set_property(HDMI_PROPERTY_PIXEL_CLOCK_TYPE, HDMI_PIXEL_CLOCK_TYPE_PAL, 0) != 0) {

        goto error;
    }

    ret = vc_tv_hdmi_power_on_explicit_new(drive, group, mode);
    CHECK_ERROR(ret, "Failed to power on HDMI with explicit settings (%s, mode %u)", HDMI_RES_GROUP_NAME(group), mode);

out:
    Py_INCREF(Py_None);
    return Py_None;

error:
    /* Cleanup everything */
    return TVService_stop(self);
}


PyDoc_STRVAR(TVService_power_off_doc, "power_off()\n\nPower off the display\n");
static PyObject* TVService_power_off(TVServiceObject *self, PyObject *args, PyObject *kwds) {

    int ret = vc_tv_power_off();
    CHECK_ERROR(ret, "Failed to power off HDMI");

    Py_INCREF(Py_None);
    return Py_None;

error:
    /* Cleanup everything */
    return TVService_stop(self);
}


/* Return the string presentation of aspect ratio */
static const char *aspect_ratio_str(HDMI_ASPECT_T aspect_ratio) {

    switch(aspect_ratio) {
    case HDMI_ASPECT_4_3:
        return "4:3";
    case HDMI_ASPECT_14_9:
        return "14:9";
    case HDMI_ASPECT_16_9:
        return "16:9";
    case HDMI_ASPECT_5_4:
        return "5:4";
    case HDMI_ASPECT_16_10:
        return "16:10";
    case HDMI_ASPECT_15_9:
        return "15:9";
    case HDMI_ASPECT_64_27:
        return "64:27 (21:9)";
    default:
        return "unknown AR";
    }
}


PyDoc_STRVAR(TVService_get_modes_doc, "get_modes()\n\nGet supported modes for GROUP (CEA, DMT)\n");
static PyObject* TVService_get_modes(TVServiceObject *self, PyObject *args, PyObject *kwds) {

    int num_modes, i, j;
    char *group_name = NULL;
    PyObject *item = NULL, *modes = PyList_New(0);
    static TV_SUPPORTED_MODE_NEW_T supported_modes[MAX_MODE_ID];
    HDMI_RES_GROUP_T groups[] = {HDMI_RES_GROUP_CEA, HDMI_RES_GROUP_DMT};

    for (i = 0; i < vcos_countof(groups); i++) {

        group_name = HDMI_RES_GROUP_NAME(groups[i]);
        memset(supported_modes, 0, sizeof(supported_modes));

        num_modes = vc_tv_hdmi_get_supported_modes_new(
                        groups[i],
                        supported_modes,
                        vcos_countof(supported_modes),
                        &self->preferred_group,
                        &self->preferred_mode);

        if (num_modes < 0) {

            fprintf(stderr, "Failed to get modes");
            goto out;
        }

        /* Create modes list */
        for (j = 0; j < num_modes; j++) {

            item = PyDict_New();
            PyDict_SetItem(item, PyString_FromString("mode"), PyLong_FromLong(supported_modes[j].code));
            PyDict_SetItem(item, PyString_FromString("rate"), PyLong_FromLong(supported_modes[j].frame_rate));
            PyDict_SetItem(item, PyString_FromString("clock"), PyLong_FromLong(supported_modes[j].pixel_freq / 1000000U));

            PyDict_SetItemString(item, "group", PyString_FromString(group_name));
            PyDict_SetItemString(item, "scan_mode", PyString_FromString(supported_modes[j].scan_mode ? "i" : "p"));
            PyDict_SetItemString(item, "ratio", PyString_FromString(aspect_ratio_str(supported_modes[j].aspect_ratio)));
            PyDict_SetItemString(item, "res", PyString_FromFormat("%ux%u", supported_modes[j].width, supported_modes[j].height));

            /* Append to modes */
            PyList_Append(modes, item);
        }
    }

out:
    return modes;
}


PyDoc_STRVAR(TVService_preferred_mode_doc, "preferred_mode()\n\nGet HDMI preferred modes for (GROUP, MODE)\n");
static PyObject* TVService_preferred_mode(TVServiceObject *self, PyObject *args, PyObject *kwds) {

    TVService_get_modes(self, NULL, NULL);
    return Py_BuildValue("sI", HDMI_RES_GROUP_NAME(self->preferred_group), self->preferred_mode);
}

/* pylibi2c module methods */
static PyMethodDef TVService_methods[] = {

    {"preferred_mode", (PyCFunction)TVService_preferred_mode, METH_NOARGS, TVService_preferred_mode_doc},
    {"set_preferred", (PyCFunction)TVService_set_preferred, METH_NOARGS, TVService_set_preferred_doc},
    {"set_explicit", (PyCFunction)TVService_set_explicit, METH_VARARGS, TVService_set_explicit_doc},
    {"power_off", (PyCFunction)TVService_power_off, METH_NOARGS, TVService_power_off_doc},
    {"get_modes", (PyCFunction)TVService_get_modes, METH_NOARGS, TVService_get_modes_doc},
    {"__enter__", (PyCFunction)TVService_enter, METH_NOARGS, NULL},
    {"__exit__", (PyCFunction)TVService_exit, METH_NOARGS, NULL},
    {NULL},
};


PyTypeObject TVServiceObjectType = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,				            /* ob_size */
#endif
    TVService_name,	            /* tp_name */
    sizeof(TVServiceObject),	/* tp_basicsize */
    0,				            /* tp_itemsize */
    (destructor)TVService_free,	/* tp_dealloc */
    0,				            /* tp_print */
    0,				            /* tp_getattr */
    0,				            /* tp_setattr */
    0,				            /* tp_compare */
    0,				            /* tp_repr */
    0,				            /* tp_as_number */
    0,				            /* tp_as_sequence */
    0,				            /* tp_as_mapping */
    0,				            /* tp_hash */
    0,				            /* tp_call */
    0,				            /* tp_str */
    0,				            /* tp_getattro */
    0,				            /* tp_setattro */
    0,				            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    TVServiceObject_type_doc,	/* tp_doc */
    0,				            /* tp_traverse */
    0,				            /* tp_clear */
    0,				            /* tp_richcompare */
    0,				            /* tp_weaklistoffset */
    0,				            /* tp_iter */
    0,				            /* tp_iternext */
    TVService_methods,		    /* tp_methods */
    0,				            /* tp_members */
    0,				            /* tp_getset */
    0,				            /* tp_base */
    0,				            /* tp_dict */
    0,				            /* tp_descr_get */
    0,				            /* tp_descr_set */
    0,				            /* tp_dictoffset */
    (initproc)TVService_init,	/* tp_init */
    0,				            /* tp_alloc */
    TVService_new,		        /* tp_new */
};

