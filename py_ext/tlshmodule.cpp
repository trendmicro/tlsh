#define PY_SSIZE_T_CLEAN 1
#include <Python.h>
#include <new>
#include <bytesobject.h>
#include "tlsh.h"

#define TLSH_VERSION "0.2.0"
#define AUTHOR "Chun Cheng"

/* We want to update the hash using bytes object in Python 3 */
#if PY_MAJOR_VERSION >= 3
# define BYTES_VALUE_CHAR "y"
#else
# define BYTES_VALUE_CHAR "s"
#endif

static char tlsh_doc[] =
  "TLSH C version - similarity matching and searching";
static char tlsh_hash_doc[] =
  "tlsh.hash(data)\n\n\
  returns tlsh hexadecimal representation (string)";
static char tlsh_diff_doc[] =
  "tlsh.diff(hash1, hash2)\n\n\
  returns tlsh score (integer)";
static char tlsh_diffxlen_doc[] =
  "tlsh.diffxlen(hash1, hash2) - ignore object lengths\n\n\
  returns tlsh score (integer)";

// hash(data) returns byte buffer
static PyObject* hash_py(PyObject* self, PyObject* args) {
  unsigned char* pBuffer;
  Py_ssize_t len;
  if (!PyArg_ParseTuple(args, BYTES_VALUE_CHAR "#", &pBuffer, &len)) {
    return NULL;
  }
  
  Tlsh tlsh;
  tlsh.update(pBuffer, len);
  tlsh.final();
  const char *s = tlsh.getHash();

  return Py_BuildValue("s", s);
}

// diff(hash1, hash2) returns integer
static PyObject* diff_py(PyObject* self, PyObject* args) {
  char *hash1, *hash2;
  if (!PyArg_ParseTuple(args, "ss", &hash1, &hash2)) {
    return NULL;
  }
  
  Tlsh tlsh1, tlsh2;
  if (tlsh1.fromTlshStr(hash1) != 0) {
    return PyErr_Format(PyExc_ValueError, "argument %s is not a TLSH hex string", hash1);
  }
  if (tlsh2.fromTlshStr(hash2) != 0) {
    return PyErr_Format(PyExc_ValueError, "argument %s is not a TLSH hex string", hash2);
  }

  int score = tlsh1.totalDiff(&tlsh2);

  return Py_BuildValue("i", score);
}

//diffxlen(hash1, hash2) returns integer
static PyObject* diffxlen_py(PyObject* self, PyObject* args) {
  char *hash1, *hash2;
  if (!PyArg_ParseTuple(args, "ss", &hash1, &hash2)) {
      return NULL;
  }

  Tlsh tlsh1, tlsh2;
  if (tlsh1.fromTlshStr(hash1) != 0) {
    return PyErr_Format(PyExc_ValueError, "argument %s is not a TLSH hex string", hash1);
  }
  if (tlsh2.fromTlshStr(hash2) != 0) {
    return PyErr_Format(PyExc_ValueError, "argument %s is not a TLSH hex string", hash2);
  }

  int score = tlsh1.totalDiff(&tlsh2, false);

  return Py_BuildValue("i", score);
}

// The module's methods
static PyMethodDef tlsh_methods[] =
{
  { "hash", hash_py, METH_VARARGS, tlsh_hash_doc },
  { "diff", diff_py, METH_VARARGS, tlsh_diff_doc },
  { "diffxlen", diffxlen_py, METH_VARARGS, tlsh_diffxlen_doc },
  { NULL, NULL } /* sentinel */
};

typedef struct {
    PyObject_HEAD
    unsigned short required_data;
    bool finalized;
    Tlsh tlsh;
} tlsh_TlshObject;

static int Tlsh_init(PyObject *, PyObject *, PyObject *);
static PyObject * Tlsh_fromTlshStr(tlsh_TlshObject *, PyObject *);
static PyObject * Tlsh_update(tlsh_TlshObject *, PyObject *);
static PyObject * Tlsh_final(tlsh_TlshObject *);
static PyObject * Tlsh_hexdigest(tlsh_TlshObject *);
static PyObject * Tlsh_diff(tlsh_TlshObject *, PyObject *);

