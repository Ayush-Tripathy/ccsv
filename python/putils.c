#include "putils.h"

int Is_FileOpenInPy(PyObject *py_file)
{
  // Ensure the object is file-like
  if (!PyObject_HasAttrString(py_file, "closed"))
  {
    PyErr_SetString(PyExc_TypeError, "Expected a file-like object");
    return 0; // Not a file-like object
  }

  // Access the 'closed' attribute
  PyObject *closed_attr = PyObject_GetAttrString(py_file, "closed");
  if (!closed_attr)
  {
    PyErr_SetString(PyExc_RuntimeError, "Unable to access 'closed' attribute");
    return 0;
  }

  int is_closed = PyObject_IsTrue(closed_attr);
  Py_DECREF(closed_attr);

  if (is_closed)
  {
    PyErr_SetString(PyExc_ValueError, "The file is closed");
    return 0; // File is closed
  }

  return 1; // File is open
}

FILE *Get_CFileFromPythonFile(PyObject *py_file, const char *mode)
{
  // Ensure the object has 'fileno'
  if (!PyObject_HasAttrString(py_file, "fileno"))
  {
    PyErr_SetString(PyExc_TypeError, "The object does not have a 'fileno' method");
    return NULL;
  }

  // Call the 'fileno' method
  PyObject *fileno_method = PyObject_GetAttrString(py_file, "fileno");
  if (!fileno_method || !PyCallable_Check(fileno_method))
  {
    PyErr_SetString(PyExc_RuntimeError, "Unable to call 'fileno' method");
    Py_XDECREF(fileno_method);
    return NULL;
  }

  PyObject *result = PyObject_CallObject(fileno_method, NULL);
  Py_DECREF(fileno_method);

  if (!result)
  {
    PyErr_SetString(PyExc_RuntimeError, "Call to 'fileno' failed");
    return NULL;
  }

  int fd = (int)PyLong_AsLong(result);
  Py_DECREF(result);

  if (PyErr_Occurred())
  {
    PyErr_SetString(PyExc_RuntimeError, "Invalid file descriptor");
    return NULL;
  }

  // C FILE *
  FILE *c_file = fdopen(fd, mode);
  if (!c_file)
  {
    PyErr_SetString(PyExc_RuntimeError, "Failed to associate file descriptor with FILE *");
    return NULL;
  }

  return c_file;
}

const char *Get_PythonFileMode(PyObject *py_file)
{
  // Check if the file-like object has a 'mode' attribute
  if (!PyObject_HasAttrString(py_file, "mode"))
  {
    PyErr_SetString(PyExc_TypeError, "Expected a file-like object with a 'mode' attribute");
    return NULL;
  }

  // Get the 'mode' attribute
  PyObject *mode_attr = PyObject_GetAttrString(py_file, "mode");
  if (!mode_attr)
  {
    PyErr_SetString(PyExc_RuntimeError, "Unable to access 'mode' attribute");
    return NULL;
  }

  // Ensure the attribute is a string
  if (!PyUnicode_Check(mode_attr))
  {
    Py_DECREF(mode_attr);
    PyErr_SetString(PyExc_TypeError, "'mode' attribute is not a string");
    return NULL;
  }

  // Convert the Python string to a C string
  const char *mode = PyUnicode_AsUTF8(mode_attr);
  Py_DECREF(mode_attr);

  if (!mode)
  {
    PyErr_SetString(PyExc_RuntimeError, "Failed to convert 'mode' to C string");
    return NULL;
  }

  return mode;
}
