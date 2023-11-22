// File: ccsv.c

// Created: 2023 by Ayush Tripathy
// github.com/Ayush-Tripathy

/*
    This library provides functions to handle reading, writing csv files.
*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// ccsv header file
#include "ccsv.h"

/* Reader */

#define ADD_FIELD(field)                                              \
    field[field_pos++] = CSV_NULL_CHAR;                               \
    fields_count++;                                                   \
    fields = (char **)realloc(fields, sizeof(char *) * fields_count); \
    if (fields == NULL)                                               \
    {                                                                 \
        _free_multiple(3, field, row_string, row);                    \
        return (void *)CSV_ERNOMEM;                                   \
    }                                                                 \
    fields[fields_count - 1] = field;

#define GROW_FIELD_BUFFER_IF_NEEDED(field, field_size, field_pos) \
    if (field_pos > field_size - 1)                               \
    {                                                             \
        field_size += MAX_FIELD_SIZE;                             \
        field = (char *)realloc(field, field_size + 1);           \
        if (field == NULL)                                        \
        {                                                         \
            _free_multiple(3, fields, row_string, row);           \
            return (void *)CSV_ERNOMEM;                           \
        }                                                         \
    }

#define GROW_ROW_BUFFER_IF_NEEDED(row_string, row_len, row_pos)        \
    if (row_pos > row_len - 1)                                         \
    {                                                                  \
        row_string_size += MAX_BUFFER_SIZE;                            \
        row_string = (char *)realloc(row_string, row_string_size + 1); \
        if (row_string == NULL)                                        \
        {                                                              \
            _free_multiple(2, fields, row);                            \
            return (void *)CSV_ERNOMEM;                                \
        }                                                              \
    }

ccsv_reader *ccsv_init_reader(ccsv_reader_options *options)
{
    char delim, quote_char;
    int skip_initial_space, skip_empty_lines, skip_comments;
    if (options == NULL)
    {
        delim = DEFAULT_DELIMITER;
        quote_char = DEFAULT_QUOTE_CHAR;
        skip_initial_space = 0;
        skip_empty_lines = 0;
        skip_comments = 0;
    }
    else
    {
        // It is not mandatory to pass all options to options struct
        // So check if the option is passed or not, if not then use the default value
        if (options->delim == CSV_NULL_CHAR)
            delim = DEFAULT_DELIMITER;

        else
            delim = options->delim;

        if (options->quote_char == CSV_NULL_CHAR)
            quote_char = DEFAULT_QUOTE_CHAR;

        else
            quote_char = options->quote_char;

        if (options->skip_initial_space == CSV_NULL_CHAR)
            skip_initial_space = 0;

        else
            skip_initial_space = options->skip_initial_space;

        if (options->skip_empty_lines == CSV_NULL_CHAR)
            skip_empty_lines = 0;

        else
            skip_empty_lines = options->skip_empty_lines;

        if (options->skip_comments == CSV_NULL_CHAR)
            skip_comments = 0;

        else
            skip_comments = options->skip_comments;
    }

    // Parser
    ccsv_reader *parser = (ccsv_reader *)malloc(sizeof(ccsv_reader));
    if (parser == NULL)
    {
        return (void *)CSV_ERNOMEM;
    }
    parser->__delim = delim;
    parser->__quote_char = quote_char;
    parser->__skip_initial_space = skip_initial_space;
    parser->__skip_empty_lines = skip_empty_lines;
    parser->__skip_comments = skip_comments;

    parser->rows_read = 0;

    return parser;
}

void free_row(CSVRow *row)
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

CSVRow *read_row(FILE *fp, ccsv_reader *reader)
{
    CSVRow *row = (CSVRow *)malloc(sizeof(CSVRow));
    if (row == NULL)
    {
        return (void *)CSV_ERNOMEM;
    }

    const char DELIM = reader->__delim;
    const char QUOTE_CHAR = reader->__quote_char;
    const int SKIP_INITIAL_SPACE = reader->__skip_initial_space;
    const int SKIP_EMPTY_LINES = reader->__skip_empty_lines;
    const int SKIP_COMMENTS = reader->__skip_comments;

    State state = FIELD_START;

    char *row_string = (char *)malloc(MAX_BUFFER_SIZE + 1);
    if (row_string == NULL)
    {
        _free_multiple(1, row);
        return (void *)CSV_ERNOMEM;
    }
    size_t row_string_size = MAX_BUFFER_SIZE;
    size_t row_pos = 0;

    char **fields = (char **)malloc(sizeof(char *));
    if (fields == NULL)
    {
        _free_multiple(2, row_string, row);
        return (void *)CSV_ERNOMEM;
    }
    size_t fields_count = 0;

    char *field = (char *)malloc(MAX_FIELD_SIZE + 1);
    if (field == NULL)
    {
        _free_multiple(3, row_string, fields, row);
        return (void *)CSV_ERNOMEM;
    }
    size_t field_size = MAX_FIELD_SIZE;
    size_t field_pos = 0;

    int inside_quotes = 0;

readfile:
    if (fgets(row_string, MAX_BUFFER_SIZE, fp) == NULL)
    {
        /* If fields is not empty then return the row */
        if (fields_count > 0 || field_pos > 0)
        {
            /* Add the last field */

            /*
                If fields_count > 0:
                This happens when the function holding the values of last row
                but yet to return the last row

                if field_pos > 0:
                This only happens when there is a single element in the last row and also
                there is no line after the current line
                So we need to add the only field of the last row
             */
            ADD_FIELD(field);
            goto end;
        }

        _free_multiple(4, row_string, field, fields, row);
        return NULL;
    }

    size_t row_len = strlen(row_string);
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
            else if (SKIP_INITIAL_SPACE && c == CSV_SPACE)
            {
                /* Skip initial spaces */
                state = FIELD_NOT_STARTED;
            }
            else if (c == DELIM || c == CSV_CR || c == CSV_LF)
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
            else if (c == CSV_SPACE)
            {
                /*
                    Skip initial spaces, will only get to this point if
                    skip_initial_spaces = 1
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
                     ((c == CSV_LF || c == CSV_CR) && !inside_quotes) ||
                     (c == CSV_NULL_CHAR))
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
                (c == CSV_CR || c == CSV_LF || c == CSV_NULL_CHAR) &&
                !inside_quotes)
            {
                /* Do not return empty lines, parse again */
                goto readfile;
            }
            else if (SKIP_COMMENTS &&
                     fields_count == 0 &&
                     field_pos > 0 &&
                     field[0] == CSV_COMMENT_CHAR &&
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
                return (void *)CSV_ERNOMEM;
            }
            field_pos = 0;

            if (c == CSV_CR || c == CSV_LF || c == CSV_NULL_CHAR)
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
    return row;
}

