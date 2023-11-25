#include <stdio.h>
#include <stdlib.h>

#include "ccsv.h"

int main(void)
{
    // Initialize ccsv_writer_options
    ccsv_writer_options options = {
        .delim = ',',
        .quote_char = '"'
        // Add other options if necessary
    };

    // Initialize the writer
    ccsv_writer *writer = ccsv_init_writer(&options);
    if (writer == NULL || writer == (void *)CCSV_ERNOMEM)
    {
        fprintf(stderr, "Error initializing CSV writer\n");
        return 1;
    }

    ccsv_reader *reader = ccsv_init_reader(NULL);

    FILE *dest_file = fopen("output.csv", "a+");
    if (dest_file == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        free(writer);
        return 1;
    }
    FILE *source_file = fopen("../../ign.csv", "r");
    if (source_file == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        free(writer);
        return 1;
    }

    CSVRow *row = read_row(source_file, reader);
    write_row(dest_file, writer, *row); // Pass the value of the row pointer

    row = read_row(source_file, reader); // Read the next row
    write_row(dest_file, writer, *row);  // Write row to file

    if (ccsv_is_error(writer, NULL))
    {
        fprintf(stderr, "Error writing CSV row.\n");
        fclose(dest_file);
        fclose(source_file);
        free(reader);
        free(writer);
        return 1;
    }

    fclose(dest_file);
    fclose(source_file);
    free(reader);
    free(writer);

    return 0;
}
