//
// Dynamic Loader Interface.
//
#ifndef RPM_LOADER_H
#define RPM_LOADER_H

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
} rpm_executable_t;

//
// Definition of exported procedure for dynamic linking.
//
typedef struct {
    const char *name;       // Name of procedure
    void *address;          // Address of procedure
} rpm_binding_t;

//
// Load dynamic binary.
// Return true on success.
//
bool rpm_load(rpm_executable_t *dynobj, const char *filename);

//
// Unmap ELF binary from memory.
//
void rpm_unload(rpm_executable_t *dynobj);

//
// Get names of linked procedures.
//
void rpm_get_symbols(rpm_executable_t *dynobj, const char *symbols[]);

//
// Invoke entry address of the ELF binary with argc, argv arguments.
// Bind dynamic symbols of the binary according to the given linkmap.
// Assume the entry has signature:
//
//      int main(int argc, char *argv[])
//
// Return the exit code.
//
bool rpm_execv(rpm_executable_t *dynobj, rpm_binding_t linkmap[], int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif // RPM_LOADER_H
