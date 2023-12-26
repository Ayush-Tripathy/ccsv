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

    ccsv_reader_options options = {.skip_initial_space = 1, .skip_empty_lines = 0};

    ccsv_reader *reader = ccsv_open(filename, CCSV_READER, "r", &options, NULL);
    if (reader == NULL || ccsv_is_error(reader, NULL))
    {
        fprintf(stderr, "Error initializing CSV reader\n");
        return 1;
    }

    ccsv_row *row;

    size_t fields_count = 0;
    while ((row = ccsv_next(reader)) != NULL)
    {
        fields_count += row->fields_count;
        ccsv_free_row(row);
    }

    printf("%s: %d rows, %ld fields\n", argv[1], reader->rows_read, fields_count);

    ccsv_close(reader);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time taken: %lf seconds\n", time_spent);

    return 0;
}
