/*
 * TLSH is provided for use under two licenses: Apache OR BSD.
 * Users may opt to use either license depending on the license
 * restictions of the systems with which they plan to integrate
 * the TLSH code.
 */ 

/* ==============
 * Apache License
 * ==============
 * Copyright 2013 Trend Micro Incorporated
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* ===========
 * BSD License
 * ===========
 * Copyright (c) 2013, Trend Micro Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.

 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define PY_SSIZE_T_CLEAN 1
#include <Python.h>
#include <new>
#include <bytesobject.h>
#include "tlsh.h"

// to generate the "T1" hashes introduced in TLSH 4.0.0
// see 4.0.0 from 26/Mar/2020 at https://github.com/trendmicro/tlsh/blob/master/Change_History.md
#define SHOWVERSION	1

#define TLSH_VERSION "4.7.1"
#define AUTHOR "Jonathan Oliver, Chun Cheng and Yanggui Chen"

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
static char tlsh_oldhash_doc[] =
  "tlsh.oldhash(data)\n\n\
  returns old tlsh hexadecimal representation (string) - no 'T1'";
static char tlsh_forcehash_doc[] =
  "tlsh.forcehash(data)\n\n\
  returns tlsh hexadecimal representation (string) - allows strings down to 50 char";
static char tlsh_conservativehash_doc[] =
  "tlsh.conservativehash(data)\n\n\
  returns tlsh hexadecimal representation (string) - does not allow strings < 256 char";
static char tlsh_oldconservativehash_doc[] =
  "tlsh.oldconservativehash(data)\n\n\
  returns tlsh hexadecimal representation (string) - does not allow strings < 256 char";
static char tlsh_diff_doc[] =
  "tlsh.diff(hash1, hash2)\n\n\
  returns tlsh score (integer)";
static char tlsh_diffxlen_doc[] =
  "tlsh.diffxlen(hash1, hash2) - ignore object lengths\n\n\
  returns tlsh score (integer)";

static PyObject* eval_tlsh(unsigned char* pBuffer, Py_ssize_t len, int showvers)
{
Tlsh tlsh;
	tlsh.update(pBuffer, len);
	tlsh.final();
	const char *s = tlsh.getHash(showvers);
	if (*s == '\0')
		return Py_BuildValue("s", "TNULL");
	return Py_BuildValue("s", s);
}

// hash(data) returns byte buffer
static PyObject* hash_py(PyObject* self, PyObject* args)
{
	unsigned char* pBuffer;
	Py_ssize_t len;
	if (!PyArg_ParseTuple(args, BYTES_VALUE_CHAR "#", &pBuffer, &len)) {
		return NULL;
	}
	return (eval_tlsh(pBuffer, len, SHOWVERSION));
}

// oldhash(data) returns byte buffer
static PyObject* oldhash_py(PyObject* self, PyObject* args) {
	unsigned char* pBuffer;
	Py_ssize_t len;
	if (!PyArg_ParseTuple(args, BYTES_VALUE_CHAR "#", &pBuffer, &len)) {
		return NULL;
	}
	return (eval_tlsh(pBuffer, len, 0));
}

// forcehash(data) returns byte buffer
static PyObject* forcehash_py(PyObject* self, PyObject* args)
{
	return ( hash_py(self, args) );
}


static PyObject* eval_cons_tlsh(unsigned char* pBuffer, Py_ssize_t len, int showvers)
{
Tlsh tlsh;
	tlsh.update(pBuffer, len);
	tlsh.final(NULL, 0, 2);
	const char *s = tlsh.getHash(showvers);
	if (*s == '\0')
		return Py_BuildValue("s", "TNULL");
	return Py_BuildValue("s", s);
}

// conservativehash(data) returns byte buffer
static PyObject* conservativehash_py(PyObject* self, PyObject* args)
{
	unsigned char* pBuffer;
	Py_ssize_t len;
	if (!PyArg_ParseTuple(args, BYTES_VALUE_CHAR "#", &pBuffer, &len)) {
		return NULL;
	}
	return (eval_cons_tlsh(pBuffer, len, SHOWVERSION));
}

// oldconservativehash(data) returns byte buffer
static PyObject* oldconservativehash_py(PyObject* self, PyObject* args)
{
	unsigned char* pBuffer;
	Py_ssize_t len;
	if (!PyArg_ParseTuple(args, BYTES_VALUE_CHAR "#", &pBuffer, &len)) {
		return NULL;
	}
	return (eval_cons_tlsh(pBuffer, len, 0));
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
  { "oldhash", oldhash_py, METH_VARARGS, tlsh_oldhash_doc },
  { "forcehash", forcehash_py, METH_VARARGS, tlsh_forcehash_doc },
  { "conservativehash", conservativehash_py, METH_VARARGS, tlsh_conservativehash_doc },
  { "oldconservativehash", oldconservativehash_py, METH_VARARGS, tlsh_oldconservativehash_doc },
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
static PyObject * Tlsh_lvalue(tlsh_TlshObject *);
static PyObject * Tlsh_q1ratio(tlsh_TlshObject *);
static PyObject * Tlsh_q2ratio(tlsh_TlshObject *);
static PyObject * Tlsh_is_valid(tlsh_TlshObject *);
static PyObject * Tlsh_checksum(tlsh_TlshObject *, PyObject *);
static PyObject * Tlsh_bucket_value(tlsh_TlshObject *, PyObject *);

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
    {"checksum", (PyCFunction) Tlsh_checksum, METH_VARARGS,
     "TLSH checksum."
    },
    {"bucket_value", (PyCFunction) Tlsh_bucket_value, METH_VARARGS,
     "TLSH bucket value."
    },
    {NULL} /* Sentinel */
};

