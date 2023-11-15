CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I../include

%: %.c
	$(CC) $(CFLAGS) -o $@.out $< ../src/ccsv.c

clean:
	rm -f *.out
