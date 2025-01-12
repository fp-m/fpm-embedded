//
// Dynamic Loader Interface.
//
#ifndef FPM_LOADER_H
#define FPM_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

//
// Dynamic object structure.
//
typedef struct {
    int fd;                  // File descriptor
    size_t file_size;        // Size of file in bytes
    void *base;              // File is mapped at this address
    unsigned num_links;      // Number of linked procedures
    const void *rel_section; // Header of .rela.plt section
    int exit_code;           // Return value of invoked object
} fpm_executable_t;

//
// Definition of exported procedure for dynamic linking.
//
typedef struct {
    const char *name;       // Name of procedure
    void *address;          // Address of procedure
} fpm_binding_t;

//
// Load dynamic binary.
// Return true on success.
//
bool fpm_load(fpm_executable_t *dynobj, const char *filename);

//
// Unmap ELF binary from memory.
//
void fpm_unload(fpm_executable_t *dynobj);

//
// Internal platform-dependent helper routines.
//
bool fpm_load_arch(fpm_executable_t *dynobj, const char *filename);
void fpm_unload_arch(fpm_executable_t *dynobj);

//
// Get names of linked procedures.
//
void fpm_get_symbols(fpm_executable_t *dynobj, const char *symbols[]);

//
// Invoke entry address of the ELF binary with argc, argv arguments.
// Bind dynamic symbols of the binary according to the given linkmap.
// Assume the entry has signature:
//
//      int main(int argc, char *argv[])
//
// Return the exit code.
//
bool fpm_execv(fpm_executable_t *dynobj, fpm_binding_t linkmap[], int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif // FPM_LOADER_H
