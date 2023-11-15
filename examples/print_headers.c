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

    CSVRow *row = read_row(fp, reader);

    int row_len = row->fields_count; // Get number of fields in the row
    for (int i = 0; i < row_len; i++)
    {
        printf("%d.Field: %s\n", i + 1, row->fields[i]); // Print each field
    }

    free_row(row); // Free the memory allocated to the row
    free(reader);  // Free the memory allocated to the reader
    fclose(fp);

    return 0;
}