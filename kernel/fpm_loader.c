//
// Test dynamic loader.
//
#include <fpm/api.h>
#include <fpm/loader.h>
#include <fpm/context.h>
#include <fpm/elf.h>
#include <alloca.h>

#if (__x86_64__ || __i386__) && __unix__
    // MacOS/x86 is not supported
#   include <unistd.h>
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
static const Native_Shdr *fpm_section_by_type(fpm_context_t *ctx, unsigned type)
{
    const Native_Ehdr *hdr     = ctx->base;
    const Native_Shdr *section = (const Native_Shdr *) (hdr->e_shoff + (char*)ctx->base);

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
static const Native_Shdr *fpm_section_by_index(fpm_context_t *ctx, unsigned index)
{
    const Native_Ehdr *hdr     = ctx->base;
    const Native_Shdr *section = (const Native_Shdr *) (hdr->e_shoff + (char*)ctx->base);

    return &section[index];
}

//
// Map ELF binary into memory.
//
bool fpm_load(fpm_context_t *ctx, const char *filename)
{
    if (!fpm_load_arch(ctx, filename)) {
err:    fpm_unload_arch(ctx);
        return false;
    }

    //
    // Check file format.
    //
    const char *id = (const char*) ctx->base;
    if (id[EI_MAG0] != ELFMAG0 || id[EI_MAG1] != ELFMAG1 ||
        id[EI_MAG2] != ELFMAG2 || id[EI_MAG3] != ELFMAG3) {
        fpm_printf("%s: Not ELF binary\r\n", filename);
        goto err;
    }
    if (id[EI_VERSION] != EV_CURRENT) {
        fpm_printf("%s: Incompatible ELF version\r\n", filename);
        goto err;
    }

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    static const char NATIVE_ENDIANNESS = ELFDATA2LSB;
#else
    static const char NATIVE_ENDIANNESS = ELFDATA2MSB;
#endif
    if (id[EI_DATA] != NATIVE_ENDIANNESS) {
        fpm_printf("%s: Incompatible endianness\r\n", filename);
        goto err;
    }

#if __SIZE_WIDTH__ == 64
    static const char NATIVE_CLASS = ELFCLASS64;
#else
    static const char NATIVE_CLASS = ELFCLASS32;
#endif
    if (id[EI_CLASS] != NATIVE_CLASS) {
        fpm_printf("%s: Incompatible word size\r\n", filename);
        goto err;
    }

    const Native_Ehdr *hdr = ctx->base;
    if (hdr->e_type != ET_DYN) {
        fpm_printf("%s: Bad exec type\r\n", filename);
        goto err;
    }

#if __x86_64__
    static const unsigned NATIVE_MACHINE = EM_X86_64;
#elif __i386__
    static const unsigned NATIVE_MACHINE = EM_386;
#elif __ARM_ARCH_ISA_A64
    static const unsigned NATIVE_MACHINE = EM_AARCH64;
#elif __ARM_ARCH_ISA_ARM || __ARM_ARCH_6M__ || __ARM_ARCH_8M_MAIN__
    static const unsigned NATIVE_MACHINE = EM_ARM;
#elif __mips__
    static const unsigned NATIVE_MACHINE = EM_MIPS;
#elif __riscv
    static const unsigned NATIVE_MACHINE = EM_RISCV;
#else
#   error "This architecture is not supported"
#endif
    if (hdr->e_machine != NATIVE_MACHINE) {
        fpm_printf("%s: Incompatible machine\r\n", filename);
        goto err;
    }

    // Find relocation section.
    const Native_Shdr *rel_section = fpm_section_by_type(ctx, SHT_RELA);
    if (rel_section == NULL) {
        rel_section = fpm_section_by_type(ctx, SHT_REL);
        if (rel_section == NULL) {
            fpm_printf("%s: No relocation section\r\n", filename);
            goto err;
        }
    }
    ctx->rel_section = rel_section;

    // Number of linked procedures.
    ctx->num_links = rel_section->sh_size / rel_section->sh_entsize;
    return true;
}

//
// Unmap ELF binary from memory.
//
void fpm_unload(fpm_context_t *ctx)
{
    fpm_unload_arch(ctx);
}

//
// Get name from .dynsym section.
//
static const char *fpm_get_name(fpm_context_t *ctx, unsigned reloc_index)
{
    // Get pointer to REL or RELA section.
    const Native_Shdr *rel_section = (const Native_Shdr *) ctx->rel_section;

    // Get index of the symbol in .dynsym section.
    unsigned dynsym_index;
    if (rel_section->sh_type == SHT_RELA) {
        const Native_Rela *relocations = (const Native_Rela *) (rel_section->sh_offset + (char*)ctx->base);
        dynsym_index = NATIVE_R_SYM(relocations[reloc_index].r_info);
    } else {
        const Native_Rel *relocations = (const Native_Rel *) (rel_section->sh_offset + (char*)ctx->base);
        dynsym_index = NATIVE_R_SYM(relocations[reloc_index].r_info);
    }

    // Get pointer to .dynsym contents.
    const Native_Shdr *fpm_section = fpm_section_by_index(ctx, rel_section->sh_link);
    const Native_Sym *symbols      = (const Native_Sym *) (fpm_section->sh_offset + (char*)ctx->base);

    // Get pointer to .dynstr contents.
    const Native_Shdr *str_section = fpm_section_by_index(ctx, fpm_section->sh_link);
    const char *strings            = str_section->sh_offset + (char*)ctx->base;

    // Get name from .dynsym section.
    return &strings[symbols[dynsym_index].st_name];
}

//
// Get names of linked procedures.
// Array result[] must have ctx->num_links entries.
//
void fpm_get_symbols(fpm_context_t *ctx, const char *result[])
{
    for (unsigned index = 0; index < ctx->num_links; index++) {
        result[index] = fpm_get_name(ctx, index);
    }
}

//
// Search linkmap for a given name.
// Return address of the symbol, or NULL on failure.
//
static void *find_address_by_name(const fpm_binding_t *linkmap, const char *name)
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
        linkmap = (const fpm_binding_t *) linkmap[0].address;
        if (linkmap == NULL) {
            return NULL;
        }
    }
}