static PyGetSetDef Tlsh_getsetters[] = {
    {"lvalue", (getter) Tlsh_lvalue, NULL,
     "TLSH Lvalue.", NULL
    },
    {"q1ratio", (getter) Tlsh_q1ratio, NULL,
     "TLSH Q1ratio.", NULL
    },
    {"q2ratio", (getter) Tlsh_q2ratio, NULL,
     "TLSH Q2ratio.", NULL
    },
    {"is_valid", (getter) Tlsh_is_valid, NULL,
     "Is it a valid TLSH.", NULL
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
    Tlsh_getsetters,           /* tp_getset */
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

    if ((len != TLSH_STRING_LEN_REQ) && (len != TLSH_STRING_LEN_REQ-2)) {
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
    char hash[TLSH_STRING_LEN_REQ + 1];

    if (!self->finalized) {
        PyErr_SetString(PyExc_ValueError, "final() has not been called");
        return NULL;
    }
    self->tlsh.getHash(hash, TLSH_STRING_LEN_REQ + 1, SHOWVERSION);
    if (hash[0] == '\0') {
        PyErr_SetString(PyExc_ValueError, "error while getting hash (not enough variation in input?)");
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
      if ((len != TLSH_STRING_LEN_REQ) && (len != TLSH_STRING_LEN_REQ-2)) {
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

static PyObject *
Tlsh_lvalue(tlsh_TlshObject *self)
{
    if (!self->finalized) {
        PyErr_SetString(PyExc_ValueError, "final() has not been called");
        return NULL;
    }
    return Py_BuildValue("i", self->tlsh.Lvalue());
}

static PyObject *
Tlsh_q1ratio(tlsh_TlshObject *self)
{
    if (!self->finalized) {
        PyErr_SetString(PyExc_ValueError, "final() has not been called");
        return NULL;
    }
    return Py_BuildValue("i", self->tlsh.Q1ratio());
}

static PyObject *
Tlsh_q2ratio(tlsh_TlshObject *self)
{
    if (!self->finalized) {
        PyErr_SetString(PyExc_ValueError, "final() has not been called");
        return NULL;
    }
    return Py_BuildValue("i", self->tlsh.Q2ratio());
}

static PyObject *
Tlsh_is_valid(tlsh_TlshObject *self)
{
    return PyBool_FromLong(self->tlsh.isValid());
}

static PyObject *
Tlsh_checksum(tlsh_TlshObject *self, PyObject *args)
{
    int id;
    if (!self->finalized) {
        PyErr_SetString(PyExc_ValueError, "final() has not been called");
        return NULL;
    }
    PyArg_ParseTuple(args, "i", &id);

    return Py_BuildValue("i", self->tlsh.Checksum(id));
}

static PyObject *
Tlsh_bucket_value(tlsh_TlshObject *self, PyObject *args)
{
    int id;
    if (!self->finalized) {
        PyErr_SetString(PyExc_ValueError, "final() has not been called");
        return NULL;
    }
    PyArg_ParseTuple(args, "i", &id);

    return Py_BuildValue("i", self->tlsh.BucketValue(id));
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
