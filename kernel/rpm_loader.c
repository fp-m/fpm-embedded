//
// Test dynamic loader.
//
#include <rpm/api.h>
#include <rpm/loader.h>
#include <rpm/elf.h>
#if __unix__
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
#else
    typedef Elf32_Ehdr Native_Ehdr;
    typedef Elf32_Shdr Native_Shdr;
#endif

//
// Find section by type.
//
static const Native_Shdr *dyn_find_section(dyn_object_t *dynobj, unsigned type)
{
    const Native_Shdr *section = (const Native_Shdr *) (dynobj->e_shoff + (char*)dynobj->base);

    for (unsigned i = 0; i < dynobj->e_shnum; i++) {
        if (section[i].sh_type == type) {
            return &section[i];
        }
    }
    return NULL;
}

//
// Map ELF binary into memory.
//
bool dyn_load(dyn_object_t *dynobj, const char *filename)
{
#if __unix__
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
    const Native_Ehdr *hdr = dynobj->base;
    if (id[EI_CLASS] != NATIVE_CLASS) {
        rpm_printf("%s: Incompatible word size\r\n", filename);
        goto err;
    }

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

    //
    // Copy fields of exec header.
    //
    dynobj->e_entry     = hdr->e_entry;
    dynobj->e_phoff     = hdr->e_phoff;
    dynobj->e_shoff     = hdr->e_shoff;
    dynobj->e_flags     = hdr->e_flags;
    dynobj->e_ehsize    = hdr->e_ehsize;
    dynobj->e_phentsize = hdr->e_phentsize;
    dynobj->e_phnum     = hdr->e_phnum;
    dynobj->e_shentsize = hdr->e_shentsize;
    dynobj->e_shnum     = hdr->e_shnum;
    dynobj->e_shstrndx  = hdr->e_shstrndx;

    // Find relocation section.
    const Native_Shdr *rel_sect = dyn_find_section(dynobj, SHT_RELA);
    if (rel_sect == NULL) {
        rpm_printf("%s: No relocation section\r\n", filename);
        goto err;
    }
#if 1
    rpm_printf("Relocation section:\r\n");
    rpm_printf("    name      = 0x%x\r\n", rel_sect->sh_name);      // index of section name
    rpm_printf("    type      = 0x%x\r\n", rel_sect->sh_type);      // section type
    rpm_printf("    flags     = 0x%x\r\n", rel_sect->sh_flags);     // section flags
    rpm_printf("    addr      = 0x%x\r\n", rel_sect->sh_addr);      // in-memory address of section
    rpm_printf("    offset    = 0x%x\r\n", rel_sect->sh_offset);    // file offset of section
    rpm_printf("    size      = 0x%x\r\n", rel_sect->sh_size);      // section size in bytes
    rpm_printf("    link      = 0x%x\r\n", rel_sect->sh_link);      // section header table link
    rpm_printf("    info      = 0x%x\r\n", rel_sect->sh_info);      // extra information
    rpm_printf("    addralign = 0x%x\r\n", rel_sect->sh_addralign); // alignment constraint
    rpm_printf("    entsize   = 0x%x\r\n", rel_sect->sh_entsize);   // size for fixed-size entries
#endif
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
