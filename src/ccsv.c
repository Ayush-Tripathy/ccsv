// File: ccsv.c

// Created: 2023 by Ayush Tripathy
// github.com/Ayush-Tripathy

/*
    This file contains the implementation of the functions declared in ccsv.h
*/

#include <stdlib.h>
#include <string.h>

// ccsv header file
#include "ccsv.h"

/* Reader */

ccsv_reader *ccsv_init_reader(ccsv_reader_options *options)
{
    char delim, quote_char, *escape_char, comment_char;
    int ignore_empty_line, ignore_comment, ignore_escape_sequence;
    if (options == NULL)
    {
        delim = DEFAULT_DELIMITER;
        quote_char = DEFAULT_QUOTE_CHAR;
#ifdef __cplusplus
        escape_char = (char *)DEFAULT_ESCAPE_SEQUENCE;
#else
        escape_char = DEFAULT_ESCAPE_SEQUENCE;
#endif
        comment_char = DEFAULT_COMMENT_CHAR;

        ignore_empty_line = 1;
        ignore_comment = 1;
        ignore_escape_sequence = 1;
    }
    else
    {
        // It is not mandatory to pass all options to options struct
        // So check if the option is passed or not, if not then use the default value
        if (options->delim == '\0')
            delim = DEFAULT_DELIMITER;

        else
            delim = options->delim;

        if (options->quote_char == '\0')
            quote_char = DEFAULT_QUOTE_CHAR;

        else
            quote_char = options->quote_char;

        if (options->escape_char == NULL)
        {
#ifdef __cplusplus
            escape_char = (char *)DEFAULT_ESCAPE_SEQUENCE;
#else
            escape_char = DEFAULT_ESCAPE_SEQUENCE;
#endif
        }
        else
            escape_char = options->escape_char;

        if (options->comment_char == '\0')
            comment_char = DEFAULT_COMMENT_CHAR;

        else
            comment_char = options->comment_char;

        if (options->ignore_empty_line == '\0')
            ignore_empty_line = 1;

        else
            ignore_empty_line = options->ignore_empty_line;

        if (options->ignore_comment == '\0')
            ignore_comment = 1;

        else
            ignore_comment = options->ignore_comment;

        if (options->ignore_escape_sequence == '\0')
            ignore_escape_sequence = 1;

        else
            ignore_escape_sequence = options->ignore_escape_sequence;
    }

    // Parser
    ccsv_reader *parser = (ccsv_reader *)malloc(sizeof(ccsv_reader));
    parser->__delim = delim;
    parser->__quote_char = quote_char;
    parser->__escape_char = escape_char;
    parser->__comment_char = comment_char;

    parser->__ignore_empty_line = ignore_empty_line;
    parser->__ignore_comment = ignore_comment;
    parser->__ignore_escape_sequence = ignore_escape_sequence;

    parser->__inside_quotes = 0;

    parser->rows_read = 0;

    return parser;
}

CSVRow *read_row(FILE *fp, ccsv_reader *parser)
{
    CSVRow *row = (CSVRow *)malloc(sizeof(CSVRow));
    row->__row_string = _get_row_string(fp, parser);
    if (row->__row_string == NULL)
    {
        free(row);
        return NULL;
    }

    char *row_string = row->__row_string;
    size_t row_len = strlen(row_string);

    row->__row_string_start = row_string;
    row->original_row_string = (char *)malloc(row_len + 1);
    memcpy(row->original_row_string, row->__row_string, row_len + 1);
    row->fields_count = 0;
    row->fields = (char **)malloc(sizeof(char *));

    char *field;
    int *fields_count = &(row->fields_count);

    while ((field = _get_field(&(row_string), parser)) != NULL)
    {
        size_t field_len = strlen(field);
        (*fields_count)++;
        row->fields = (char **)realloc(row->fields, sizeof(char *) * (*fields_count));
        row->fields[*fields_count - 1] = (char *)malloc(field_len + 1);
        // strcpy(row->fields[*fields_count - 1], field);
        memcpy(row->fields[*fields_count - 1], field, field_len + 1);
        free(field);
    }

    parser->rows_read++;
    return row;
}

void free_row(CSVRow *row)
{
    free(row->__row_string_start);
    free(row->original_row_string);

    const int fields_count = row->fields_count;
    for (int i = 0; i < fields_count; i++)
    {
        free(row->fields[i]);
    }
    free(row->fields);
    free(row);
}

