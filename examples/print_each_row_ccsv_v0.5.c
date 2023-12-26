#include <stdio.h>
#include <stdlib.h>

#include "../include/ccsv.h"

int main(void)
{
    ccsv_reader_options options = {
        .delim = ',',
        .quote_char = '"',
        .skip_comments = 1,
        .skip_initial_space = 0,
        .skip_empty_lines = 0,
    };
    // Reader object
    ccsv_reader *reader = ccsv_open("../../comments.csv", CCSV_READER, "r", &options, NULL); // NULL for default options
    if (reader == NULL)
    {
        fprintf(stderr, "Error initializing CSV reader\n");
        return 1;
    }

    ccsv_row *row;

    // Read each row and print each field
    while ((row = ccsv_next(reader)) != NULL)
    {
        int row_len = row->fields_count; // Get number of fields in the row
        for (int i = 0; i < row_len; i++)
        {
            printf("%d.Field: %s\n", i + 1, row->fields[i]); // Print each field
        }
        printf("\n");
        ccsv_free_row(row); // Free the memory allocated to the row
    }
    printf("\n\nRows read: %d\n", reader->rows_read); // Print number of rows read

    ccsv_close(reader); // Close the reader

    return 0;
}