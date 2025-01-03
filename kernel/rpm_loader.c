//
// Test dynamic loader.
//
#include <rpm/api.h>
#include <rpm/loader.h>
#include <rpm/elf.h>
#if __unix__ || __APPLE__
#   include <unistd.h>
#   include <fcntl.h>
#   include <sys/mman.h>
#   include <sys/stat.h>
#else
#   error "This platform is not supported"
#endif

#if __SIZE_WIDTH__ == 64
    typedef Elf64_Ehdr Native_Ehdr;
    typedef Elf64_Shdr Native_Shdr;
    typedef Elf64_Rela Native_Rela;
    typedef Elf64_Rel Native_Rel;
    typedef Elf64_Sym Native_Sym;
#   define NATIVE_R_TYPE(x) ELF64_R_TYPE(x)
#   define NATIVE_R_SYM(x) ELF64_R_SYM(x)
#else
    typedef Elf32_Ehdr Native_Ehdr;
    typedef Elf32_Shdr Native_Shdr;
    typedef Elf32_Rela Native_Rela;
    typedef Elf32_Rel Native_Rel;
    typedef Elf32_Sym Native_Sym;
#   define NATIVE_R_TYPE(x) ELF32_R_TYPE(x)
#   define NATIVE_R_SYM(x) ELF32_R_SYM(x)
#endif

//
// Find section by type.
//
static const Native_Shdr *dyn_section_by_type(dyn_object_t *dynobj, unsigned type)
{
    const Native_Ehdr *hdr     = dynobj->base;
    const Native_Shdr *section = (const Native_Shdr *) (hdr->e_shoff + (char*)dynobj->base);

    for (unsigned i = 0; i < hdr->e_shnum; i++) {
        if (section[i].sh_type == type) {
            return &section[i];
        }
    }
    return NULL;
}

//
// Get section by index.
//
static const Native_Shdr *dyn_section_by_index(dyn_object_t *dynobj, unsigned index)
{
    const Native_Ehdr *hdr     = dynobj->base;
    const Native_Shdr *section = (const Native_Shdr *) (hdr->e_shoff + (char*)dynobj->base);

    return &section[index];
}

//
// Map ELF binary into memory.
//
bool dyn_load(dyn_object_t *dynobj, const char *filename)
{
#if __unix__ || __APPLE__
    // Open the shared library file in read-only mode
    dynobj->fd = open(filename, O_RDONLY);
    if (dynobj->fd < 0) {
        rpm_printf("%s: Cannot open\r\n", filename);
        return false;
    }

    // Get the size of the file
    struct stat sb;
    if (fstat(dynobj->fd, &sb) < 0) {
        rpm_printf("%s: Cannot fstat\r\n", filename);
err:    close(dynobj->fd);
        return false;
    }
    dynobj->file_size = sb.st_size;

    // Map the file into memory
    dynobj->base = mmap(NULL, dynobj->file_size, PROT_READ | PROT_EXEC, MAP_SHARED, dynobj->fd, 0);
    if (dynobj->base == MAP_FAILED) {
        rpm_printf("%s: Cannot mmap\r\n", filename);
        goto err;
    }
#endif

    //
    // Check file format.
    //
    const char *id = (const char*) dynobj->base;
    if (id[EI_MAG0] != ELFMAG0 || id[EI_MAG1] != ELFMAG1 ||
        id[EI_MAG2] != ELFMAG2 || id[EI_MAG3] != ELFMAG3) {
        rpm_printf("%s: Not ELF binary\r\n", filename);
        goto err;
    }
    if (id[EI_VERSION] != EV_CURRENT) {
        rpm_printf("%s: Incompatible ELF version\r\n", filename);
        goto err;
    }

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    static const char NATIVE_ENDIANNESS = ELFDATA2LSB;
#else
    static const char NATIVE_ENDIANNESS = ELFDATA2MSB;
#endif
    if (id[EI_DATA] != NATIVE_ENDIANNESS) {
        rpm_printf("%s: Incompatible endianness\r\n", filename);
        goto err;
    }

#if __SIZE_WIDTH__ == 64
    static const char NATIVE_CLASS = ELFCLASS64;
#else
    static const char NATIVE_CLASS = ELFCLASS32;
#endif
    if (id[EI_CLASS] != NATIVE_CLASS) {
        rpm_printf("%s: Incompatible word size\r\n", filename);
        goto err;
    }

    const Native_Ehdr *hdr = dynobj->base;
    if (hdr->e_type != ET_DYN) {
        rpm_printf("%s: Bad exec type\r\n", filename);
        goto err;
    }

#if __x86_64__
    static const unsigned NATIVE_MACHINE = EM_X86_64;
#elif __i386__
    static const unsigned NATIVE_MACHINE = EM_386;
#elif __ARM_ARCH_ISA_ARM
    static const unsigned NATIVE_MACHINE = EM_ARM;
#elif __ARM_ARCH_ISA_A64
    static const unsigned NATIVE_MACHINE = EM_ARM;
#elif __mips__
    static const unsigned NATIVE_MACHINE = EM_MIPS;
#elif __riscv
    static const unsigned NATIVE_MACHINE = EM_RISCV;
#else
#   error "This architecture is not supported"
#endif
    if (hdr->e_machine != NATIVE_MACHINE) {
        rpm_printf("%s: Incompatible machine\r\n", filename);
        goto err;
    }

    // Find relocation section.
    const Native_Shdr *rel_section = dyn_section_by_type(dynobj, SHT_RELA);
    if (rel_section == NULL) {
        rel_section = dyn_section_by_type(dynobj, SHT_REL);
        if (rel_section == NULL) {
            rpm_printf("%s: No relocation section\r\n", filename);
            goto err;
        }
    }
    dynobj->rel_section = rel_section;

    // Number of linked procedures.
    dynobj->num_links = rel_section->sh_size / rel_section->sh_entsize;
    return true;
}

