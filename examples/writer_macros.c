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

    FILE *file = fopen("output.csv", "w+");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        free(writer);
        return 1;
    }

    CCSV_WRITE_ROW_START(file, writer);                // Write row start
    CCSV_WRITE_FIELD(file, writer, "hi");              // Write field
    CCSV_WRITE_FIELD(file, writer, "hello, world!");   // Write field
    CCSV_WRITE_FIELD(file, writer, "\"escapedword\""); // Write field
    CCSV_WRITE_ROW_END(file, writer, NULL);            // Write row end

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
