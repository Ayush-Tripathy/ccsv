#pragma once

#include <Python.h>
#include <stdio.h>

/**
 * @brief Checks if a file-like object is open
 *
 * @param py_file
 * @return int
 */
int Is_FileOpenInPy(PyObject *py_file);

/**
 * @brief Converts a Python file-like object to a C FILE *
 *
 * @param py_file
 * @param mode
 * @return FILE*
 */
FILE *Get_CFileFromPythonFile(PyObject *py_file, const char *mode);

/**
 * @brief Gets the mode of a Python file-like object as a C string
 *
 * @param py_file
 * @return const char*
 */
const char *Get_PythonFileMode(PyObject *py_file);