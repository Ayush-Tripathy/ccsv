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
    char delim, quote_char;
    int skip_initial_space;
    if (options == NULL)
    {
        delim = DEFAULT_DELIMITER;
        quote_char = DEFAULT_QUOTE_CHAR;
        skip_initial_space = 0;
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

        if (options->skip_intial_space == '\0')
        {
            skip_initial_space = 0;
        }
        else
        {
            skip_initial_space = options->skip_intial_space;
        }
    }

    // Parser
    ccsv_reader *parser = (ccsv_reader *)malloc(sizeof(ccsv_reader));
    parser->__delim = delim;
    parser->__quote_char = quote_char;
    parser->__skip_intial_space = skip_initial_space;

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

CSVRow *read_row(FILE *fp, ccsv_reader *reader)
{
    CSVRow *row = (CSVRow *)malloc(sizeof(CSVRow));
    if (row == NULL)
    {
        printf("Row is null\n");
        exit(1);
    }

    const char DELIM = reader->__delim;
    const char QUOTE_CHAR = reader->__quote_char;
    const int SKIP_INITIAL_SPACE = reader->__skip_intial_space;

    State state = FIELD_START;

    char *row_string = (char *)malloc(MAX_BUFFER_SIZE + 1);
    if (row_string == NULL)
    {
        printf("Row string is null\n");
        exit(1);
    }
    size_t row_string_size = MAX_BUFFER_SIZE;
    size_t row_pos = 0;

    char **fields = (char **)malloc(sizeof(char *));
    size_t fields_count = 0;

    char *field = (char *)malloc(MAX_FIELD_SIZE + 1);
    if (field == NULL)
    {
        printf("2.Field is null\n");
        exit(1);
    }
    size_t field_size = MAX_FIELD_SIZE;
    size_t field_pos = 0;

    int inside_quotes = 0;

readfile:
    if (fgets(row_string, MAX_BUFFER_SIZE, fp) == NULL)
    {
        free(row_string);
        free(field);
        free(fields);
        free(row);
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
                state = INSIDE_QUOTED_FIELD;
            }
            else if (SKIP_INITIAL_SPACE && c == ' ')
            {
                state = FIELD_NOT_STARTED;
            }
            else if (c == DELIM)
            {
                row_pos--;
                state = FIELD_END;
            }
            else if (c == '\r' || c == '\n')
            {
                state = FIELD_END;
                row_pos--;
            }
            else
            {
                state = FIELD_STARTED;
                field[field_pos++] = c;
            }
            row_pos++;
            break;

        case INSIDE_QUOTED_FIELD:
            inside_quotes = 1;
            if (c == QUOTE_CHAR)
            {
                state = MAY_BE_ESCAPED;
            }
            else
            {
                state = FIELD_STARTED;
                field[field_pos++] = c;
            }
            row_pos++;
            break;

        case FIELD_NOT_STARTED:
            if (c == QUOTE_CHAR)
            {
                state = INSIDE_QUOTED_FIELD;
            }
            else if (c == DELIM)
            {
                row_pos--;
                state = FIELD_END;
            }
            else if (c == ' ')
            {
                state = FIELD_NOT_STARTED;
            }
            else
            {
                state = FIELD_STARTED;
                field[field_pos++] = c;
            }
            row_pos++;
            break;

        case FIELD_STARTED:
            if (field_pos > field_size - 1)
            {
                field_size += MAX_FIELD_SIZE;
                field = (char *)realloc(field, field_size + 1);
                if (field == NULL)
                {
                    printf("1.Field is null\n");
                    exit(1);
                }
            }

            if (row_pos > row_len - 1)
            {
                row_string_size += MAX_BUFFER_SIZE;
                row_string = (char *)realloc(row_string, row_string_size + 1);
                if (row_string == NULL)
                {
                    printf("Row string is null\n");
                    exit(1);
                }
            }
            if (c == QUOTE_CHAR && inside_quotes)
            {
                state = MAY_BE_ESCAPED;
            }
            else if (c == QUOTE_CHAR && !inside_quotes)
            {
                state = FIELD_STARTED;
                field[field_pos++] = c;
            }
            else if (c == DELIM && !inside_quotes)
            {
                row_pos--;
                state = FIELD_END;
            }
            else if ((c == '\n' || c == '\r') && !inside_quotes)
            {
                row_pos--;
                state = FIELD_END;
            }
            else if (c == '\0')
            {
                row_pos--;
                state = FIELD_END;
            }
            else
            {
                state = FIELD_STARTED;
                field[field_pos++] = c;
            }
            row_pos++;
            break;

        case MAY_BE_ESCAPED:
            if (c == QUOTE_CHAR)
            {
                state = FIELD_STARTED;
                field[field_pos++] = c;
            }
            else
            {
                inside_quotes = 0;
                state = FIELD_STARTED;
                row_pos--;
            }

            row_pos++;
            break;

        case FIELD_END:
            state = FIELD_START;
            field[field_pos++] = '\0';
            fields_count++;
            fields = (char **)realloc(fields, sizeof(char *) * fields_count);
            fields[fields_count - 1] = field;
            row_pos++;
            field = (char *)malloc(MAX_FIELD_SIZE + 1);
            field_size = MAX_FIELD_SIZE;
            if (field == NULL)
            {
                printf("Field is null\n");
                exit(1);
            }
            field_pos = 0;

            if (c == '\r' || c == '\n' || c == '\0')
            {
                free(field);
                goto end;
            }

            break;

        default:
            printf("Default\n");
            break;
        }

        if (row_string[row_pos] == '\0' && !inside_quotes)
        {
            // Add the last field
            fields_count++;
            fields = (char **)realloc(fields, sizeof(char *) * fields_count);
            fields[fields_count - 1] = field;
            goto end;
        }
        else if (row_pos > row_len - 1)
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
