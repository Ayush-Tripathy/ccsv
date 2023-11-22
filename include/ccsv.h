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

    typedef enum State
    {
        FIELD_START,
        FIELD_NOT_STARTED,
        FIELD_END,
        FIELD_STARTED,
        INSIDE_QUOTED_FIELD,
        MAY_BE_ESCAPED
    } State;

    typedef struct ccsv_reader_options
    {
        char delim;
        char quote_char;
        int skip_initial_space;
        int skip_empty_lines;
        int skip_comments;
    } ccsv_reader_options;

    typedef struct ccsv_reader
    {
        int rows_read;
        char __delim;
        char __quote_char;
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
            row: pointer to the CSVRow struct
    */
    void write_row(FILE *fp, ccsv_writer *writer, CSVRow *row);

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
    int write_row_from_string(FILE *fp, ccsv_writer *writer, char *row_string);

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
    size_t _write_field(FILE *fp, ccsv_writer *writer, char *string, size_t string_len, size_t *string_pos);

#ifdef __cplusplus
}
#endif