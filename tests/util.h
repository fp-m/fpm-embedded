extern "C" {

extern const char *input;   // Input stream for the current test, utf-8 encoded

//
// Get Unicode character from input buffer.
//
char fpm_getchar();

//
// Write Unicode character to output buffer.
//
void fpm_putchar(char ch);

void disk_setup(void);

//
// Create a file with given name and contents.
//
void write_file(const char *filename, const char *contents);

//
// Check contents of the file.
//
void read_file(const char *filename, const char *contents);

//
// Create new directory.
//
void create_directory(const char *dirname);

//
// Make sure directory exists.
//
void check_directory(const char *dirname);

};
