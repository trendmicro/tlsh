#include <Python.h>
#include "tlsh.h"

#define TLSH_VERSION "0.1.0"
#define AUTHOR "Chun Cheng"

static char tlsh_doc[] =
  "TLSH C version - similarity matching and searching";
static char tlsh_hash_doc[] =
  "tlsh.hash(data)\n\n\
  returns tlsh hash (string)";
static char tlsh_diff_doc[] =
  "tlsh.diff(hash1, hash2)\n\n\
  returns tlsh score (integer)";
static char tlsh_diffxlen_doc[] =
  "tlsh.diffxlen(hash1, hash2) - ignore object lengths\n\n\
  returns tlsh score (integer)";

// hash(data) returns byte buffer
static PyObject* hash_py(PyObject* self, PyObject* args) {
  unsigned char* pBuffer;
  int len;
  if (!PyArg_ParseTuple(args, "t#", &pBuffer, &len)) {
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
  tlsh1.fromTlshStr(hash1);
  tlsh2.fromTlshStr(hash2);

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
  tlsh1.fromTlshStr(hash1);
  tlsh2.fromTlshStr(hash2);

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
        PyObject *module = PyModule_Create(&moduledef);
        PyModule_AddStringConstant(module,
        "__version__",
        TLSH_VERSION);
        PyModule_AddStringConstant(module,
        "__author__",
        AUTHOR);
        return module;
    }
#else

    PyMODINIT_FUNC inittlsh(void)
    {
        PyObject *module = Py_InitModule3("tlsh",
        tlsh_methods,
        tlsh_doc);
        PyModule_AddStringConstant(module,
        "__version__",
        TLSH_VERSION);
        PyModule_AddStringConstant(module,
        "__author__",
        AUTHOR);
    }
#endif
