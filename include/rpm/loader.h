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

} dyn_object_t;

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
// dyn_symbol symtab[] = { ... };
// dyn_bind(&dynobj, symtab);
// dyn_run(&dynobj, argc, argv);
// dyn_call(&dynobj, name, argc, argv);
// dyn_locate(&dynobj, name) -> pointer
//

#ifdef __cplusplus
}
#endif

#endif // RPM_LOADER_H
