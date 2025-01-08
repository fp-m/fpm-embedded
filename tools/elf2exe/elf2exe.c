#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "rpm/elf.h"

// Name of the file being processed.
const char *filename;

// Descriptor of opened ELF file.
int file_desc;

// Size of file in bytes.
size_t file_size;

// File is mapped at this address.
void *base;

// Number of linked procedures.
unsigned num_links;

//
// Print usage message.
//
void usage()
{
    printf("Usage:\n");
    printf("    elf2exe [-v] executable.elf\n");
}

//
// Open ELF file and map it to memory.
// Update file_desc, file_size and base.
//
void open_file()
{
    // Open file in read/write mode
    file_desc = open(filename, O_RDWR);
    if (file_desc < 0) {
        fprintf(stderr, "%s: Cannot open\n", filename);
        exit(EXIT_FAILURE);
    }

    // Get the size of the file
    struct stat sb;
    if (fstat(file_desc, &sb) < 0) {
        fprintf(stderr, "%s: Cannot get size\n", filename);
        exit(EXIT_FAILURE);
    }
    file_size = sb.st_size;

    // Map the file into memory
    base = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_desc, 0);
    if (base == MAP_FAILED) {
        fprintf(stderr, "%s: Cannot map to memory\n", filename);
        exit(EXIT_FAILURE);
    }
}

//
// Check file format.
// Return ELF class value.
//
int check_file_format()
{
    const char *id = (const char*) base;
    if (id[EI_MAG0] != ELFMAG0 || id[EI_MAG1] != ELFMAG1 ||
        id[EI_MAG2] != ELFMAG2 || id[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "%s: Not ELF binary\n", filename);
        exit(EXIT_FAILURE);
    }

    if (id[EI_VERSION] != EV_CURRENT) {
        fprintf(stderr, "%s: Incompatible ELF version\n", filename);
        exit(EXIT_FAILURE);
    }

    // Only little endian format is supported for now.
    if (id[EI_DATA] != ELFDATA2LSB) {
        fprintf(stderr, "%s: Incompatible endianness\n", filename);
        exit(EXIT_FAILURE);
    }

    // Note: we check only fields e_type and e_machine here.
    // Location of these fields is the same for Elf32 and Elf64.
    const Elf64_Ehdr *hdr = base;
    if (hdr->e_type != ET_DYN) {
        fprintf(stderr, "%s: Bad exec type\n", filename);
        exit(EXIT_FAILURE);
    }

    switch (id[EI_CLASS]) {
    case ELFCLASS64:
        switch (hdr->e_machine) {
        case EM_X86_64:
            // Intel/AMD 64-bit machine.
            break;
        case EM_AARCH64:
            // ARM64 machine.
            break;
        case EM_MIPS:
        case EM_RISCV:
        default:
            fprintf(stderr, "%s: Unsupported 64-bit machine type\n", filename);
            exit(EXIT_FAILURE);
        }
        break;
    case ELFCLASS32:
        switch (hdr->e_machine) {
        case EM_ARM:
            // ARM32 machine.
            break;
        case EM_386:
            // Intel 32-bit machine.
            break;
        case EM_MIPS:
        case EM_RISCV:
        default:
            fprintf(stderr, "%s: Unsupported 32-bit machine type\n", filename);
            exit(EXIT_FAILURE);
        }
        break;
    default:
        fprintf(stderr, "%s: Incompatible word size\n", filename);
        exit(EXIT_FAILURE);
    }
    return id[EI_CLASS];
}

//
// Unmap ELF binary from memory.
//
void close_file()
{
    munmap(base, file_size);
    close(file_desc);
}

//
// Elf64 format: get name from .shstrtab section.
//
static const char *get_elf64_section_name(unsigned name_offset)
{
    // Get pointer to .shstrtab contents.
    const Elf64_Ehdr *hdr     = base;
    const Elf64_Shdr *section = (const Elf64_Shdr *) (hdr->e_shoff + (char*)base);
    const char *strings       = section[hdr->e_shstrndx].sh_offset + (char*)base;

    return &strings[name_offset];
}

//
// Elf32 format: get name from .shstrtab section.
//
static const char *get_elf32_section_name(unsigned name_offset)
{
    // Get pointer to .shstrtab contents.
    const Elf32_Ehdr *hdr     = base;
    const Elf32_Shdr *section = (const Elf32_Shdr *) (hdr->e_shoff + (char*)base);
    const char *strings       = section[hdr->e_shstrndx].sh_offset + (char*)base;

    return &strings[name_offset];
}

//
// Elf64 format: find section by name.
//
const Elf64_Shdr *find_elf64_section(const char *wanted_name)
{
    const Elf64_Ehdr *hdr     = base;
    const Elf64_Shdr *section = (const Elf64_Shdr *) (hdr->e_shoff + (char*)base);

    for (unsigned i = 0; i < hdr->e_shnum; i++) {
        const char *section_name = get_elf64_section_name(section[i].sh_name);
        if (strcmp(section_name, wanted_name) == 0) {
            return &section[i];
        }
    }
    return NULL;
}

//
// Elf32 format: find section by name.
//
const Elf32_Shdr *find_elf32_section(const char *wanted_name)
{
    const Elf32_Ehdr *hdr     = base;
    const Elf32_Shdr *section = (const Elf32_Shdr *) (hdr->e_shoff + (char*)base);

    for (unsigned i = 0; i < hdr->e_shnum; i++) {
        const char *section_name = get_elf32_section_name(section[i].sh_name);
        if (strcmp(section_name, wanted_name) == 0) {
            return &section[i];
        }
    }
    return NULL;
}

//
// Elf64 format: process the Procedure Linkage Table.
//
void process_elf64_plt(const Elf64_Shdr *hdr)
{
    //TODO
}

//
// Elf32 format: process the Procedure Linkage Table.
//
void process_elf32_plt(const Elf32_Shdr *hdr)
{
    //TODO
}

int main(int argc, char **argv)
{
    int verbose = 0;

    // Parse arguments.
    for (;;) {
        switch (getopt(argc, argv, "v")) {
        case EOF:
            break;
        case 'v':
            ++verbose;
            continue;
        default:
            fprintf(stderr, "Unrecognized option\n");
            usage();
            exit(EXIT_FAILURE);
        }
        break;
    }
    argc -= optind;
    argv += optind;
    if (argc < 1) {
        usage();
        exit(EXIT_SUCCESS);
    }
    if (argc > 1) {
        fprintf(stderr, "Too many arguments\n");
        usage();
        exit(EXIT_FAILURE);
    }

    filename = argv[0];
    open_file();

    switch (check_file_format()) {
        case ELFCLASS64: {
            const Elf64_Shdr *hdr = find_elf64_section(".plt.sec");
            if (hdr == NULL) {
                hdr = find_elf64_section(".plt");
                if (hdr == NULL) {
                    fprintf(stderr, "%s: No procedure linkage table\n", filename);
                    exit(EXIT_FAILURE);
                }
            }
            process_elf64_plt(hdr);
            break;
        }

        case ELFCLASS32: {
            const Elf32_Shdr *hdr = find_elf32_section(".plt");
            if (hdr == NULL) {
                fprintf(stderr, "%s: No procedure linkage table\n", filename);
                exit(EXIT_FAILURE);
            }
            process_elf32_plt(hdr);
            break;
        }
    }
    close_file();
}
