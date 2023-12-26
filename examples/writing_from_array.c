#include <stdio.h>
#include <stdlib.h>
#include "../include/ccsv.h"

int main(void)
{
    // Initialize ccsv_writer_options
    ccsv_writer_options options = {
        .delim = ',',
        .quote_char = '"'
        // Add other options if necessary
    };

    // Initialize the writer
    ccsv_writer *writer = ccsv_init_writer(&options, NULL);
    if (writer == NULL)
    {
        fprintf(stderr, "Error initializing CSV writer\n");
        return 1;
    }

    FILE *file = fopen("output.csv", "a+");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        free(writer);
        return 1;
    }

    char *row_string[] = {"hi", "hello", "hello, world!", "\"escapedword\"", "hola", "bonjour"};

    write_row_from_array(file, writer, row_string, ARRAY_LEN(row_string)); /* Write row to file */

    short err_status;
    if (ccsv_is_error(writer, &err_status))
    {
        fprintf(stderr, "Error writing CSV row from string: %s\n", ccsv_get_status_message(err_status));
        fclose(file);
        free(writer);
        return 1;
    }

    fclose(file);
    free(writer);

    return 0;
}
