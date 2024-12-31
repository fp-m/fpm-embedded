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
    int fd;
    size_t file_size;
    void *base;

    // ELF executable header (EHDR).
    size_t   e_entry;     // Start address
    size_t   e_phoff;     // File offset to the PHDR table
    size_t   e_shoff;     // File offset to the SHDRheader
    uint32_t e_flags;     // Flags (EF_*)
    uint16_t e_ehsize;    // Elf header size in bytes
    uint16_t e_phentsize; // PHDR table entry size in bytes
    uint16_t e_phnum;     // Number of PHDR entries
    uint16_t e_shentsize; // SHDR table entry size in bytes
    uint16_t e_shnum;     // Number of SHDR entries
    uint16_t e_shstrndx;  // Index of section name string table

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
