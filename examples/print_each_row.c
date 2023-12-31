#include <stdio.h>
#include <stdlib.h>

#include "../include/ccsv.h"

int main(void)
{

    // ** There is new way to read rows in ccsv v0.5.0 **
    // ** Check examples/print_each_row_ccsv_v0.5.c **

    FILE *fp = fopen("../../ign.csv", "r"); // Specify the path to your file

    if (fp == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    // Reader object
    ccsv_reader *reader = ccsv_init_reader(NULL, NULL); // NULL for default options
    /*
        Default options:
        delim = ','
        quote_char = '"'
        skip_initial_space = 0
        skip_empty_lines = 0
        skip_comments = 0
    */

    ccsv_row *row;

    // Read each row and print each field
    while ((row = read_row(fp, reader)) != NULL)
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

    free(reader); // Free the memory allocated to the reader
    fclose(fp);

    return 0;
}