//
// Unmap ELF binary from memory.
//
void dyn_unload(dyn_object_t *dynobj)
{
    // Unmap the file from memory.
    if (dynobj->base != NULL) {
         munmap(dynobj->base, dynobj->file_size);
         dynobj->base = NULL;
    }

    // Close file.
    // Note: descriptor cannot be zero.
    if (dynobj->fd > 0) {
        close(dynobj->fd);
        dynobj->fd = 0;
    }
}

//
// Get names from RELA section.
//
static void dyn_get_symbols_rela(dyn_object_t *dynobj, const Native_Shdr *rela_section, const char *result[])
{
    const Native_Rela *relocations = (const Native_Rela *) (rela_section->sh_offset + (char*)dynobj->base);

#if 0
    rpm_printf("Relocation section:\r\n");
    rpm_printf("    name      = 0x%x\r\n", rela_section->sh_name);      // index of section name
    rpm_printf("    type      = 0x%x\r\n", rela_section->sh_type);      // section type
    rpm_printf("    flags     = 0x%x\r\n", rela_section->sh_flags);     // section flags
    rpm_printf("    addr      = 0x%x\r\n", rela_section->sh_addr);      // in-memory address of section
    rpm_printf("    offset    = 0x%x\r\n", rela_section->sh_offset);    // file offset of section
    rpm_printf("    size      = 0x%x\r\n", rela_section->sh_size);      // section size in bytes
    rpm_printf("    link      = 0x%x\r\n", rela_section->sh_link);      // section header table link
    rpm_printf("    info      = 0x%x\r\n", rela_section->sh_info);      // extra information
    rpm_printf("    addralign = 0x%x\r\n", rela_section->sh_addralign); // alignment constraint
    rpm_printf("    entsize   = 0x%x\r\n", rela_section->sh_entsize);   // size for fixed-size entries
    rpm_printf("Relocation entries:\r\n");
#endif

    // Get pointer to .dynsym contents.
    const Native_Shdr *dyn_section = dyn_section_by_index(dynobj, rela_section->sh_link);
    const Native_Sym *symbols      = (const Native_Sym *) (dyn_section->sh_offset + (char*)dynobj->base);

    // Get pointer to .dynstr contents.
    const Native_Shdr *str_section = dyn_section_by_index(dynobj, dyn_section->sh_link);
    const char *strings            = str_section->sh_offset + (char*)dynobj->base;

    for (unsigned reloc_index = 0; reloc_index < dynobj->num_links; reloc_index++) {
        const Native_Rela *item = &relocations[reloc_index];
        unsigned dynsym_index   = NATIVE_R_SYM(item->r_info);

        // Get name from .dynsym section.
        result[reloc_index] = &strings[symbols[dynsym_index].st_name];
#if 0
        rpm_printf("    entry: %u\r\n", index);
        rpm_printf("        offset: %#zx\r\n", (size_t) item->r_offset);
        rpm_printf("          info: %#zx\r\n", (size_t) item->r_info);
        rpm_printf("           sym: %#zx\r\n", (size_t) NATIVE_R_SYM(item->r_info));
        rpm_printf("          type: %#zx\r\n", (size_t) NATIVE_R_TYPE(item->r_info));
        rpm_printf("        addend: %zd\r\n", (size_t) item->r_addend);
        rpm_printf("          name: %s\r\n", result[reloc_index]);
#endif
    }
}

