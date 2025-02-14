
# CC = gcc
# Set CC to your target's 'C' compiler to build for it
# Example:
#       make CC=/arm/bin/gcc clean vi
CFLAGS = -Wall -O

# LD = gcc
LDFLAGS =

vi : vi.c Makefile
	$(CC) $(CFLAGS) vi.c -o vi

clean:
	rm -f vi

distclean: clean

realclean: distclean
