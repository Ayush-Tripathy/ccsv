# ccsv
Fast, flexible, easy-to-use CSV reading, writing library for C

For full documentation, see the [docs](https://github.com/Ayush-Tripathy/ccsv/tree/main/docs)

## Usage
### Create a reader object

```c
ccsv_reader *reader = ccsv_init_reader(NULL); // NULL for default options
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
CSVRow *row = read_row(fp, reader); // Will return NULL if all rows are read 
```

`fp` is a FILE pointer to the CSV file you want to read

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

#include "ccsv.h"

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


For full documentation, see the [docs](https://github.com/Ayush-Tripathy/ccsv/tree/main/docs)

## License
This project is licensed under the MIT License. See the [LICENSE](LICENSE)

## Feel free to contribute!
