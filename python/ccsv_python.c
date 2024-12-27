#define PY_SSIZE_T_CLEAN

#include <stdio.h>
#include <stdlib.h>
#include <Python.h>

#include "ccsv.h"
#include "putils.h"
#include "ccsv_python.h"

static PyObject *Reader_Free(Reader *self)
{
  ccsv_close(self->_reader);
  Py_TYPE(self)->tp_free((PyObject *)self);
  Py_RETURN_NONE;
}

static PyObject *Reader_Str(PyObject *self)
{
  return PyUnicode_FromString("CSV Reader object");
}

static PyObject *CCSV_Open(PyObject *self, PyObject *args)
{
  const char *filename;
  const char *mode;
  ccsv_reader_options options = {
      .delim = ',',
      .quote_char = '"',
      .skip_comments = 1,
      .skip_initial_space = 0,
      .skip_empty_lines = 0,
  };

  if (!PyArg_ParseTuple(args, "ss", &filename, &mode))
    return NULL;

  ccsv_reader *reader = ccsv_open(filename, CCSV_READER, mode, &options, NULL);

  if (reader == NULL)
  {
    PyErr_SetString(PyExc_RuntimeError, "Error initializing CSV reader");
    return NULL;
  }

  return PyCapsule_New((void *)reader, "ccsv_reader", NULL);
}

static PyObject *CCSVReader_Next(PyObject *self, PyObject *args)
{
  ccsv_reader *reader = ((Reader *)self)->_reader;
  ccsv_row *row;

  if (reader == NULL)
  {
    PyErr_SetString(PyExc_RuntimeError, "Invalid CSV reader");
    return NULL;
  }

  row = ccsv_next(reader);

  if (row == NULL)
  {
    Py_RETURN_NONE;
  }

  PyObject *list = PyList_New(row->fields_count);

  for (int i = 0; i < row->fields_count; i++)
  {
    PyList_SetItem(list, i, PyUnicode_FromString(row->fields[i]));
  }

  ccsv_free_row(row);

  return list;
}

static PyObject *CCSV_Close(PyObject *self, PyObject *args)
{
  PyObject *reader_capsule;
  ccsv_reader *reader;

  if (!PyArg_ParseTuple(args, "O", &reader_capsule))
    return NULL;

  reader = (ccsv_reader *)PyCapsule_GetPointer(reader_capsule, "ccsv_reader");
  if (reader == NULL)
  {
    PyErr_SetString(PyExc_RuntimeError, "Invalid CSV reader");
    return NULL;
  }

  ccsv_close(reader);
  Py_RETURN_NONE;
}

static PyObject *Reader_Iter(PyObject *self)
{
  Py_INCREF(self);
  return self;
}

static PyObject *Reader_Next(PyObject *self)
{
  PyObject *row = CCSVReader_Next(self, NULL);
  if (row == Py_None)
  {
    return NULL;
  }

  return row;
}

static ccsv_reader *CCSV_ReaderFromFP(PyObject *args, PyObject *kwds)
{
  PyObject *py_file;
  char delim = ',';           // Default delimiter
  char quote_char = '"';      // Default quote character
  int skip_comments = 1;      // Default skip comments
  int skip_initial_space = 0; // Default skip initial space
  int skip_empty_lines = 0;   // Default skip empty lines

  static char *kwlist[] = {"file", "delim", "quote_char", "skip_comments", "skip_initial_space", "skip_empty_lines", NULL};

  // if (!PyArg_ParseTuple(args, "O|ssbbb", &py_file, &delim, &quote_char, &skip_comments, &skip_initial_space, &skip_empty_lines))
  //   return NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|CCppp", kwlist, &py_file, &delim, &quote_char, &skip_comments, &skip_initial_space, &skip_empty_lines))
    return NULL;

  if (!Is_FileOpenInPy(py_file))
    return NULL;

  const char *mode = Get_PythonFileMode(py_file);

  FILE *fp = Get_CFileFromPythonFile(py_file, mode);
  if (fp == NULL)
    return NULL;

  ccsv_reader_options options = {
      .delim = delim,
      .quote_char = quote_char,
      .skip_comments = skip_comments,
      .skip_initial_space = skip_initial_space,
      .skip_empty_lines = skip_empty_lines,
  };

  ccsv_reader *reader = ccsv_open_from_file(fp, CCSV_READER, mode, &options, NULL);
  if (reader == NULL)
  {
    PyErr_SetString(PyExc_RuntimeError, "Error initializing CSV reader");
    return NULL;
  }

  return reader;
}

static ccsv_reader *Reader_Create(PyObject *args, PyObject *kwds)
{
  return CCSV_ReaderFromFP(args, kwds);
}

// __init__ method in Python
static int Reader_Init(Reader *self, PyObject *args, PyObject *kwds)
{

  ccsv_reader *reader = Reader_Create(args, kwds);
  if (reader == NULL)
  {
    return -1;
  }

  if (self->_reader != NULL)
  {
    ccsv_close(self->_reader);
  }

  self->_reader = reader;
  return 0;
}

// __new__ method in Python
static PyObject *Reader_New(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  Reader *self;

  self = (Reader *)type->tp_alloc(type, 0);

  if (self != NULL)
  {
    // TODO: Required initialization
    self->_reader = NULL;
  }

  return (PyObject *)self;
}

static PyGetSetDef ReaderGetSet[] = {
    {NULL} // Sentinel
};

static PyMethodDef ReaderMethods[] = {
    {"next", CCSVReader_Next, METH_VARARGS, "Get the next row"},
    {NULL, NULL, 0, NULL}};

PyTypeObject ReaderType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "ccsv.Reader",
    .tp_basicsize = sizeof(Reader),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "CSV Reader object",
    .tp_iter = Reader_Iter,     // __iter__ method in Python
    .tp_iternext = Reader_Next, // __next__ method in Python
    .tp_getset = ReaderGetSet,
    .tp_new = Reader_New,
    .tp_init = (initproc)Reader_Init, // __init__ method in Python
    .tp_str = Reader_Str,
    .tp_dealloc = (destructor)Reader_Free,
    .tp_methods = ReaderMethods,
    .tp_as_sequence = NULL,
    .tp_as_mapping = &(PyMappingMethods){
        .mp_length = NULL,
        .mp_subscript = NULL},
};

static PyMethodDef CcsvMethods[] = {
    {"ccsv_open", CCSV_Open, METH_VARARGS, "Open a CSV file"},
    {"ccsv_close", CCSV_Close, METH_VARARGS, "Close the CSV file"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef ccsvmodule = {
    PyModuleDef_HEAD_INIT,
    "ccsv",
    NULL,
    -1,
    CcsvMethods};

PyMODINIT_FUNC PyInit_ccsv(void)
{

  PyObject *m;
  if (PyType_Ready(&ReaderType) < 0)
  {
    return NULL;
  }

  m = PyModule_Create(&ccsvmodule);
  if (m == NULL)
  {
    return NULL;
  }

  Py_INCREF(&ReaderType);
  if (PyModule_AddObject(m, "Reader", (PyObject *)&ReaderType) < 0)
  {
    Py_DECREF(&ReaderType);
    Py_DECREF(m);
    return NULL;
  }

  return m;
}