//
// Get names from REL section.
//
static void dyn_get_symbols_rel(dyn_object_t *dynobj, const Native_Shdr *rel_section, const char *result[])
{
    const Native_Rel *relocations = (const Native_Rel *) (rel_section->sh_offset + (char*)dynobj->base);

#if 0
    rpm_printf("Relocation section:\r\n");
    rpm_printf("    name      = 0x%x\r\n", rel_section->sh_name);      // index of section name
    rpm_printf("    type      = 0x%x\r\n", rel_section->sh_type);      // section type
    rpm_printf("    flags     = 0x%x\r\n", rel_section->sh_flags);     // section flags
    rpm_printf("    addr      = 0x%x\r\n", rel_section->sh_addr);      // in-memory address of section
    rpm_printf("    offset    = 0x%x\r\n", rel_section->sh_offset);    // file offset of section
    rpm_printf("    size      = 0x%x\r\n", rel_section->sh_size);      // section size in bytes
    rpm_printf("    link      = 0x%x\r\n", rel_section->sh_link);      // section header table link
    rpm_printf("    info      = 0x%x\r\n", rel_section->sh_info);      // extra information
    rpm_printf("    addralign = 0x%x\r\n", rel_section->sh_addralign); // alignment constraint
    rpm_printf("    entsize   = 0x%x\r\n", rel_section->sh_entsize);   // size for fixed-size entries
    rpm_printf("Relocation entries:\r\n");
#endif

    // Get pointer to .dynsym contents.
    const Native_Shdr *dyn_section = dyn_section_by_index(dynobj, rel_section->sh_link);
    const Native_Sym *symbols      = (const Native_Sym *) (dyn_section->sh_offset + (char*)dynobj->base);

    // Get pointer to .dynstr contents.
    const Native_Shdr *str_section = dyn_section_by_index(dynobj, dyn_section->sh_link);
    const char *strings            = str_section->sh_offset + (char*)dynobj->base;

    for (unsigned reloc_index = 0; reloc_index < dynobj->num_links; reloc_index++) {
        const Native_Rel *item = &relocations[reloc_index];
        unsigned dynsym_index  = NATIVE_R_SYM(item->r_info);

        // Get name from .dynsym section.
        result[reloc_index] = &strings[symbols[dynsym_index].st_name];
#if 0
        rpm_printf("    entry: %u\r\n", index);
        rpm_printf("        offset: %#zx\r\n", (size_t) item->r_offset);
        rpm_printf("          info: %#zx\r\n", (size_t) item->r_info);
        rpm_printf("           sym: %#zx\r\n", (size_t) NATIVE_R_SYM(item->r_info));
        rpm_printf("          type: %#zx\r\n", (size_t) NATIVE_R_TYPE(item->r_info));
        // No addend.
        rpm_printf("          name: %s\r\n", result[reloc_index]);
#endif
    }
}

//
// Get names of linked procedures.
// Array result[] must have dynobj->num_links entries.
//
void dyn_get_symbols(dyn_object_t *dynobj, const char *result[])
{
    // Get pointer to .rela.plt contents.
    const Native_Shdr *section = (const Native_Shdr *) dynobj->rel_section;
    if (section->sh_type == SHT_RELA) {
        dyn_get_symbols_rela(dynobj, section, result);
    } else {
        dyn_get_symbols_rel(dynobj, section, result);
    }
}
