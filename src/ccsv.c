// File: ccsv.c

// Created: 2023 by Ayush Tripathy
// github.com/Ayush-Tripathy

/*
    This library provides functions to handle reading, writing csv files.
*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// #define _POSIX1_SOURCE 2
// #include <unistd.h>

// int ftruncate(int fildes, off_t length);

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

#define RETURN_IF_WRITE_ERROR(writer, desired_status) \
    if (writer->write_status != desired_status)       \
        return writer->write_status;

ccsv_reader *ccsv_init_reader(ccsv_reader_options *options)
{
    char delim, quote_char, comment_char;
    int skip_initial_space, skip_empty_lines, skip_comments;
    if (options == NULL)
    {
        delim = DEFAULT_DELIMITER;
        quote_char = DEFAULT_QUOTE_CHAR;
        comment_char = DEFAULT_COMMENT_CHAR;
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

        if (options->comment_char == CSV_NULL_CHAR)
            comment_char = DEFAULT_COMMENT_CHAR;

        else
            comment_char = options->comment_char;

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
    parser->__comment_char = comment_char;
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
    const char COMMENT_CHAR = reader->__comment_char;
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
    WriterState state = WRITER_NOT_STARTED;
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
    writer->__state = state;

    return writer;
}

int write_row(FILE *fp, ccsv_writer *writer, CSVRow row)
{
    const int fields_count = row.fields_count;
    char **fields = row.fields;
    return (write_row_from_array(fp, writer, fields, fields_count));
}

int write_row_from_array(FILE *fp, ccsv_writer *writer, char **fields, int row_len)
{
    CSV_WRITE_ROW_START(fp, writer);
    RETURN_IF_WRITE_ERROR(writer, WRITE_STARTED);

    int i;
    for (i = 0; i < row_len - 1; i++)
    {
        const char *field = fields[i];
        CSV_WRITE_FIELD(fp, writer, field);
        RETURN_IF_WRITE_ERROR(writer, WRITE_SUCCESS);
    }
    CSV_WRITE_ROW_END(fp, writer, fields[i]); /* End with writing last field */
    RETURN_IF_WRITE_ERROR(writer, WRITE_ENDED);

    writer->write_status = WRITE_SUCCESS;
    return WRITE_SUCCESS;
}

int _write_row_start(FILE *fp, ccsv_writer *writer)
{
    switch (writer->__state)
    {
    case WRITER_NOT_STARTED:
        writer->__state = WRITER_ROW_START; /* Start writing row */

        /* Move the file pointer to the end to get the file size */
        fseek(fp, 0, SEEK_END);
        long file_size = ftell(fp);

        /* Move the pointer to the penultimate position */
        fseek(fp, -1, SEEK_END);

        char last_char = fgetc(fp);

        if (last_char != CSV_LF && last_char != CSV_CR && file_size > 0)
        {
            fputc(CSV_CR, fp);
            fputc(CSV_LF, fp);
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

        // TODO: Handle removal of last character in append mode

        // long currentPos = ftell(fp);

        // // If the file is not empty
        // if (currentPos > 0)
        // {
        //     fseek(fp, -1, SEEK_END);
        //     ftruncate(fileno(fp), currentPos - 1); /* Truncate the file at this position */
        // }
        // fseek(fp, -1, SEEK_CUR);
        fputc(CSV_CR, fp);
        fputc(CSV_LF, fp);
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

    int inside_quotes = 0;

    size_t string_len = strlen(string);

    char ch;
    for (size_t i = 0; i < string_len; i++)
    {
        ch = string[i];
        if (ch == DELIM || ch == QUOTE_CHAR || ch == CSV_CR || ch == CSV_LF)
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
            {
                fputc(QUOTE_CHAR, fp);
            }
            fputc(ch, fp);
        }
        fputc(QUOTE_CHAR, fp);
    }
    else
    {
        fputs(string, fp);
    }

    writer->write_status = WRITE_SUCCESS;
    return WRITE_SUCCESS;
}