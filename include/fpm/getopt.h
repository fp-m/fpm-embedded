//
// Get long options from command line argument list.
//
#ifdef __cplusplus
extern "C" {
#endif

enum {                    // For has_arg:
    FPM_NO_ARG = 0,       // no argument to the option is expected
    FPM_REQUIRED_ARG = 1, // an argument to the option is required
    FPM_OPTIONAL_ARG = 2, // an argument to the option may be presented
};

struct fpm_option {
    const char *name; // The name of the long option.
    int has_arg;      // One of the above enums.
    int *flag;        // Determines if fpm_getopt() returns a value for a long option.
                      // If it is non-NULL, 0 is returned as a function value and
                      // the value of val is stored in the area pointed to by flag.
                      // Otherwise, val is returned.
    int val;          // Determines the value to return if flag is NULL.

};

struct fpm_opt {
    int ret;         // Returned value
    int opt;         // Current option
    const char *arg; // Current argument
    int long_index;  // Index of long option
    int ind;         // Index of next argv
    int silent;      // Suppress error messages
    int where;       // Offset inside current argument
};

int fpm_getopt(int argc, char *const argv[], const char *optstring,
               const struct fpm_option *longopts, struct fpm_opt *opt);

#ifdef __cplusplus
}
#endif
