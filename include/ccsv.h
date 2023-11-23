// File: ccsv.h

// Created: 2023 by Ayush Tripathy
// github.com/Ayush-Tripathy

/*
    This file has functions to handle parsing of csv files.
    The parser can be initialized with the function ccsv_init_parser.
    The parser can be configured with the following parameters:
        - delimiter character
        - quote character
        - skip initial space
    The parser can be configured to ignore empty lines, comments, and escape sequences.

    For more information on how to use the parser, see the README.md file.

    Functions available:
        - ccsv_init_reader
        - read_row
        - free_row

    Structs available:
        - ccsv_reader_options
        - ccsv_reader
        - CSVRow

    Macros available:
        - MAX_BUFFER_SIZE
        - MAX_FIELD_SIZE
        - DEFAULT_DELIMITER
        - DEFAULT_QUOTE_CHAR
        - DEFAULT_ESCAPE_SEQUENCE
        - DEFAULT_COMMENT_CHAR
        - CSV_SUCCESS
        - CSV_ERROR
        - CSV_NOMEM
        - CSV_INVALID
        - WRITE_SUCCESS
        - WRITE_ERNOTSTARTED
        - WRITE_ERNOMEM
        - WRITE_ERINVALID
        - WRITE_ERALWRITING

    For full documentation, see the README.md file.
*/

#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

// Buffer sizes
#define MAX_BUFFER_SIZE 2048
#define MAX_FIELD_SIZE 512

// Default values
#define CSV_DELIMITER 0x2c
#define CSV_QUOTE_CHAR 0x22
#define CSV_CR 0x0d
#define CSV_LF 0x0a
#define CSV_SPACE 0x20
#define CSV_TAB 0x09
#define CSV_COMMENT_CHAR 0x23
#define CSV_NULL_CHAR 0x00

#define DEFAULT_DELIMITER CSV_DELIMITER
#define DEFAULT_QUOTE_CHAR CSV_QUOTE_CHAR
#define DEFAULT_ESCAPE_SEQUENCE "\"\""
#define DEFAULT_COMMENT_CHAR CSV_COMMENT_CHAR

// Return codes
#define CSV_SUCCESS 0
#define CSV_ERROR -1
#define CSV_ERNOMEM -2
#define CSV_ERINVALID -3

#define WRITE_SUCCESS CSV_SUCCESS
#define WRITE_STARTED 1
#define WRITE_ENDED 2
#define WRITE_ERNOTSTARTED -4 /* Writer not started */
#define WRITE_ERNOMEM -5
#define WRITE_ERINVALID CSV_ERINVALID
#define WRITE_ERALWRITING -7 /* Already writing field */

// Writer Macros
/* Start new row */
#define CSV_WRITE_ROW_START(fp, writer) _write_row_start(fp, writer)

/* Write field */
#define CSV_WRITE_FIELD(fp, writer, string) \
    _write_field(fp, writer, string);       \
    fputc(writer->__delim, fp);

/* End row, with an additional field */
#define CSV_WRITE_ROW_END(fp, writer, last_field) \
    if (last_field)                               \
    {                                             \
        _write_field(fp, writer, last_field);     \
    }                                             \
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
    } ccsv_reader;

    typedef struct CSVRow
    {
        char **fields;
        int fields_count;
    } CSVRow;

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
        short write_status;
    } ccsv_writer;

    // Public functions ------------------------------------------------------------------------

    /* -------- Reader -------- */

    /*
        This function initializes the parser with the given parameters, and
        returns a pointer to the parser.

        params:
            options: pointer to the reader options struct

        returns:
            ccsv_reader*: pointer to the reader
    */
    ccsv_reader *ccsv_init_reader(ccsv_reader_options *options);

    /*
        This function reads a row from the file pointer, and returns a pointer
        to CSVRow struct.

        params:
            fp: file pointer
            parser: pointer to the parser

        returns:
            CSVRow*: pointer to the CSVRow struct
    */
    CSVRow *read_row(FILE *fp, ccsv_reader *parser);

    /*
        This function frees the memory allocated to the CSVRow struct.

        params:
            row: pointer to the CSVRow struct
    */
    void free_row(CSVRow *row);

    /* -------- Writer -------- */

    /*
        This function initializes the writer with the given parameters, and
        returns a pointer to the writer.

        params:
            options: pointer to the writer options struct

        returns:
            ccsv_writer*: pointer to the writer

    */
    ccsv_writer *ccsv_init_writer(ccsv_writer_options *options);

    /*
        This function writes a row (from CSVRow struct) to the file pointer.

        params:
            fp: file pointer
            writer: pointer to the writer
            row: CSVRow struct
    */
    int write_row(FILE *fp, ccsv_writer *writer, CSVRow row);

    /*
        This function writes a row (from string) to the file pointer.

        params:
            fp: file pointer
            writer: pointer to the writer
            row_string: pointer to the row string

        returns:
            int: 0, if successful
                CSV_ERNOMEM, if memory allocation failed
    */
    int write_row_from_array(FILE *fp, ccsv_writer *writer, char **fields, int row_len);

    // Private functions -----------------------------------------------------------------------

    /*
        This function frees multiple pointers.

        params:
            num: number of pointers to free
            ...: pointers to free

    */
    void _free_multiple(int num, ...);

    /*
        This function writes a field to the file pointer.

        params:
            fp: file pointer
            writer: pointer to the writer
            string: pointer to the string
            string_len: length of the string
            string_pos: pointer to the position of the string

        returns:
            size_t: number of characters written
    */
    int _write_field(FILE *fp, ccsv_writer *writer, const char *string);

    /*
        This function writes a row start to the file pointer.

        params:
            fp: file pointer
            writer: pointer to the writer

        returns:
            int: WRITE_STARTED, if successful
            int: WRITE_ERALWRITING, if already writing field
    */
    int _write_row_start(FILE *fp, ccsv_writer *writer);

    /*
        This function writes a row end to the file pointer.

        params:
            fp: file pointer
            writer: pointer to the writer

        returns:
            int: WRITE_ENDED, if successful
            int: WRITE_ERNOTSTARTED, if writer not started
    */
    int _write_row_end(FILE *fp, ccsv_writer *writer);

#ifdef __cplusplus
}
#endif