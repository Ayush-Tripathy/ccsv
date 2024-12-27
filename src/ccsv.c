// File: ccsv.c

// Created: 2023 by Ayush Tripathy
// github.com/Ayush-Tripathy

/*
 * This library provides functions to handle reading, writing csv files.
 */

/*
    MIT License

    Copyright (c) 2023 Ayush Tripathy

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

// ccsv header file
#include "ccsv.h"

  const char *status_messages[] = {
      "Success",
      "Error",
      "Memory allocation failure.",
      "Malformed CSV file.",
      "Not started writing, CSV_WRITE_ROW_START() not called.",
      "Already writing field, CSV_WRITE_ROW_START() already called."};

  const char *ccsv_get_status_message(short status)
  {
    if (status > 0)
      return status_messages[0];

    if (status < -5)
      return NULL;
    return status_messages[-1 * status];
  }

  int _get_object_type(void *obj)
  {
    if (obj == NULL)
      return CCSV_NULL_CHAR;

    if (((ccsv_reader *)obj)->object_type == CCSV_READER)
      return CCSV_READER;

    if (((ccsv_writer *)obj)->object_type == CCSV_WRITER)
      return CCSV_WRITER;

    return CCSV_NULL_CHAR;
  }

  int ccsv_is_error(void *obj, short *status)
  {
    if (obj == NULL)
      return 0;

    if (_get_object_type(obj) == CCSV_READER)
    {
      short __status = ((ccsv_reader *)obj)->status;
      if (status != NULL)
        *status = __status;
      return __status < 0;
    }

    if (_get_object_type(obj) == CCSV_WRITER)
    {
      short __status = ((ccsv_writer *)obj)->write_status;
      if (status != NULL)
        *status = __status;
      return __status < 0;
    }

    return 0;
  }

/* Reader */

// These macros should be used only in read_row() function
#define ADD_FIELD(field)                                            \
  field[field_pos++] = CCSV_NULL_CHAR;                              \
  fields_count++;                                                   \
  fields = (char **)realloc(fields, sizeof(char *) * fields_count); \
  if (fields == NULL)                                               \
  {                                                                 \
    _free_multiple(3, field, row_string, row);                      \
    reader->status = CCSV_ERNOMEM;                                  \
    return NULL;                                                    \
  }                                                                 \
  fields[fields_count - 1] = field;

#define GROW_FIELD_BUFFER_IF_NEEDED(field, field_size, field_pos) \
  if (field_pos > field_size - 1)                                 \
  {                                                               \
    field_size += MAX_FIELD_SIZE;                                 \
    field = (char *)realloc(field, field_size + 1);               \
    if (field == NULL)                                            \
    {                                                             \
      _free_multiple(3, fields, row_string, row);                 \
      reader->status = CCSV_ERNOMEM;                              \
      return NULL;                                                \
    }                                                             \
  }

#define GROW_ROW_BUFFER_IF_NEEDED(row_string, row_len, row_pos)    \
  if (row_pos > row_len - 1)                                       \
  {                                                                \
    row_string_size += reader->__buffer_size;                      \
    row_string = (char *)realloc(row_string, row_string_size + 1); \
    if (row_string == NULL)                                        \
    {                                                              \
      _free_multiple(2, fields, row);                              \
      reader->status = CCSV_ERNOMEM;                               \
      return NULL;                                                 \
    }                                                              \
  }

