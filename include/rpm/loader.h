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

} dyn_object_t;

//
// Definition of exported procedure for dynamic linking.
//
typedef struct {
    const char *name;       // Name of procedure
    void *address;          // Address of procedure
} dyn_linkmap_t;

//
// Load dynamic binary.
// Return true on success.
//
bool dyn_load(dyn_object_t *dynobj, const char *filename);

//
// Unmap ELF binary from memory.
//
void dyn_unload(dyn_object_t *dynobj);

//
// Get names of linked procedures.
//
void dyn_get_symbols(dyn_object_t *dynobj, const char *symbols[]);

//
// Invoke entry address of the ELF binary with argc, argv arguments.
// Bind dynamic symbols of the binary according to the given linkmap.
// Assume the entry has signature:
//
//      int main(int argc, char *argv[])
//
// Return the exit code.
//
bool dyn_execv(dyn_object_t *dynobj, dyn_linkmap_t linkmap[], int argc, const char *argv[]);

#ifdef __cplusplus
}
#endif

#endif // RPM_LOADER_H
