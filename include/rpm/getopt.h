//
// Get long options from command line argument list.
//
#ifdef __cplusplus
extern "C" {
#endif

enum {                    // For has_arg:
    RPM_NO_ARG = 0,       // no argument to the option is expected
    RPM_REQUIRED_ARG = 1, // an argument to the option is required
    RPM_OPTIONAL_ARG = 2, // an argument to the option may be presented
};

struct rpm_option {
    const char *name;   // The name of the long option.
    int has_arg;        // One of the above enums.
    int *flag;          // Determines if rpm_getopt() returns a value for a long option.
                        // If it is non-NULL, 0 is returned as a function value and
                        // the value of val is stored in the area pointed to by flag.
                        // Otherwise, val is returned.
    int val;            // Determines the value to return if flag is NULL.

};

struct rpm_opt {
    int ret;        // Returned value
    int opt;        // Current option
    char *arg;      // Current argument
    int long_index; // Index of long option
    int ind;        // Index of next argv
    int silent;     // Suppress error messages
    int where;      // Offset inside current argument
};

int rpm_getopt(int argc, char *const *argv, const char *optstring,
               const struct rpm_option *longopts, struct rpm_opt *opt);

#ifdef __cplusplus
}
#endif