/* Writer */

ccsv_writer *ccsv_init_writer(ccsv_writer_options *options)
{
    char delim, quote_char;
    if (options == NULL)
    {
        delim = DEFAULT_DELIMITER;
        quote_char = DEFAULT_QUOTE_CHAR;
    }
    else
    {
        // It is not mandatory to pass all options to options struct
        // So check if the option is passed or not, if not then use the default value
        if (options->delim == CSV_NULL_CHAR)
            delim = DEFAULT_DELIMITER;

        else
            delim = options->delim;

        if (options->quote_char == CSV_NULL_CHAR)
            quote_char = DEFAULT_QUOTE_CHAR;

        else
            quote_char = options->quote_char;
    }

    // Writer
    ccsv_writer *writer = (ccsv_writer *)malloc(sizeof(ccsv_writer));
    if (writer == NULL)
    {
        return (void *)CSV_ERNOMEM;
    }
    writer->__delim = delim;
    writer->__quote_char = quote_char;

    return writer;
}

void write_row(FILE *fp, ccsv_writer *writer, CSVRow *row)
{
    const char DELIM = writer->__delim;
    const char QUOTE_CHAR = writer->__quote_char;

    const int fields_count = row->fields_count;

    fputc(CSV_LF, fp);
    for (int i = 0; i < fields_count; i++)
    {
        char *field = row->fields[i];
        int field_len = strlen(field);

        int inside_quotes = 0;
        for (int j = 0; j < field_len; j++)
        {
            char c = field[j];
            if (c == DELIM || c == QUOTE_CHAR || c == CSV_CR || c == CSV_LF)
            {
                inside_quotes = 1;
                break;
            }
        }

        if (inside_quotes)
        {
            fputc(QUOTE_CHAR, fp);
            for (int j = 0; j < field_len; j++)
            {
                char c = field[j];
                /* Escape the quote character */
                if (c == QUOTE_CHAR)
                {
                    fputc(QUOTE_CHAR, fp);
                }
                fputc(c, fp);
            }
            fputc(QUOTE_CHAR, fp);
        }
        else
        {
            fputs(field, fp);
        }

        if (i != fields_count - 1)
        {
            fputc(DELIM, fp);
        }
    }
}

int write_row_from_string(FILE *fp, ccsv_writer *writer, char *row_string)
{
    // Example string - "hi,hello,  \"hello, world!\", bye";

    size_t string_len = strlen(row_string);
    size_t string_pos = 0, write_count;

    while ((write_count = _write_field(fp, writer, row_string, string_len, &string_pos)) > 0)
    {
        if (string_len == 0)
            break;
    }

    if ((long long)write_count == CSV_ERNOMEM)
        return CSV_ERNOMEM;

    /* CRLF - line terminator */
    fputc(CSV_CR, fp);
    fputc(CSV_LF, fp);

    return 0;
}

size_t _write_field(FILE *fp, ccsv_writer *writer, char *string, size_t string_len, size_t *string_pos)
{
    const char DELIM = writer->__delim;
    const char QUOTE_CHAR = writer->__quote_char;

    int inside_quotes = 0;
    size_t write_count = 0;

    char ch;
    size_t current_pos = *string_pos;

    while (1)
    {
        if (current_pos > string_len - 1)
            break;

        ch = string[current_pos++];

        if (ch == CSV_NULL_CHAR)
            break;
        else if ((ch == CSV_CR || ch == CSV_LF) && !inside_quotes)
            break;
        else if (ch == QUOTE_CHAR)
            inside_quotes = !inside_quotes;
        else if (ch == DELIM && !inside_quotes)
            break;
    }

    write_count = current_pos - (*string_pos);

    char *string_to_write = (char *)malloc(write_count + 1);
    if (string_to_write == NULL)
        return CSV_ERNOMEM; /* Memory allocation error */

    memcpy(string_to_write, string + (*string_pos), write_count);
    string_to_write[write_count] = CSV_NULL_CHAR;

    *string_pos = current_pos;

    fputs(string_to_write, fp);
    free(string_to_write);
    return write_count;
}