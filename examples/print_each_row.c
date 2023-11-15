#include <stdio.h>
#include <stdlib.h>

#include "../include/ccsv.h"

int main(void)
{
    FILE *fp = fopen("../../ign.csv", "r"); // Specify the path to your file

    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    // Reader object
    ccsv_reader *reader = ccsv_init_reader(NULL); // NULL for default options
    /*
        Default options:
        delim = ','
        quote_char = '"'
        escape_char = "\"\""
        comment_char = '#'

        ignore_empty_line = 1
        ignore_comment = 1
        ignore_escape_sequence = 1
    */

    CSVRow *row;

    // Read each row and print each field
    while ((row = read_row(fp, reader)) != NULL)
    {
        int row_len = row->fields_count; // Get number of fields in the row
        for (int i = 0; i < row_len; i++)
        {
            printf("%d.Field: %s\n", i + 1, row->fields[i]); // Print each field
        }
        printf("\n");
        free_row(row); // Free the memory allocated to the row
    }
    printf("\n\nRows read: %d\n", reader->rows_read); // Print number of rows read

    free(reader); // Free the memory allocated to the reader
    fclose(fp);

    return 0;
}