#define RETURN_IF_WRITE_ERROR(writer, desired_status) \
  if (writer->write_status != desired_status)         \
    return writer->write_status;

  ccsv_reader *ccsv_init_reader(ccsv_reader_options *options, short *status)
  {
    char delim, quote_char, comment_char, escape_char;
    int skip_initial_space, skip_empty_lines, skip_comments;
    if (options == NULL)
    {
      delim = DEFAULT_DELIMITER;
      quote_char = DEFAULT_QUOTE_CHAR;
      comment_char = DEFAULT_COMMENT_CHAR;
      escape_char = DEFAULT_ESCAPE_CHAR;
      skip_initial_space = 0;
      skip_empty_lines = 0;
      skip_comments = 0;
    }
    else
    {
      // It is not mandatory to pass all options to options struct
      // So check if the option is passed or not, if not then use the default value
      if (options->delim == CCSV_NULL_CHAR)
        delim = DEFAULT_DELIMITER;

      else
        delim = options->delim;

      if (options->quote_char == CCSV_NULL_CHAR)
        quote_char = DEFAULT_QUOTE_CHAR;

      else
        quote_char = options->quote_char;

      if (options->comment_char == CCSV_NULL_CHAR)
        comment_char = DEFAULT_COMMENT_CHAR;

      else
        comment_char = options->comment_char;

      if (options->escape_char == CCSV_NULL_CHAR)
        escape_char = DEFAULT_ESCAPE_CHAR;

      else
        escape_char = options->escape_char;

      if (options->skip_initial_space == CCSV_NULL_CHAR)
        skip_initial_space = 0;

      else
        skip_initial_space = options->skip_initial_space;

      if (options->skip_empty_lines == CCSV_NULL_CHAR)
        skip_empty_lines = 0;

      else
        skip_empty_lines = options->skip_empty_lines;

      if (options->skip_comments == CCSV_NULL_CHAR)
        skip_comments = 0;

      else
        skip_comments = options->skip_comments;
    }

    // Parser
    ccsv_reader *parser = (ccsv_reader *)malloc(sizeof(ccsv_reader));
    if (parser == NULL)
    {
      if (status != NULL)
        *status = CCSV_ERNOMEM;
      return NULL;
    }
    parser->__delim = delim;
    parser->__quote_char = quote_char;
    parser->__comment_char = comment_char;
    parser->__escape_char = escape_char;
    parser->__skip_initial_space = skip_initial_space;
    parser->__skip_empty_lines = skip_empty_lines;
    parser->__skip_comments = skip_comments;

    parser->__fp = NULL;

    parser->rows_read = 0;
    parser->status = CCSV_SUCCESS;
    parser->object_type = CCSV_READER;

    return parser;
  }

  void ccsv_free_row(ccsv_row *row)
  {
    const int fields_count = row->fields_count;
    for (int i = 0; i < fields_count; i++)
    {
      free(row->fields[i]);
    }
    free(row->fields);
    free(row);
  }

  void _free_multiple(int num, ...)
  {
    va_list args;
    va_start(args, num);

    for (int i = 0; i < num; ++i)
    {
      void *ptr = va_arg(args, void *);
      free(ptr);
    }

    va_end(args);
  }

  void *ccsv_open(const char *filename, short object_type, const char *mode, void *options, short *status)
  {
    if (filename == NULL)
      return NULL;

    FILE *fp = fopen(filename, mode);
    if (fp == NULL)
    {
      if (status != NULL)
        *status = CCSV_EROPEN;
      return NULL;
    }

    void *obj = ccsv_open_from_file(fp, object_type, mode, options, status);
    if (obj == NULL)
    {
      fclose(fp);
      return NULL;
    }

    return obj;
  }

  void *ccsv_open_from_file(FILE *fp, short object_type, const char *mode, void *options, short *status)
  {
    if (fp == NULL)
      return NULL;

    if (object_type != CCSV_READER && object_type != CCSV_WRITER)
    {
      if (status != NULL)
        *status = CCSV_ERINVOBJTYPE;
      return NULL;
    }

    if (strcmp(mode, "r") != 0 &&
        strcmp(mode, "rb") != 0 &&
        strcmp(mode, "r+") != 0 &&
        strcmp(mode, "rb+") != 0 &&
        strcmp(mode, "w+") != 0 &&
        strcmp(mode, "a+") != 0)
    {
      if (status != NULL)
        *status = CCSV_ERMODE;
      return NULL;
    }

    if (object_type == CCSV_READER)
    {
      short init_status;

#ifdef __cplusplus
      ccsv_reader_options *reader_options = reinterpret_cast<ccsv_reader_options *>(options);
      ccsv_reader *reader = ccsv_init_reader(reader_options, &init_status);
#else
    options = (ccsv_reader_options *)options;
    ccsv_reader *reader = ccsv_init_reader(options, &init_status);
#endif

      if (init_status == CCSV_ERNOMEM || reader == NULL)
      {
        if (status != NULL)
          *status = CCSV_ERNOMEM;
        return NULL;
      }

      if (fp == NULL)
      {
        if (status != NULL)
          *status = CCSV_EROPEN;
        return NULL;
      }

      size_t buffer_size = CCSV_BUFFER_SIZE;

      size_t file_size;

      fseek(fp, 0, SEEK_END);
      file_size = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      reader->__file_size = file_size;
      reader->__file_pos = 0;

      if (file_size >= CCSV_LARGE_FILE_SIZE)
        buffer_size = CCSV_HIGH_BUFFER_SIZE;
      else if (file_size >= CCSV_MED_FILE_SIZE)
        buffer_size = CCSV_MED_BUFFER_SIZE;
      else
        buffer_size = CCSV_LOW_BUFFER_SIZE;

      reader->__buffer = (char *)malloc(buffer_size + 1);
      if (reader->__buffer == NULL)
      {
        free(reader);
        if (status != NULL)
          *status = CCSV_ERNOMEM;
        return NULL;
      }
      reader->__buffer[0] = CCSV_NULL_CHAR;

      reader->__buffer_size = buffer_size;
      // reader->__buffer_allocated = true;

      reader->__fp = fp;
      reader->object_type = object_type;

      return reader;
    }
    else if (object_type == CCSV_WRITER)
    {
      if (fp == NULL)
      {
        if (status != NULL)
          *status = CCSV_EROPEN;
        return NULL;
      }

      short init_status;

#ifdef __cplusplus
      ccsv_writer_options *writer_options = reinterpret_cast<ccsv_writer_options *>(options);
      ccsv_writer *writer = ccsv_init_writer(writer_options, &init_status);
#else
    options = (ccsv_writer_options *)options;
    ccsv_writer *writer = ccsv_init_writer(options, &init_status);
#endif
      if (init_status == CCSV_ERNOMEM || writer == NULL)
      {
        if (status != NULL)
          *status = CCSV_ERNOMEM;
        return NULL;
      }
      writer->__fp = fp;
      writer->object_type = object_type;

      return writer;
    }
    return NULL;
  }

  void ccsv_close(void *obj)
  {
    if (obj == NULL)
      return;

    if (_get_object_type(obj) == CCSV_READER)
    {
      ccsv_reader *reader = (ccsv_reader *)obj;
      fclose(reader->__fp);
      free(reader);
    }
    else if (_get_object_type(obj) == CCSV_WRITER)
    {
      ccsv_writer *writer = (ccsv_writer *)obj;
      fclose(writer->__fp);
      free(writer);
    }
    else
    {
      return;
    }
  }

  ccsv_row *ccsv_next(ccsv_reader *reader)
  {
    if (reader == NULL)
      return NULL;

    if (reader->__buffer == NULL)
    {
      reader->status = CCSV_ERBUFNTALLOC;
      return NULL;
    }

    if (reader->__fp == NULL)
    {
      reader->status = CCSV_ERNULLFP;
      return NULL;
    }

    return _next(reader->__fp, reader);
  }

  ccsv_row *read_row(FILE *fp, ccsv_reader *reader)
  {
    if (fp == NULL)
    {
      reader->status = CCSV_ERNULLFP;
      return NULL;
    }
    return _read_row(fp, reader);
  }

  ccsv_row *_read_row(FILE *fp, ccsv_reader *reader)
  {
    ccsv_row *row = (ccsv_row *)malloc(sizeof(ccsv_row));
    if (row == NULL)
    {
      reader->status = CCSV_ERNOMEM;
      return NULL;
    }

    const char DELIM = reader->__delim;
    const char QUOTE_CHAR = reader->__quote_char;
    const char COMMENT_CHAR = reader->__comment_char;
    const int SKIP_INITIAL_SPACE = reader->__skip_initial_space;
    const int SKIP_EMPTY_LINES = reader->__skip_empty_lines;
    const int SKIP_COMMENTS = reader->__skip_comments;

    State state = FIELD_START;

    char *row_string = (char *)malloc(CCSV_BUFFER_SIZE + 1);
    if (row_string == NULL)
    {
      _free_multiple(1, row);
      reader->status = CCSV_ERNOMEM;
      return NULL;
    }
    size_t row_string_size = CCSV_BUFFER_SIZE;
    size_t row_pos = 0;

    char **fields = (char **)malloc(sizeof(char *));
    if (fields == NULL)
    {
      _free_multiple(2, row_string, row);
      reader->status = CCSV_ERNOMEM;
      return NULL;
    }
    size_t fields_count = 0;

    char *field = (char *)malloc(MAX_FIELD_SIZE + 1);
    if (field == NULL)
    {
      _free_multiple(3, row_string, fields, row);
      reader->status = CCSV_ERNOMEM;
      return NULL;
    }
    size_t field_size = MAX_FIELD_SIZE;
    size_t field_pos = 0;

    int inside_quotes = 0;

    size_t row_len;

  readfile:
    if (fgets(row_string, CCSV_BUFFER_SIZE, fp) == NULL)
    {
      /* If fields is not empty then return the row */
      if (fields_count > 0 || field_pos > 0)
      {
        /* Add the last field */

        /*
         * If fields_count > 0:
         * This happens when the function holding the values of last row
         * but yet to return the last row
         *
         * if field_pos > 0:
         * This only happens when there is a single element in the last row and also
         * there is no line after the current line
         * So we need to add the only field of the last row
         */
        ADD_FIELD(field);
        goto end;
      }

      _free_multiple(4, row_string, field, fields, row);
      return NULL;
    }

    row_len = strlen(row_string);
    row_pos = 0;

    while (1)
    {
      char c = row_string[row_pos];
      switch (state)
      {
      case FIELD_START:
        if (c == QUOTE_CHAR)
        {
          /* Start of quoted field */
          state = INSIDE_QUOTED_FIELD;
        }
        else if (SKIP_INITIAL_SPACE && c == CCSV_SPACE)
        {
          /* Skip initial spaces */
          state = FIELD_NOT_STARTED;
        }
        else if (c == DELIM || c == CCSV_CR || c == CCSV_LF)
        {
          /* Return empty field or empty row */
          state = FIELD_END;
          row_pos--;
        }
        else
        {
          state = FIELD_STARTED;
          field[field_pos++] = c;
        }
        break;

      case INSIDE_QUOTED_FIELD:
        inside_quotes = 1;
        if (c == QUOTE_CHAR)
        {
          /* Might be the end of the field, or it might be a escaped quote */
          state = MAY_BE_ESCAPED;
        }
        else
        {
          state = FIELD_STARTED;
          field[field_pos++] = c;
        }
        break;

      case FIELD_NOT_STARTED:
        if (c == QUOTE_CHAR)
        {
          /* Start of quoted field */
          state = INSIDE_QUOTED_FIELD;
        }
        else if (c == DELIM)
        {
          /* Return empty field */
          row_pos--;
          state = FIELD_END;
        }
        else if (c == CCSV_SPACE)
        {
          /*
           * Skip initial spaces, will only get to this point if
           * skip_initial_spaces = 1
           */
          state = FIELD_NOT_STARTED;
        }
        else
        {
          /* Start of non-quoted field */
          state = FIELD_STARTED;
          field[field_pos++] = c;
        }
        break;

      case FIELD_STARTED:
        GROW_FIELD_BUFFER_IF_NEEDED(field, field_size, field_pos);
        GROW_ROW_BUFFER_IF_NEEDED(row_string, row_len, row_pos);

        if (c == QUOTE_CHAR && inside_quotes)
        {
          /* Might be the end of the field, or it might be a escaped quote */
          state = MAY_BE_ESCAPED;
        }
        else if (c == QUOTE_CHAR && !inside_quotes)
        {
          /* Add the quote char if not at the start of field */
          state = FIELD_STARTED;
          field[field_pos++] = c;
        }
        else if ((c == DELIM && !inside_quotes) ||
                 ((c == CCSV_LF || c == CCSV_CR) && !inside_quotes) ||
                 (c == CCSV_NULL_CHAR))
        {
          /* End of field */
          state = FIELD_END;
          row_pos--;
        }
        else
        {
          /* Add the character to the field */
          state = FIELD_STARTED;
          field[field_pos++] = c;
        }
        break;

      case MAY_BE_ESCAPED:
        if (c == QUOTE_CHAR)
        {
          /* Escaped quote */
          state = FIELD_STARTED;
          field[field_pos++] = c;
        }
        else
        {
          /* End of field */
          inside_quotes = 0;
          state = FIELD_STARTED;
          row_pos--;
        }

        break;

      case FIELD_END:
        state = FIELD_START;
        if (SKIP_EMPTY_LINES &&
            fields_count == 0 &&
            field_pos == 0 &&
            (c == CCSV_CR || c == CCSV_LF || c == CCSV_NULL_CHAR) &&
            !inside_quotes)
        {
          /* Do not return empty lines, parse again */
          goto readfile;
        }
        else if (SKIP_COMMENTS &&
                 fields_count == 0 &&
                 field_pos > 0 &&
                 field[0] == COMMENT_CHAR &&
                 !inside_quotes)
        {
          /* Do not return comment lines, parse again */
          field_pos = 0;
          goto readfile;
        }
        else
        {
          ADD_FIELD(field);
        }
        field = (char *)malloc(MAX_FIELD_SIZE + 1);
        field_size = MAX_FIELD_SIZE;
        if (field == NULL)
        {
          _free_multiple(3, fields, row_string, row);
          reader->status = CCSV_ERNOMEM;
          return NULL;
        }
        field_pos = 0;

        if (c == CCSV_CR || c == CCSV_LF || c == CCSV_NULL_CHAR)
        {
          free(field);
          goto end;
        }
        break;

      default:
        break;
      }

      row_pos++;

      if (row_pos > row_len - 1)
      {
        goto readfile;
      }
    }

  end:
    row->fields = fields;
    row->fields_count = fields_count;
    free(row_string);

    reader->rows_read++;
    reader->status = CCSV_SUCCESS;
    return row;
  }