static PyMethodDef Tlsh_methods[] = {
    {"fromTlshStr", (PyCFunction) Tlsh_fromTlshStr, METH_VARARGS,
     "Create a TLSH instance from a hex string."
    },
    {"update", (PyCFunction) Tlsh_update, METH_VARARGS,
     "Update the TLSH with the given string."
    },
    {"final", (PyCFunction) Tlsh_final, METH_NOARGS,
     "Signal that no more data will be added. This is required before reading the hash."
    },
    {"hexdigest", (PyCFunction) Tlsh_hexdigest, METH_NOARGS,
     "Get the computed TLSH as a string object containing only hexadecimal digits."
    },
    {"diff", (PyCFunction) Tlsh_diff, METH_VARARGS,
     "Returns the TLSH score compared to the given Tlsh object or hexadecimal string."
    },
    {NULL} /* Sentinel */
};

static PyTypeObject tlsh_TlshType = {
    PyObject_HEAD_INIT(NULL)
#if PY_MAJOR_VERSION < 3
    0,                         /* ob_size */
#endif
    "tlsh.Tlsh",               /* tp_name */
    sizeof(tlsh_TlshObject),   /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_compare */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "TLSH objects",            /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Tlsh_methods,              /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    Tlsh_init,                 /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};

static int
Tlsh_init(PyObject *self, PyObject *args, PyObject *kwds)
{
    tlsh_TlshObject * tlsh_object = (tlsh_TlshObject *) self;

    if (PyTuple_Size(args) > 1) {
        PyErr_Format(PyExc_TypeError, "Tlsh() takes at most 1 argument (%lu given)", PyTuple_Size(args));
        return -1;
    }
    if (kwds) {
        PyErr_SetString(PyExc_TypeError, "Tlsh() takes no keyword arguments");
        return -1;
    }

   /* Call Tlsh() constructor. */
   new (&tlsh_object->tlsh) Tlsh();

   if (PyTuple_Size(args) == 1) {
        Tlsh_update(tlsh_object, args);
        if (PyErr_Occurred())
            return -1;
   }

    return 0;
}

static PyObject *
Tlsh_fromTlshStr(tlsh_TlshObject *self, PyObject *args)
{
    char *str;
    Py_ssize_t len;

    PyObject *arg;

    if (PyTuple_Size(args) != 1)
        return PyErr_Format(PyExc_TypeError, "function takes exactly 1 argument (%lu given)", PyTuple_Size(args));

    arg = PyTuple_GetItem(args, 0);
#if PY_MAJOR_VERSION >= 3
    if (!PyUnicode_Check(arg) || (arg = PyUnicode_AsASCIIString(arg)) == NULL) {
      PyErr_SetString(PyExc_ValueError, "argument is not a TLSH hex string");
      return NULL;
    }
#else
    if (!PyString_Check(arg)) {
      PyErr_SetString(PyExc_ValueError, "argument is not a TLSH hex string");
      return NULL;
    }
#endif

    if (PyBytes_AsStringAndSize(arg, &str, &len) == -1) {
        PyErr_SetString(PyExc_ValueError, "argument is not a TLSH hex string");
        return NULL;
    }

    if (len != TLSH_STRING_LEN) {
        PyErr_SetString(PyExc_ValueError, "argument length incorrect: not a TLSH hex string");
        return NULL;
    }

    if (self->tlsh.fromTlshStr(str) != 0) {
        PyErr_SetString(PyExc_ValueError, "argument value incorrect: not a TLSH hex string");
        return NULL;
    }
    self->finalized = true;

    Py_RETURN_NONE;
}

static PyObject *
Tlsh_update(tlsh_TlshObject *self, PyObject *args)
{
    const char *str;
    Py_ssize_t len;

    if (!PyArg_ParseTuple(args, BYTES_VALUE_CHAR "#", &str, &len))
        return NULL;

    if (self->finalized) {
        PyErr_SetString(PyExc_ValueError, "final() has already been called");
        return NULL;
    }
    if (self->required_data < MIN_DATA_LENGTH) {
        self->required_data += len > MIN_DATA_LENGTH ? MIN_DATA_LENGTH : len;
    }

    self->tlsh.update((unsigned char *) str, (unsigned int) len);

    Py_RETURN_NONE;
}

