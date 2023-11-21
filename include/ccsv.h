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

    For full documentation, see the README.md file.
*/

#pragma once

#include <stdio.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_BUFFER_SIZE 2048
#define MAX_FIELD_SIZE 512

#define DEFAULT_DELIMITER ','
#define DEFAULT_QUOTE_CHAR '"'
#define DEFAULT_ESCAPE_SEQUENCE "\"\""
#define DEFAULT_COMMENT_CHAR '#'

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
        int skip_intial_space;
    } ccsv_reader_options;

    typedef struct ccsv_reader
    {
        int rows_read;
        char __delim;
        char __quote_char;
        int __skip_intial_space;
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
    */
    ccsv_reader *ccsv_init_reader(ccsv_reader_options *options);

    /*
        This function reads a row from the file pointer, and returns a pointer
        to CSVRow struct.

        params:
            fp: file pointer
            parser: pointer to the parser
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
    */
    void write_row_from_string(FILE *fp, ccsv_writer *writer, char *row_string);

    // Private functions -----------------------------------------------------------------------

    /* -------- Writer -------- */

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