char *_get_row_string(FILE *fp, ccsv_reader *parser)
{
    char *buffer = (char *)malloc(MAX_BUFFER_SIZE);
    size_t size = MAX_BUFFER_SIZE;
    size_t current_pos = 0;
    size_t last_pos = 0;
    int inside_empty_line = 0;

    const char QUOTE_CHAR = parser->__quote_char;
    const char COMMENT_CHAR = parser->__comment_char;
    const int IGNORE_COMMENT = parser->__ignore_comment;
    const int IGNORE_EMPTY_LINE = parser->__ignore_empty_line;

    int inside_quotes = parser->__inside_quotes;

    while (1)
    {
        char temp[MAX_BUFFER_SIZE] = "";

        // Get some part of line from file
        if (fgets(temp, MAX_BUFFER_SIZE, fp) == NULL)
        {
            break; // Break if the end of the file is reached
        }

        size_t len = strlen(temp);

        // Ignore line if it is a comment
        if (IGNORE_COMMENT &&
            temp[0] == COMMENT_CHAR &&
            !inside_quotes)
        {
            continue;
        }

        // Ignore line if it is empty
        // Need to fix this, some cases are not handled
        if (IGNORE_EMPTY_LINE &&
            (temp[0] == '\r' && temp[1] == '\n') &&
            !inside_quotes)
        {
            // To handle the case where the fgets function reads the ending of the previous line
            temp[0] = '\0';
            inside_empty_line = 1;
            // continue;
        }
        else
        {
            inside_empty_line = 0;
        }

        // Check if the line ended in that part
        // Check if inside quote char
        while (1)
        {
            if (temp[current_pos] == QUOTE_CHAR)
            {
                inside_quotes = !inside_quotes ? 1 : 0;
            }
            current_pos++;
            if (current_pos == len)
                break;
        }

        // Store it in buffer
        memcpy(buffer + last_pos, temp, len + 1);

        if ((temp[len - 1] == '\n' || (temp[len - 2] == '\r' && temp[len - 1] == '\n')) &&
            !inside_quotes)
        {
            last_pos += current_pos - 1;
            current_pos = 0;
            break;
        }
        else
        {
            size += MAX_BUFFER_SIZE;
            buffer = (char *)realloc(buffer, size);
        }
        last_pos += current_pos;
        current_pos = 0;
    }

    parser->__inside_quotes = inside_quotes;

    // Null terminate the string
    buffer[last_pos + current_pos] = '\0';

    if (strlen(buffer) == 0 && !inside_empty_line)
    {
        free(buffer);
        return NULL;
    }
    // Check if all quotes are closed or not, if not raise error
    if (inside_quotes)
    {
        printf("CSVError: Quotes not closed\n");
        exit(1);
    }

    return buffer;
}

char *_get_field(char **row_string, ccsv_reader *parser)
{
    size_t len = strlen((*row_string));

    if (!len)
        return NULL;

    char *field = (char *)malloc(MAX_FIELD_SIZE);
    size_t current_pos = 0;
    int field_pos = 0;
    size_t size = MAX_FIELD_SIZE;

    const char QUOTE_CHAR = parser->__quote_char;
    const char DELIM = parser->__delim;
    const char *ESCAPE_CHAR = parser->__escape_char;
    const int IGNORE_ESCAPE_SEQUENCE = parser->__ignore_escape_sequence;

    int inside_quotes = parser->__inside_quotes;

    while (1)
    {
        if ((*row_string)[current_pos] == '\0')
        {
            goto update_row_string;
        }

        if ((*row_string)[current_pos] == QUOTE_CHAR)
        {
            if (IGNORE_ESCAPE_SEQUENCE &&
                _is_escaped((*row_string) + current_pos, ESCAPE_CHAR))
            {
                field[field_pos] = (*row_string)[current_pos];
                current_pos++;
            }
            else
            {
                inside_quotes = !inside_quotes ? 1 : 0;
            }
        }
        else if ((*row_string)[current_pos] == DELIM &&
                 !inside_quotes)
        {
            break;
        }
        else
        {
            if ((*row_string)[current_pos] != '\r')
            {
                field[field_pos] = (*row_string)[current_pos];
                field_pos++;
            }
        }
        current_pos++;

        if (current_pos >= size)
        {
            size += MAX_FIELD_SIZE;
            field = (char *)realloc(field, size);
        }
    }

    field[current_pos] = '\0';

update_row_string:
    if ((*row_string)[current_pos] == DELIM)
    {
        *row_string = (*row_string) + current_pos + 1;
    }
    else
    {
        *row_string = (*row_string) + current_pos;
    }

    parser->__inside_quotes = inside_quotes;

    return field;
}

int _is_escaped(char *str, const char *escape_seq)
{
    int pos = 0;
    int is_escaped = 0;

    while (escape_seq[pos] != '\0')
    {
        if (str[pos] != escape_seq[pos])
        {
            is_escaped = 0;
            return is_escaped;
        }
        else
        {
            is_escaped = 1;
        }
        pos++;
    }

    return is_escaped;
}

/* Writer */