#define IS_TERMINATOR(c) (c == CCSV_CR || c == CCSV_LF || c == CCSV_NULL_CHAR)

  ccsv_row *_next(FILE *fp, ccsv_reader *reader)
  {
    ccsv_row *row = (ccsv_row *)malloc(sizeof(ccsv_row));
    if (row == NULL)
    {
      reader->status = CCSV_ERNOMEM;
      return NULL;
    }

    const char DELIM = reader->__delim;
    const char QUOTE_CHAR = reader->__quote_char;
    const char COMMENT_CHAR = reader->__comment_char;
    const char ESCAPE_CHAR = reader->__escape_char;
    const int SKIP_INITIAL_SPACE = reader->__skip_initial_space;
    const int SKIP_EMPTY_LINES = reader->__skip_empty_lines;
    const int SKIP_COMMENTS = reader->__skip_comments;

    size_t buffer_size = reader->__buffer_size;

    State state = FIELD_START;

    char *row_string = reader->__buffer;
    size_t row_pos = 0;

    char **fields = (char **)malloc(sizeof(char *));
    if (fields == NULL)
    {
      _free_multiple(2, row_string, row);
      reader->status = CCSV_ERNOMEM;
      return NULL;
    }

    size_t fields_count = 0;

    char *field = (char *)malloc(MAX_FIELD_SIZE + 1);
    if (field == NULL)
    {
      _free_multiple(3, row_string, fields, row);
      reader->status = CCSV_ERNOMEM;
      return NULL;
    }

    size_t field_size = MAX_FIELD_SIZE;
    size_t field_pos = 0;

    size_t bytes_read;
  readfile:

    /* Checking buffer */
    if (_is_buffer_empty(reader))
    {
      bytes_read = fread(reader->__buffer, sizeof(char), buffer_size, fp);
      reader->__file_pos += bytes_read;

      if (bytes_read <= 0)
      {
        if (fields_count > 0 || field_pos > 0)
        {
          /* Add the last field */

          /*
           * If fields_count > 0:
           * This happens when the function holding the values of last row
           * but yet to return the last row
           *
           * if field_pos > 0:
           * This only happens when there is a single element in the last row and also
           * there is no line after the current line
           * So we need to add the only field of the last row
           */
          ADD_FIELD(field);
          goto end;
        }

        _free_multiple(4, row_string, field, fields, row);
        return NULL;
      }

      reader->__buffer[bytes_read] = CCSV_NULL_CHAR;
      reader->__buffer_size = bytes_read;
      reader->__buffer_pos = 0;
      row_pos = 0;
      row_string = reader->__buffer;

      if (IS_TERMINATOR(row_string[row_pos]) && state == FIELD_START)
        row_pos++;
    }
    else
    {
      row_string = reader->__buffer;
      bytes_read = reader->__buffer_size;
      row_pos = reader->__buffer_pos;
    }

    for (; row_pos < bytes_read;)
    {
      char c = row_string[row_pos++];

      switch (state)
      {
      case FIELD_START:
        if (c == QUOTE_CHAR)
          state = INSIDE_QUOTED_FIELD; /* Start of quoted field */
        else if (SKIP_INITIAL_SPACE && c == CCSV_SPACE)
          state = FIELD_NOT_STARTED; /* Skip initial spaces */
        else if (c == DELIM || IS_TERMINATOR(c))
        {
          state = FIELD_END; /* Empty field or empty row */
          row_pos--;
        }
        else
        {
          state = FIELD_STARTED;
          field[field_pos++] = c;
        }
        break;

      case INSIDE_QUOTED_FIELD:
        GROW_FIELD_BUFFER_IF_NEEDED(field, field_size, field_pos);
        if (c == QUOTE_CHAR)
          state = MAY_BE_ESCAPED; /* Might be the end of the field, or it might be a escaped quote */
        else if (c == ESCAPE_CHAR)
        {
          field[field_pos++] = c; /* Escaped escape character */
          row_pos++;
        }
        else
          field[field_pos++] = c;

        break;

      case MAY_BE_ESCAPED:
        if (c == QUOTE_CHAR)
        {
          state = INSIDE_QUOTED_FIELD; /* Escaped quote */
          field[field_pos++] = c;
        }
        else if (c == DELIM || IS_TERMINATOR(c))
        {
          state = FIELD_END; /* End of field */
          row_pos--;
        }
        else
        {
          state = FIELD_STARTED;
          field[field_pos++] = c;
        }

        break;

      case FIELD_NOT_STARTED:
        if (c == QUOTE_CHAR)
          state = INSIDE_QUOTED_FIELD; /* Start of quoted field */
        else if (c == DELIM)
        {
          state = FIELD_END; /* Return empty field */
          row_pos--;
        }
        else if (c == CCSV_SPACE)
          state = FIELD_NOT_STARTED; /* Skip initial spaces, will only get to this point if skip_initial_spaces = 1 */
        else
        {
          state = FIELD_STARTED;
          field[field_pos++] = c; /* Start of non-quoted field */
        }
        break;

      case FIELD_STARTED:
        GROW_FIELD_BUFFER_IF_NEEDED(field, field_size, field_pos);

        if (c == DELIM || IS_TERMINATOR(c))
        {
          state = FIELD_END; /* End of field */
          row_pos--;
        }
        else
          field[field_pos++] = c; /* Add the character to the field */
        break;

      case FIELD_END:
        state = FIELD_START;

        if (SKIP_EMPTY_LINES &&
            fields_count == 0 &&
            field_pos == 0 &&
            IS_TERMINATOR(c))
        {
          /* Do not return empty lines, parse again */
          reader->__buffer_pos = row_pos;
          goto readfile;
        }
        else if (SKIP_COMMENTS &&
                 fields_count == 0 &&
                 field_pos > 0 &&
                 field[0] == COMMENT_CHAR)
        {
          /* Do not return comment lines, parse again */
          field_pos = 0;
          reader->__buffer_pos = row_pos + 1;
          goto readfile;
        }
        else
        {
          ADD_FIELD(field);
        }

        field = (char *)malloc(MAX_FIELD_SIZE + 1);
        field_size = MAX_FIELD_SIZE;
        if (field == NULL)
        {
          _free_multiple(3, fields, row_string, row);
          reader->status = CCSV_ERNOMEM;
          return NULL;
        }
        field_pos = 0;

        if (IS_TERMINATOR(c)) /* CR or LF */
        {

          if (IS_TERMINATOR(row_string[row_pos])) /* CRLF */
            row_pos++;

          free(field);
          goto end;
        }
        break;

      default:
        break;
      }
    }

    // This point is reached only when for loop is completed fully
    if (row_pos > bytes_read - 1)
    {
      reader->__buffer[0] = CCSV_NULL_CHAR; /* Reset the buffer */
      goto readfile;
    }

  end:
    row->fields = fields;
    row->fields_count = fields_count;

    if (row_pos > bytes_read - 1)
      reader->__buffer[0] = CCSV_NULL_CHAR; /* Reset the buffer */
    else
      reader->__buffer_pos = row_pos;

    reader->rows_read++;
    reader->status = CCSV_SUCCESS;
    return row;
  }

  int _is_buffer_empty(ccsv_reader *reader)
  {
    return reader->__buffer[0] == CCSV_NULL_CHAR;
  }

  /* Writer */

  ccsv_writer *ccsv_init_writer(ccsv_writer_options *options, short *status)
  {
    char delim, quote_char, escape_char;
    WriterState state = WRITER_NOT_STARTED;
    if (options == NULL)
    {
      delim = DEFAULT_DELIMITER;
      quote_char = DEFAULT_QUOTE_CHAR;
      escape_char = DEFAULT_ESCAPE_CHAR;
    }
    else
    {
      // It is not mandatory to pass all options to options struct
      // So check if the option is passed or not, if not then use the default value
      if (options->delim == CCSV_NULL_CHAR)
        delim = DEFAULT_DELIMITER;

      else
        delim = options->delim;

      if (options->quote_char == CCSV_NULL_CHAR)
        quote_char = DEFAULT_QUOTE_CHAR;

      else
        quote_char = options->quote_char;

      if (options->escape_char == CCSV_NULL_CHAR)
        escape_char = DEFAULT_ESCAPE_CHAR;

      else
        escape_char = options->escape_char;
    }

    // Writer
    ccsv_writer *writer = (ccsv_writer *)malloc(sizeof(ccsv_writer));
    if (writer == NULL)
    {
      if (status != NULL)
        *status = CCSV_ERNOMEM;
      return NULL;
    }
    writer->__delim = delim;
    writer->__quote_char = quote_char;
    writer->__escape_char = escape_char;
    writer->__state = state;

    writer->write_status = WRITER_NOT_STARTED;
    writer->object_type = CCSV_WRITER;

    return writer;
  }

  int ccsv_write(ccsv_writer *writer, ccsv_row row)
  {
    if (writer == NULL)
      return WRITE_ERNOTSTARTED;

    if (writer->__fp == NULL)
      return CCSV_ERNULLFP;

    return write_row(writer->__fp, writer, row);
  }

  int ccsv_write_from_array(ccsv_writer *writer, char **fields, int fields_len)
  {
    if (writer == NULL)
      return WRITE_ERNOTSTARTED;

    if (writer->__fp == NULL)
      return CCSV_ERNULLFP;

    return write_row_from_array(writer->__fp, writer, fields, fields_len);
  }

  int write_row(FILE *fp, ccsv_writer *writer, ccsv_row row)
  {
    const int fields_count = row.fields_count;
    char **fields = row.fields;
    return (write_row_from_array(fp, writer, fields, fields_count));
  }

  int write_row_from_array(FILE *fp, ccsv_writer *writer, char **fields, int row_len)
  {
    CCSV_WRITE_ROW_START(fp, writer);
    RETURN_IF_WRITE_ERROR(writer, WRITE_STARTED);

    for (int i = 0; i < row_len; i++)
    {
      const char *field = fields[i];
      CCSV_WRITE_FIELD(fp, writer, field);
      RETURN_IF_WRITE_ERROR(writer, WRITE_SUCCESS);
    }
    CCSV_WRITE_ROW_END(fp, writer, NULL);
    RETURN_IF_WRITE_ERROR(writer, WRITE_ENDED);

    writer->write_status = WRITE_SUCCESS;
    return WRITE_SUCCESS;
  }

  int _write_row_start(FILE *fp, ccsv_writer *writer)
  {
    long file_size;
    char last_char;

    switch (writer->__state)
    {
    case WRITER_NOT_STARTED:
      writer->__state = WRITER_ROW_START; /* Start writing row */

      /* Move the file pointer to the end to get the file size */
      fseek(fp, 0, SEEK_END);
      file_size = ftell(fp);

      /* Move the pointer to the penultimate position */
      fseek(fp, -1, SEEK_END);

      last_char = fgetc(fp);

      if (last_char != CCSV_LF && last_char != CCSV_CR && file_size > 0)
      {
        fputc(CCSV_CR, fp);
        fputc(CCSV_LF, fp);
      }

      /* Rewind the file pointer */
      fseek(fp, -file_size, SEEK_END);
      break;

    case WRITER_ROW_END:
      writer->__state = WRITER_ROW_START; /* Start writing row */
      break;

    case WRITER_WRITING_FIELD:
    case WRITER_ROW_START:
      writer->__state = WRITER_ROW_END;
      writer->write_status = WRITE_ERALWRITING; /* Already writing field */
      return WRITE_ERALWRITING;

    default:
      break;
    }

    writer->write_status = WRITE_STARTED;
    return WRITE_STARTED;
  }

  int _write_row_end(FILE *fp, ccsv_writer *writer)
  {
    switch (writer->__state)
    {
    case WRITER_NOT_STARTED:
      writer->__state = WRITER_ROW_END;
      writer->write_status = WRITE_ERNOTSTARTED; /* Not started writing, CSV_WRITE_ROW_START() not called */
      return WRITE_ERNOTSTARTED;

    case WRITER_ROW_START:
    case WRITER_WRITING_FIELD:
      writer->__state = WRITER_ROW_END;
      fputc(CCSV_CR, fp);
      fputc(CCSV_LF, fp);
      break;

    case WRITER_ROW_END:
      writer->__state = WRITER_NOT_STARTED; /* Reset the state */
      break;

    default:
      break;
    }

    writer->write_status = WRITE_ENDED;
    return WRITE_ENDED;
  }

  int _write_field(FILE *fp, ccsv_writer *writer, const char *string)
  {
    WriterState state = writer->__state;
    if (state != WRITER_ROW_START && state != WRITER_WRITING_FIELD)
    {
      /* Not started writing, CSV_WRITE_ROW_START() not called */
      writer->write_status = WRITE_ERNOTSTARTED;
      return WRITE_ERNOTSTARTED;
    }

    const char DELIM = writer->__delim;
    const char QUOTE_CHAR = writer->__quote_char;
    const char ESCAPE_CHAR = writer->__escape_char;

    int inside_quotes = 0;

    size_t string_len = strlen(string);

    char ch;
    for (size_t i = 0; i < string_len; i++)
    {
      ch = string[i];
      if (ch == DELIM || ch == QUOTE_CHAR || ch == CCSV_CR || ch == CCSV_LF)
      {
        inside_quotes = 1;
        break;
      }
    }

    if (inside_quotes)
    {
      fputc(QUOTE_CHAR, fp);
      for (size_t i = 0; i < string_len; i++)
      {
        ch = string[i];
        /* Escape the quote character */
        if (ch == QUOTE_CHAR)
          fputc(ESCAPE_CHAR, fp);

        fputc(ch, fp);
      }
      fputc(QUOTE_CHAR, fp);
    }
    else
      fputs(string, fp);

    writer->write_status = WRITE_SUCCESS;
    return WRITE_SUCCESS;
  }

#ifdef __cplusplus
}
#endif