static PyObject *
Tlsh_final(tlsh_TlshObject *self)
{
    if (self->finalized) {
        PyErr_SetString(PyExc_ValueError, "final() has already been called");
        return NULL;
    }
    if (self->required_data < MIN_DATA_LENGTH) {
        return PyErr_Format(PyExc_ValueError, "less than %u of input", MIN_DATA_LENGTH);
    }
    self->finalized = true;
    self->tlsh.final();

    Py_RETURN_NONE;
}

static PyObject *
Tlsh_hexdigest(tlsh_TlshObject *self)
{
    char hash[TLSH_STRING_LEN + 1];

    if (!self->finalized) {
        PyErr_SetString(PyExc_ValueError, "final() has not been called");
        return NULL;
    }
    self->tlsh.getHash(hash, TLSH_STRING_LEN + 1);
    if (hash[0] == '\0') {
        PyErr_SetString(PyExc_ValueError, "error while getting hash (not enough entropy?)");
        return NULL;
    }
    return Py_BuildValue("s", hash);
}

static PyObject *
Tlsh_diff(tlsh_TlshObject *self, PyObject *args)
{
    PyObject *arg;
    int score;

    if (PyTuple_Size(args) != 1)
        return PyErr_Format(PyExc_TypeError, "function takes exactly 1 argument (%lu given)", PyTuple_Size(args));

    arg = PyTuple_GetItem(args, 0);
#if PY_MAJOR_VERSION >= 3
    if (PyUnicode_Check(arg)) {
      if ((arg = PyUnicode_AsASCIIString(arg)) == NULL) {
        PyErr_SetString(PyExc_ValueError, "argument is not a TLSH hex string");
        return NULL;
      }
#else
    if (PyString_Check(arg)) {
#endif
      char *str;
      Py_ssize_t len;
      Tlsh other;
      if (PyBytes_AsStringAndSize(arg, &str, &len) == -1) {
        PyErr_SetString(PyExc_ValueError, "argument is not a TLSH hex string");
        return NULL;
      }
      if (len != TLSH_STRING_LEN) {
        PyErr_SetString(PyExc_ValueError, "argument is not a TLSH hex string");
        return NULL;
      }
      if (other.fromTlshStr(str) != 0) {
        PyErr_SetString(PyExc_ValueError, "argument is not a TLSH hex string");
        return NULL;
      }
      score = self->tlsh.totalDiff(&other);
    } else if (PyObject_TypeCheck(arg, &tlsh_TlshType)) {
      tlsh_TlshObject * other_tlsh = (tlsh_TlshObject *) arg;
      score = self->tlsh.totalDiff(&other_tlsh->tlsh);
    } else {
      PyErr_SetString(PyExc_ValueError, "argument is neither a Tlsh object nor a TLSH hex string");
      return NULL;
    }

    return Py_BuildValue("i", score);
}

// Initializes the module
#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
            PyModuleDef_HEAD_INIT,
            "tlsh",     /* m_name */
            tlsh_doc,  /* m_doc */
            -1,                  /* m_size */
            tlsh_methods,    /* m_methods */
            NULL,                /* m_reload */
            NULL,                /* m_traverse */
            NULL,                /* m_clear */
            NULL,                /* m_free */
        };
    
    PyMODINIT_FUNC PyInit_tlsh(void)
    {
        PyObject *module;

        tlsh_TlshType.tp_new = PyType_GenericNew;
        if (PyType_Ready(&tlsh_TlshType) < 0)
            return NULL;

        module = PyModule_Create(&moduledef);
        PyModule_AddStringConstant(module,
        "__version__",
        TLSH_VERSION);
        PyModule_AddStringConstant(module,
        "__author__",
        AUTHOR);

        Py_INCREF(&tlsh_TlshType);
        PyModule_AddObject(module, "Tlsh", (PyObject *) &tlsh_TlshType);

        return module;
    }
#else

    PyMODINIT_FUNC inittlsh(void)
    {
        PyObject *module;

        tlsh_TlshType.tp_new = PyType_GenericNew;
        if (PyType_Ready(&tlsh_TlshType) < 0)
            return;

        module = Py_InitModule3("tlsh",
        tlsh_methods,
        tlsh_doc);
        PyModule_AddStringConstant(module,
        "__version__",
        TLSH_VERSION);
        PyModule_AddStringConstant(module,
        "__author__",
        AUTHOR);

        Py_INCREF(&tlsh_TlshType);
        PyModule_AddObject(module, "Tlsh", (PyObject *) &tlsh_TlshType);
    }
#endif
