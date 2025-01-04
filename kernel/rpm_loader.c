//
// Test dynamic loader.
//
#include <rpm/api.h>
#include <rpm/loader.h>
#include <rpm/elf.h>
#include <alloca.h>
#if __unix__ || __APPLE__
#   include <unistd.h>
#   include <fcntl.h>
#   include <sys/mman.h>
#   include <sys/stat.h>
#else
#   error "This platform is not supported"
#endif

#if __x86_64__ || __i386__
#   include <asm/prctl.h>
#   include <sys/syscall.h>
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
#elif __ARM_ARCH_ISA_A64
    static const unsigned NATIVE_MACHINE = EM_AARCH64;
#elif __ARM_ARCH_ISA_ARM
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
// Get name from .dynsym section.
//
static const char *dyn_get_name(dyn_object_t *dynobj, unsigned reloc_index)
{
    // Get pointer to REL or RELA section.
    const Native_Shdr *rel_section = (const Native_Shdr *) dynobj->rel_section;

    // Get index of the symbol in .dynsym section.
    unsigned dynsym_index;
    if (rel_section->sh_type == SHT_RELA) {
        const Native_Rela *relocations = (const Native_Rela *) (rel_section->sh_offset + (char*)dynobj->base);
        dynsym_index = NATIVE_R_SYM(relocations[reloc_index].r_info);
    } else {
        const Native_Rel *relocations = (const Native_Rel *) (rel_section->sh_offset + (char*)dynobj->base);
        dynsym_index = NATIVE_R_SYM(relocations[reloc_index].r_info);
    }

    // Get pointer to .dynsym contents.
    const Native_Shdr *dyn_section = dyn_section_by_index(dynobj, rel_section->sh_link);
    const Native_Sym *symbols      = (const Native_Sym *) (dyn_section->sh_offset + (char*)dynobj->base);

    // Get pointer to .dynstr contents.
    const Native_Shdr *str_section = dyn_section_by_index(dynobj, dyn_section->sh_link);
    const char *strings            = str_section->sh_offset + (char*)dynobj->base;

    // Get name from .dynsym section.
    return &strings[symbols[dynsym_index].st_name];
}

//
// Get names of linked procedures.
// Array result[] must have dynobj->num_links entries.
//
void dyn_get_symbols(dyn_object_t *dynobj, const char *result[])
{
    for (unsigned index = 0; index < dynobj->num_links; index++) {
        result[index] = dyn_get_name(dynobj, index);
    }
}

//
// Search linkmap for a given name.
// Return address of the symbol, or NULL on failure.
//
static void *find_address_by_name(const dyn_linkmap_t *linkmap, const char *name)
{
    for (;;) {
        // Skip first entry - it's a parent link.
        for (unsigned index = 1; linkmap[index].name != NULL; index++) {
            if (strcmp(name, linkmap[index].name) == 0) {
                return linkmap[index].address;
            }
        }

        // Name not found in this link map.
        // Search parent recursively.
        linkmap = (const dyn_linkmap_t *) linkmap[0].address;
        if (linkmap == NULL) {
            return NULL;
        }
    }
}

//
// Setup arch-dependent GOT register.
//
static void set_got_pointer(void *addr)
{
#if __ARM_ARCH_6M__
    // For RP2040: use slot #4 of the interrupt vector table,
    // at address 0x2000 0010. This vector is unused by hardware.
    *(volatile void**) 0x20000010 = addr;

#elif __x86_64__ || __i386__
    // For x86-64 or i386 Linux: use %gs register.
    syscall(SYS_arch_prctl, ARCH_SET_GS, addr);

#elif __ARM_ARCH_ISA_A64
    // For arm64 Linux or MacOS: use TPIDR_EL0 register.
    asm volatile("msr tpidr_el0, %0" : : "r" (addr) : "memory");

#elif __ARM_ARCH_ISA_ARM
    // For arm32 Linux: use TPIDRURW register.
    asm volatile("mcr p15, 0, %0, c13, c0, 2" : : "r" (addr) : "memory");
#else
    //TODO: other platforms.
#   error "This platform is not supported"
#endif
}

//
// Invoke entry address of the ELF binary with argc, argv arguments.
// Bind dynamic symbols of the binary according to the given linkmap.
// Assume the entry has signature:
//
//      int main(int argc, char *argv[])
//
// Return the exit code.
//
bool dyn_execv(dyn_object_t *dynobj, dyn_linkmap_t linkmap[], int argc, const char *argv[])
{
    // Build a Global Offset Table on stack.
    void **got = alloca(dynobj->num_links);

    // Bind dynamic symbols.
    unsigned fail_count = 0;
    for (unsigned index = 0; index < dynobj->num_links; index++) {

        // Find symbol's name and address.
        const char *name = dyn_get_name(dynobj, index);
        void *address    = find_address_by_name(linkmap, name);

        got[index] = address;
        if (address == NULL) {
            rpm_printf("%s: Symbol not found\r\n", name);
            fail_count++;
        }
    }
    if (fail_count > 0) {
        // Cannot map some symbols.
        return false;
    }
    set_got_pointer(got);

    // Compute entry address.
    typedef int (*entry_t)(int, const char **);
    const Native_Ehdr *hdr = dynobj->base;
    const entry_t entry    = (entry_t) (hdr->e_entry + (char*)dynobj->base);

    // Invoke ELF binary.
    dynobj->exit_code = (*entry)(argc, argv);
    return true;
}
