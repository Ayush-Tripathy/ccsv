# ccsv
Simple, not slow CSV parsing library for C

## Usage
### Create a reader object

```
ccsv_reader *reader = ccsv_init_reader(NULL); // NULL for default options
```

### Create a reader object with custom options

```
ccsv_options *options = malloc(sizeof(ccsv_options));

options->delim = ','; // Specify the delimiter
options->quote_char = '"'; // Specify the quote character
options->escape_seq = "\"\""; // Specify the escape character
options->ignore_comment = 1; // Specify whether to skip comment lines or not

// Below options are not working properly yet
options->ignore_empty_lines = 1; // Specify whether to skip empty rows or not
options->ignore_escape_sequence = 1; // Specify whether to ignore escape sequences or not

ccsv_reader *reader = ccsv_init_reader(options);
// After initializing the reader, you can
free(options); // Free the memory allocated to the options


```


### Read a row with

```
CSVRow *row = read_row(fp, reader); // Will return NULL if all rows are read 
```

`fp` is a FILE pointer to the CSV file you want to read

`reader` is a ccsv_reader object

### Get the number of fields in a row with

```
int row_len = row->fields_count;
```

### Get a field from a row with

```
char *field = row->fields[0]; // 0 for the first field
```

### Free the memory allocated to a row with

```
free_row(row);
```

### Free the memory allocated to the reader with

```
free(reader);
```



## Example

```
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
```

#### You can find more examples in the `examples` folder

Compile with `make ./example_file_name`
