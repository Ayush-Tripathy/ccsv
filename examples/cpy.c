#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ccsv.h"

int main(int argc, char *argv[])
{
    clock_t start = clock();

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];

    ccsv_reader *reader = ccsv_init_reader(NULL);
    if (reader == NULL || reader == (void *)CCSV_ERNOMEM)
    {
        fprintf(stderr, "Error initializing CSV reader\n");
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        free(reader);
        return 1;
    }

    FILE *output = fopen("output.csv", "w+");
    if (output == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        free(reader);
        fclose(file);
        return 1;
    }

    ccsv_writer *writer = ccsv_init_writer(NULL);

    CSVRow *row;

    size_t fields_count = 0;
    while ((row = read_row(file, reader)) != NULL)
    {
        fields_count += row->fields_count;
        write_row(output, writer, *row);
        free_row(row);
    }

    printf("%s - %d rows, %ld fields\n", argv[1], reader->rows_read, fields_count);
    printf("Output written to output.csv\n");

    fclose(file);
    fclose(output);
    free(reader);
    free(writer);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time taken: %lf seconds\n", time_spent);

    return 0;
}
