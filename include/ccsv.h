// File: ccsv.h

// Created: 2023 by Ayush Tripathy
// github.com/Ayush-Tripathy

/*
    This file has functions to handle parsing of csv files.
    The parser can be initialized with the function ccsv_init_parser.
    The parser can be configured with the following parameters:
        - delimiter character
        - quote character
        - escape character
        - comment character
        - ignore empty lines
        - ignore comments
        - ignore escape sequences
    The parser can be configured to ignore empty lines, comments, and escape sequences.

    For more information on how to use the parser, see the README.md file.

    Functions available:
        - ccsv_init_reader
        - read_row
        - free_row
        - get_row_string (internal use)
        - get_field (internal use)
        - is_escaped (internal use)

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

    typedef struct ccsv_reader_options
    {
        char delim;
        char quote_char;
        char *escape_char;
        char comment_char;
        char ignore_empty_line;
        char ignore_comment;
        char ignore_escape_sequence;
    } ccsv_reader_options;

    typedef struct ccsv_reader
    {
        char __delim;
        char __quote_char;
        char *__escape_char;
        char __comment_char;
        char __ignore_empty_line; // Currently not working properly
        char __ignore_comment;
        char __ignore_escape_sequence;
        int rows_read;
        int __inside_quotes;
        int __inside_comment;

    } ccsv_reader;

    typedef struct CSVRow
    {
        char *__row_string;
        char *__row_string_start;
        char *original_row_string;
        char **fields;
        int fields_count;
    } CSVRow;

    // Public functions ------------------------------------------------------------------------

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

    // Private functions -----------------------------------------------------------------------

    /*
        This function reads a row in form of string from the file pointer, and returns a pointer
        to that row string.

        params:
            fp: file pointer
            parser: pointer to the parser
    */
    char *_get_row_string(FILE *fp, ccsv_reader *parser);

    /*
        This function reads a field in form of string from the row string, and returns a pointer
        to that field string.

        params:
            row_string: pointer to the row string
            parser: pointer to the parser
    */
    char *_get_field(char **row_string, ccsv_reader *parser);

    /*
        This function checks if the given string is escaped or not.

        params:
            str: pointer to the string
            escape_char: pointer to the escape sequence
    */
    int _is_escaped(const char *str, const char *escape_seq);

#ifdef __cplusplus
}
#endif