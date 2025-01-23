//
// Export dynamically linked routines.
//
#include <fpm/api.h>
#include <fpm/loader.h>
#include <fpm/fs.h>
#include <fpm/getopt.h>
#include <stdlib.h>

fpm_binding_t fpm_bindings[] = {
#include <fpm/bindings.h>
    {},
};
