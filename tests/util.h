extern const char *input;   // Input stream for the current test, utf-8 encoded

//
// Get Unicode character from input buffer.
//
char rpm_getchar();

//
// Write Unicode character to output buffer.
//
void rpm_putchar(char ch);

//
// Create a file with given name and contents.
//
void write_file(const char *filename, const char *contents);

//
// Check contents of the file.
//
void read_file(const char *filename, const char *contents);
