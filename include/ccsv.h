// File: ccsv.h

// Created: 2023 by Ayush Tripathy
// github.com/Ayush-Tripathy

/*
 *   CCSV - A CSV parser and writer library for C.
 *   Version: 0.1
 *
 *   For full documentation, see the README.md file.
 */

#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Buffer sizes
#define MAX_BUFFER_SIZE 2048
#define MAX_FIELD_SIZE 256

// Default values
#define CCSV_DELIMITER 0x2c
#define CCSV_QUOTE_CHAR 0x22
#define CCSV_CR 0x0d
#define CCSV_LF 0x0a
#define CCSV_SPACE 0x20
#define CCSV_TAB 0x09
#define CCSV_COMMENT_CHAR 0x23
#define CCSV_NULL_CHAR 0x00

#define DEFAULT_DELIMITER CCSV_DELIMITER
#define DEFAULT_QUOTE_CHAR CCSV_QUOTE_CHAR
#define DEFAULT_ESCAPE_SEQUENCE "\"\""
#define DEFAULT_COMMENT_CHAR CCSV_COMMENT_CHAR

#define TOTAL_ERROR_MESSAGES 7

// Return codes
#define CCSV_SUCCESS 0
#define CCSV_ERROR -1
#define CCSV_ERNOMEM -2
#define CCSV_ERINVALID -3
#define CCSV_ERNULLFP -6     /* File pointer is NULL */
#define CCSV_ERMODE -7       /* Invalid mode */
#define CCSV_EROPEN -8       /* Error opening file */
#define CCSV_ERINVOBJTYPE -9 /* Invalid object type */
#define CCSV_ERNULLROW -10   /* Row is NULL */

#define WRITE_SUCCESS CCSV_SUCCESS
#define WRITE_STARTED 1
#define WRITE_ENDED 2
#define WRITE_ERNOTSTARTED -4 /* Writer not started */
#define WRITE_ERNOMEM CCSV_ERNOMEM
#define WRITE_ERINVALID CCSV_ERINVALID
#define WRITE_ERALWRITING -5 /* Already writing field */

// Object types
#define CCSV_READER 21
#define CCSV_WRITER 22

#define ARRAY_LEN(array) sizeof(array) / sizeof(array[0])

// Writer Macros
/* Start new row */
#define CCSV_WRITE_ROW_START(fp, writer) _write_row_start(fp, writer)

/* Write field */
#define CCSV_WRITE_FIELD(fp, writer, string)    \
    if (writer->__state == WRITER_ROW_START)    \
    {                                           \
        writer->__state = WRITER_WRITING_FIELD; \
    }                                           \
    else                                        \
    {                                           \
        fputc(writer->__delim, fp);             \
    }                                           \
    _write_field(fp, writer, string);