//
// Setup arch-dependent GOT register.
//
static inline void set_got_pointer(void *addr)
{
#if __ARM_ARCH_6M__
    // For RP2040: use slot #4 of the interrupt vector table,
    // at address 0x2000 0010. This vector is unused by hardware.
    *(volatile void**) 0x20000010 = addr;

#elif (__x86_64__ || __i386__) && __unix__
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
#endif
}

//
// Read arch-dependent GOT register.
//
static inline void *get_got_pointer()
{
    void *addr = NULL;
#if __ARM_ARCH_ISA_A64
    // For arm64 Linux or MacOS: use TPIDR_EL0 register.
    asm volatile("mrs %0, tpidr_el0" : "=r" (addr) : : "memory");
#endif
    return addr;
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
bool fpm_invoke(fpm_context_t *ctx, fpm_binding_t linkmap[], int argc, char *argv[])
{
    // Build a Global Offset Table on stack.
    void **got = alloca(ctx->num_links);

    // Bind dynamic symbols.
    unsigned fail_count = 0;
    for (unsigned index = 0; index < ctx->num_links; index++) {

        // Find symbol's name and address.
        const char *name = fpm_get_name(ctx, index);
        void *address    = find_address_by_name(linkmap, name);

        if (address == NULL) {
            fpm_printf("%s: Symbol not found\r\n", name);
            fail_count++;
        }
        if (fail_count == 0) {
            got[index] = address;
        }
    }
    if (fail_count > 0) {
        // Cannot map some symbols.
        return false;
    }
#if __APPLE__ && __x86_64__
    {
        // This platform is not supported.
        fpm_printf("Cannot set %%gs register on MacOS\r\n");
        return false;
    }
#endif
    void *save_got = get_got_pointer();
    set_got_pointer(got);

    // Compute entry address.
    typedef int (*entry_t)(int, char **);
    const Native_Ehdr *hdr = ctx->base;
    const entry_t entry    = (entry_t) (hdr->e_entry + (char*)ctx->base);

    // Invoke ELF binary.
    ctx->exit_code = (*entry)(argc, argv);

    set_got_pointer(save_got);
    return true;
}
