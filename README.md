# ccsv
Fast, flexible, easy-to-use CSV reading, writing library for C

For full documentation, see the [docs](https://github.com/Ayush-Tripathy/ccsv/tree/main/docs)

## Usage
### Create a reader object

```c
ccsv_reader *reader = ccsv_open("../../comments.csv", CCSV_READER, "r", &options, NULL); // NULL for default options
```

### Create a reader object with custom options

```c
ccsv_reader_options options = {
        .delim = ',',
        .quote_char = '"',
        .skip_initial_space = 0,
        .skip_empty_lines = 1,
        .skip_comments = 1};

// Initialize the reader with the options
ccsv_reader *reader = ccsv_init_reader(&options);
```


### Read a row with

```c
ccsv_row *row = ccsv_row(reader); // Will return NULL if all rows are read 
```

`reader` is a ccsv_reader object

### Get the number of fields in a row with

```c
int row_len = row->fields_count;
```

### Get a field from a row with

```c
char *field = row->fields[0]; // 0 for the first field
```

### Free the memory allocated to a row with

```c
free_row(row);
```

### Free the memory allocated to the reader with

```c
free(reader);
```



## Example

```c
#include <stdio.h>
#include <stdlib.h>

#include "../include/ccsv.h"

int main(void)
{
    ccsv_reader_options options = {
        .delim = ',',
        .quote_char = '"',
        .skip_comments = 1,
        .skip_initial_space = 0,
        .skip_empty_lines = 0,
    };
    // Reader object
    ccsv_reader *reader = ccsv_open("../../comments.csv", CCSV_READER, "r", &options, NULL); // NULL for default options
    if (reader == NULL)
    {
        fprintf(stderr, "Error initializing CSV reader\n");
        return 1;
    }

    ccsv_row *row;

    // Read each row and print each field
    while ((row = ccsv_next(reader)) != NULL)
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

    ccsv_close(reader); // Close the reader

    return 0;
}
```

#### You can find more examples in the `examples` folder

Compile with `make ./example_file_name`


For full documentation, see the [docs](https://github.com/Ayush-Tripathy/ccsv/tree/main/docs)

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE)

## Feel free to contribute!
