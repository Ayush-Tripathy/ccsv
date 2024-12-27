#pragma once

#include <Python.h>
// #include <structmember.h>

#include "ccsv.h"

typedef struct Reader
{
  PyObject_HEAD ccsv_reader *_reader;
} Reader;

extern PyTypeObject ReaderType;