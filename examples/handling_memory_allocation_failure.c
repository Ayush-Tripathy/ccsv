#include <stdio.h>
#include <stdlib.h>

#include "../include/ccsv.h"

int main(void)
{
    FILE *csv_file = fopen("../../ign.csv", "r");
    if (csv_file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    ccsv_reader_options options = {
        .delim = ',', // Example delimiter, change according to your CSV file
        .quote_char = '"',
        .skip_initial_space = 0,
        // Add other options if necessary
    };

    ccsv_reader *reader = ccsv_init_reader(&options);
    if (reader == (void *)CCSV_ERNOMEM)
    {
        fprintf(stderr, "Error initializing CSV reader\n");
        fclose(csv_file);
        return 1;
    }

    CSVRow *row;
    while (1)
    {
        row = read_row(csv_file, reader);
        if (row == NULL)
        {
            break;
        }
        else if (row == (void *)CCSV_ERNOMEM)
        {
            fprintf(stderr, "Memory allocation failure while reading row\n");
            break;
        }

        int fields_count = row->fields_count;
        for (int i = 0; i < fields_count; ++i)
        {
            printf("%s\t", row->fields[i]);
        }
        printf("\n");

        free_row(row);
    }
    printf("\n\nRows read: %d\n", reader->rows_read);

    fclose(csv_file);
    free(reader);

    return 0;
}
