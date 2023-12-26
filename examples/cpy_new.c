#include <stdio.h>
#include <time.h>

#include "ccsv.h"

int main(int argc, char **argv)
{
    clock_t start = clock();

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        return 1;
    }

    char *source = argv[1];
    char *destination = argv[2];

    ccsv_reader *reader = ccsv_open(source, CCSV_READER, "r", NULL, NULL);
    if (reader == NULL)
    {
        fprintf(stderr, "Error initializing CSV reader\n");
        return 1;
    }

    ccsv_writer *writer = ccsv_open(destination, CCSV_WRITER, "w+", NULL, NULL);
    if (writer == NULL)
    {
        fprintf(stderr, "Error initializing CSV writer\n");
        return 1;
    }

    ccsv_row *row;
    while ((row = ccsv_next(reader)) != NULL)
    {
        ccsv_write(writer, *row);
        ccsv_free_row(row);
    }

    printf("Rows read: %d\n", reader->rows_read);

    printf("CSV file written to %s\n", destination);

    ccsv_close(reader);
    ccsv_close(writer);

    clock_t end = clock();
    double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time taken: %lf seconds\n", time_spent);

    return 0;
}