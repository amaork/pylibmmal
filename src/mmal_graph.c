#include <Python.h>
#include <stdio.h>
#include <mmal.h>
#include <bcm_host.h>
#include <util/mmal_graph.h>
#include <util/mmal_util_params.h>
#include <util/mmal_default_components.h>
#include "mmal_graph.h"


PyDoc_STRVAR(MmalGraphObject_type_doc, "MmalGraph() -> Video core graph object.\n");
typedef struct {
    PyObject_HEAD;
    MMAL_GRAPH_T *graph;
    uint32_t display_num;
    MMAL_COMPONENT_T *reader, *decoder, *renderer;
} MmalGraphObject;


static PyObject *MmalGraph_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    MmalGraphObject *self;
    if ((self = (MmalGraphObject *)type->tp_alloc(type, 0)) == NULL) {

        return NULL;
    }

    self->graph = NULL;
    self->reader = NULL;
    self->decoder = NULL;
    self->renderer = NULL;
    self->display_num = 5;

    Py_INCREF(self);
    return (PyObject *)self;
}


PyDoc_STRVAR(MmalGraph_close_doc, "close()\n\nStop playback.\n");
static PyObject *MmalGraph_close(MmalGraphObject *self) {

    if (self->graph)
        mmal_graph_disable(self->graph);
    if (self->reader)
        mmal_component_release(self->reader);
    if (self->decoder)
        mmal_component_release(self->decoder);
    if (self->renderer)
        mmal_component_release(self->renderer);
    if (self->graph)
        mmal_graph_destroy(self->graph);

    Py_INCREF(Py_None);
    return Py_None;
}


static void MmalGraph_free(MmalGraphObject *self) {

    PyObject *ref = MmalGraph_close(self);
    Py_XDECREF(ref);

    Py_TYPE(self)->tp_free((PyObject *)self);
}


static int MmalGraph_init(MmalGraphObject *self, PyObject *args, PyObject *kwds) {

    int display = -1;
    static char *kwlist[] = {"display", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &display)) {

        return -1;
    }

    if (display >= 0) {

        self->display_num = display;
    }

    return 0;
}


static PyObject *MmalGraph_enter(PyObject *self, PyObject *args) {

    if (!PyArg_ParseTuple(args, ""))
        return NULL;

    Py_INCREF(self);
    return self;
}


static PyObject *MmalGraph_exit(MmalGraphObject *self, PyObject *args) {

    PyObject *exc_type = 0;
    PyObject *exc_value = 0;
    PyObject *traceback = 0;

    if (!PyArg_UnpackTuple(args, "__exit__", 3, 3, &exc_type, &exc_value, &traceback)) {

        return 0;
    }

    MmalGraph_close(self);
    Py_RETURN_FALSE;
}


static void graph_control_cb(MMAL_GRAPH_T *graph, MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer, void *cb_data) {

    mmal_buffer_header_release(buffer);
}


PyDoc_STRVAR(MmalGraph_open_doc, "open(uri)\n\nOpen a uri to start playback.\n");
#define CHECK_STATUS(status, msg) if (status != MMAL_SUCCESS) { fprintf(stderr, msg"\n"); goto error; }
static PyObject* MmalGraph_open(MmalGraphObject *self, PyObject *args, PyObject *kwds) {

    char *uri = NULL;
    MMAL_STATUS_T status;
    MMAL_DISPLAYREGION_T param;

    if (!PyArg_ParseTuple(args, "s", &uri)) {

        fprintf(stderr, "Get arguments error!\n");
        goto error;
    }

    bcm_host_init();

    /* Create the graph */
    status = mmal_graph_create(&self->graph, 0);
    CHECK_STATUS(status, "failed to create graph");

    /* Add the components */
    status = mmal_graph_new_component(self->graph, MMAL_COMPONENT_DEFAULT_CONTAINER_READER, &self->reader);
    CHECK_STATUS(status, "failed to create reader");

    status = mmal_graph_new_component(self->graph, MMAL_COMPONENT_DEFAULT_IMAGE_DECODER, &self->decoder);
    CHECK_STATUS(status, "failed to create decoder");

    status = mmal_graph_new_component(self->graph, MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER, &self->renderer);
    CHECK_STATUS(status, "failed to create renderer");

    param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
    param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);
    param.set = MMAL_DISPLAY_SET_LAYER|MMAL_DISPLAY_SET_NUM;
    param.display_num = self->display_num;
    status = mmal_port_parameter_set(self->renderer->input[0], &param.hdr);

    /* Configure the reader using the given URI */
    status = mmal_util_port_set_uri(self->reader->control, uri);
    CHECK_STATUS(status, "failed to set uri");

    /* connect them up - this propagates port settings from outputs to inputs */
    status = mmal_graph_new_connection(self->graph, self->reader->output[0], self->decoder->input[0], 0, NULL);
    CHECK_STATUS(status, "failed to connect reader to decoder");
    status = mmal_graph_new_connection(self->graph, self->decoder->output[0], self->renderer->input[0], 0, NULL);
    CHECK_STATUS(status, "failed to connect decoder to renderer");

    /* Start playback */
    status = mmal_graph_enable(self->graph, graph_control_cb, NULL);
    CHECK_STATUS(status, "failed to enable graph");

    Py_INCREF(Py_None);
    return Py_None;

error:
    /* Cleanup everything */
    return MmalGraph_close(self);
}


/* pylibi2c module methods */
static PyMethodDef MmalGraph_methods[] = {

    {"open", (PyCFunction)MmalGraph_open, METH_VARARGS, MmalGraph_open_doc},
    {"close", (PyCFunction)MmalGraph_close, METH_NOARGS, MmalGraph_close_doc},
    {"__enter__", (PyCFunction)MmalGraph_enter, METH_NOARGS, NULL},
    {"__exit__", (PyCFunction)MmalGraph_exit, METH_NOARGS, NULL},
    {NULL},
};


PyTypeObject MmalGraphObjectType = {
#if PY_MAJOR_VERSION >= 3
    PyVarObject_HEAD_INIT(NULL, 0)
#else
    PyObject_HEAD_INIT(NULL)
    0,				            /* ob_size */
#endif
    MmalGraph_name,		        /* tp_name */
    sizeof(MmalGraphObject),	/* tp_basicsize */
    0,			        	    /* tp_itemsize */
    (destructor)MmalGraph_free,/* tp_dealloc */
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
    MmalGraphObject_type_doc,	/* tp_doc */
    0,				            /* tp_traverse */
    0,				            /* tp_clear */
    0,				            /* tp_richcompare */
    0,				            /* tp_weaklistoffset */
    0,				            /* tp_iter */
    0,				            /* tp_iternext */
    MmalGraph_methods,		    /* tp_methods */
    0,				            /* tp_members */
    0,				            /* tp_getset */
    0,				            /* tp_base */
    0,				            /* tp_dict */
    0,				            /* tp_descr_get */
    0,				            /* tp_descr_set */
    0,				            /* tp_dictoffset */
    (initproc)MmalGraph_init,	/* tp_init */
    0,				            /* tp_alloc */
    MmalGraph_new,		        /* tp_new */
};

