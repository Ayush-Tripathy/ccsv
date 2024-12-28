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

  /*
  ------- Way 1 -------
  ccsv_reader_options *options = (ccsv_reader_options *)malloc(sizeof(ccsv_reader_options));
  options->delim = ',';
  options->quote_char = '"';
  options->skip_initial_space = 0;
  */

  /* OR */

  ccsv_reader_options options = {
      .delim = ',',
      .quote_char = '"',
      .skip_initial_space = 0,
      .skip_empty_lines = 1,
      .skip_comments = 1};

  // Reader object
  ccsv_reader *reader = ccsv_open_from_file(fp, CCSV_READER, "r", &options, NULL);
  // free(options); /* If you used Way 1 */

  ccsv_row *row;

  // Read each row and print each field
  while ((row = ccsv_next(reader)) != NULL)
  {
    int row_len = row->fields_count; // Get number of fields in the row
    for (int i = 0; i < row_len; i++)
    {
      printf("%s\t", row->fields[i]); // Print each field
    }
    printf("\n");
    ccsv_free_row(row); // Free the memory allocated to the row
  }
  printf("\n\nRows read: %d\n", reader->rows_read); // Print number of rows read

  free(reader); // Free the memory allocated to the reader
  fclose(fp);

  return 0;
}