/* End row, with an additional field */
#define CCSV_WRITE_ROW_END(fp, writer, last_field) \
    if (last_field)                                \
    {                                              \
        _write_field(fp, writer, last_field);      \
    }                                              \
    _write_row_end(fp, writer);

    typedef enum State
    {
        FIELD_START,
        FIELD_NOT_STARTED,
        FIELD_END,
        FIELD_STARTED,
        INSIDE_QUOTED_FIELD,
        MAY_BE_ESCAPED
    } State;

    typedef enum WriterState
    {
        WRITER_NOT_STARTED,
        WRITER_ROW_START,
        WRITER_WRITING_FIELD,
        WRITER_ROW_END
    } WriterState;

    typedef struct ccsv_reader_options
    {
        char delim;
        char quote_char;
        char comment_char;
        int skip_initial_space;
        int skip_empty_lines;
        int skip_comments;
    } ccsv_reader_options;

    typedef struct ccsv_reader
    {
        int rows_read;
        char __delim;
        char __quote_char;
        char __comment_char;
        int __skip_initial_space;
        int __skip_empty_lines;
        int __skip_comments;
        char *__buffer;
        size_t __buffer_pos;
        short __awaiting_termination;
        size_t __last_bytes_read;
        size_t __buffer_size;
        size_t __largest_buffer_size;
        size_t __last_row_pos;
        FILE *__fp;
        short status;
        short object_type;
    } ccsv_reader;

    typedef struct ccsv_row
    {
        char **fields;
        int fields_count;
    } ccsv_row;

    typedef struct ccsv_writer_options
    {
        char delim;
        char quote_char;
    } ccsv_writer_options;

    typedef struct ccsv_writer
    {
        char __delim;
        char __quote_char;
        WriterState __state;
        FILE *__fp;
        short write_status;
        short object_type;
    } ccsv_writer;

    // Public functions ------------------------------------------------------------------------

    /* -------- General -------- */

    /*
     *  This function opens a file and attaches it with specified object.
     *
     *  returns:
     *      void*: pointer to the object
     */
    void *ccsv_open(const char *filename, short object_type, const char *mode, void *options, short *status);

    /*
     *  This function closes the ccsv object (reader or writer).
     *
     *  params:
     *      obj: pointer to the object
     */
    void ccsv_close(void *obj);

    /*
     *   This function returns the status message for the given status code.
     *
     *  params:
     *      status: status code
     *
     *  returns:
     *      char*: pointer to the status message
     */
    const char *ccsv_get_status_message(short status);

    /*
     *    This function returns if error occurred in the object.
     *
     *   params:
     *        obj: pointer to the object
     *
     *   returns:
     *       int: 1, if error occurred
     *       int: 0, if no error occurred
     */
    int ccsv_is_error(void *obj, short *status);

    /* -------- Reader -------- */

    /*
     *  This function initializes the parser with the given parameters, and
     *  returns a pointer to the parser.
     *
     * params:
     *   options: pointer to the reader options struct
     *
     * returns:
     *    ccsv_reader*: pointer to the reader
     */
    ccsv_reader *ccsv_init_reader(ccsv_reader_options *options, short *status);

    /*
     * This function reads a row from the file pointer, and returns a pointer
     *   to CSVRow struct.
     *
     * params:
     *    fp: file pointer
     *    parser: pointer to the parser
     *
     * returns:
     *     CSVRow*: pointer to the CSVRow struct
     */
    ccsv_row *read_row(FILE *fp, ccsv_reader *parser);

    /*
     * This function reads a row from reader, and returns a pointer
     *   to CSVRow struct.
     *
     * params:
     *    reader: pointer to the reader
     *
     * returns:
     *     CSVRow*: pointer to the CSVRow struct
     */
    ccsv_row *ccsv_next(ccsv_reader *reader);

    /*
     *  This function frees the memory allocated to the CSVRow struct.
     *
     * params:
     *    row: pointer to the CSVRow struct
     */
    void free_row(ccsv_row *row);

    /* -------- Writer -------- */

    /*
     * This function initializes the writer with the given parameters, and
     *  returns a pointer to the writer.
     *
     * params:
     *    options: pointer to the writer options struct
     *
     * returns:
     *     ccsv_writer*: pointer to the writer
     *
     */
    ccsv_writer *ccsv_init_writer(ccsv_writer_options *options, short *status);

    /*
     *  This function writes a row (from CSVRow struct) to the file pointer.
     *
     * params:
     *   fp: file pointer
     *   writer: pointer to the writer
     *   row: CSVRow struct
     */
    int write_row(FILE *fp, ccsv_writer *writer, ccsv_row row);

    /*
     *This function writes a row (from string) to the file pointer.
     *
     * params:
     *    fp: file pointer
     *    writer: pointer to the writer
     *    row_string: pointer to the row string
     *
     * returns:
     *    int: 0, if successful
     *    CSV_ERNOMEM, if memory allocation failed
     */
    int write_row_from_array(FILE *fp, ccsv_writer *writer, char **fields, int row_len);

    // Private functions -----------------------------------------------------------------------

    /*
     * This function returns the object type of the object.
     *
     *  params:
     *     obj: pointer to the object
     *
     *  returns:
     *     int: object type
     */
    int _get_object_type(void *obj);

    /*
     * This function reads a row from the file pointer, and returns a pointer to CSVRow struct.
     *
     * params:
     *    fp: file pointer
     *    reader: pointer to the reader
     *
     * returns:
     *     CSVRow*: pointer to the CSVRow struct
     */
    ccsv_row *_read_row(FILE *fp, ccsv_reader *reader);

    /*
     * This function reads a row from the file pointer, and returns a pointer to CSVRow struct.
     *
     * params:
     *    fp: file pointer
     *    reader: pointer to the reader
     *
     * returns:
     *     CSVRow*: pointer to the CSVRow struct
     */
    ccsv_row *_next(FILE *fp, ccsv_reader *reader);

    /*
     * This functions checks if the reader buffer is empty.
     */
    int _is_buffer_empty(ccsv_reader *reader);

    /*
     *This function frees multiple pointers.
     *
     * params:
     *      num: number of pointers to free
     *      ...: pointers to free
     *
     */
    void _free_multiple(int num, ...);

    /*
     * This function writes a field to the file pointer.
     *
     * params:
     *   fp: file pointer
     *   writer: pointer to the writer
     *   string: pointer to the string
     *   string_len: length of the string
     *   string_pos: pointer to the position of the string
     *
     * returns:
     *    size_t: number of characters written
     */
    int _write_field(FILE *fp, ccsv_writer *writer, const char *string);

    /*
     *This function writes a row start to the file pointer.
     *
     * params:
     *    fp: file pointer
     *    writer: pointer to the writer
     *
     * returns:
     *   int: WRITE_STARTED, if successful
     *   int: WRITE_ERALWRITING, if already writing field
     */
    int _write_row_start(FILE *fp, ccsv_writer *writer);

    /*
     * This function writes a row end to the file pointer.
     *
     * params:
     *     fp: file pointer
     *     writer: pointer to the writer
     *
     * returns:
     *    int: WRITE_ENDED, if successful
     *    int: WRITE_ERNOTSTARTED, if writer not started
     */
    int _write_row_end(FILE *fp, ccsv_writer *writer);

#ifdef __cplusplus
}
#endif