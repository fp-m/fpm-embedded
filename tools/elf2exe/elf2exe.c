#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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

// Class type: 64-bit or 32-bit.
int class_type;

// Machine type: Intel or ARM or others.
int machine_type;

// Verbose mode: flag -v.
int verbose;

//
// Print usage message.
//
void usage()
{
    printf("Modify ELF binary for RP/M operating system\n");
    printf("Usage:\n");
    printf("    elf2exe [-v] executable.elf\n");
}

//
// Open ELF file and map it to memory.
// Set file_desc, file_size and base.
//
void open_file()
{
    // Open file in read/write mode
    if (verbose) {
        printf("Open %s\n", filename);
    }
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
    if (verbose) {
        printf("Size %zu bytes\n", file_size);
    }

    // Map the file into memory
    base = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, file_desc, 0);
    if (base == MAP_FAILED) {
        fprintf(stderr, "%s: Cannot map to memory\n", filename);
        exit(EXIT_FAILURE);
    }
}

const char *machine_name()
{
    switch (machine_type) {
    case EM_X86_64: return "amd64";
    case EM_AARCH64: return "arm64";
    case EM_MIPS: return "mips";
    case EM_RISCV: return "risc-v";
    case EM_ARM: return "arm32";
    case EM_386: return "i386";
    default: return "unknown";
    }
}

//
// Check file format.
// Set class_type and machine_type.
//
void check_file_format()
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

    // Check OSABI field: when Standalone, it means we have already processed this ELF file.
    if (id[EI_OSABI] == (char)ELFOSABI_STANDALONE) {
        fprintf(stderr, "%s: Already processed\n", filename);
        exit(EXIT_SUCCESS);
    }

    class_type = id[EI_CLASS];
    machine_type = hdr->e_machine;
    if (verbose) {
        printf("Class %s, machine %s\n", (class_type == ELFCLASS64) ? "elf64" : "elf32", machine_name());
    }
}

//
// Unmap ELF binary from memory.
//
void close_file()
{
    msync(base, file_size, MS_SYNC);
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
// Elf64 format: parse REL/RELA section and build the symbol map.
//
void parse_elf64_relocation()
{
    //TODO
}

//
// Elf32 format: parse REL/RELA section and build the symbol map.
//
void parse_elf32_relocation()
{
    //TODO
}

//
// Detect PLT entry for x86_64 machine, for example:
//      f3 0f 1e fa        endbr64
//      ff 25 d6 2f 00 00  jmp  *0x2fd6(%rip)
//      66 0f 1f 44 00 00  nopw (%rax,%rax,1)
//
bool is_amd64_entry(const unsigned char *code)
{
    return code[0] == 0xf3 && code[1] == 0x0f && code[2] == 0x1e && code[3] == 0xfa &&   // endbr64
           code[4] == 0xff && code[5] == 0x25 &&                                         // jmp *NUM(%rip)
           code[10] == 0x66 && code[11] == 0x0f && code[12] == 0x1f && code[13] == 0x44; // nopw (%rax,%rax,1)
}

//
// X86_64 machine: process the Procedure Linkage Table.
// Update num_links.
//
void process_amd64_plt(const Elf64_Shdr *hdr)
{
    // Example of .plt section on x86_64 (or amd64) architecture:
    //
    // Disassembly of section .plt:
    // 0000000000001000 <.plt>:
    // 1000: ff 35 ea 2f 00 00  push   0x2fea(%rip)     # 3ff0 <_GLOBAL_OFFSET_TABLE_+0x8>
    // 1006: ff 25 ec 2f 00 00  jmp    *0x2fec(%rip)    # 3ff8 <_GLOBAL_OFFSET_TABLE_+0x10>
    // 100c: 0f 1f 40 00        nopl   0x0(%rax)
    // 1010: f3 0f 1e fa        endbr64
    // 1014: 68 00 00 00 00     push   $0x0
    // 1019: e9 e2 ff ff ff     jmp    1000 <rpm_puts@plt-0x20>
    // 101e: 66 90              xchg   %ax,%ax
    //
    // Disassembly of section .plt.sec:
    // 0000000000001020 <rpm_puts@plt>:
    // 1020: f3 0f 1e fa        endbr64
    // 1024: ff 25 d6 2f 00 00  jmp    *0x2fd6(%rip)    # 4000 <rpm_puts>
    // 102a: 66 0f 1f 44 00 00  nopw   0x0(%rax,%rax,1)

    //
    // Scan contents of the PLT section, find entries and replace
    // with jumps via a table pointed by %gs register.
    // For example:
    //      f3 0f 1e fa              endbr64
    //      65 ff 24 25 08 00 00 00  jmp    *%gs:0x8
    //      0f 1f 04 00              nopl   (%rax,%rax,1)
    //
    unsigned const entry_size = 16;
    for (unsigned offset = 0; offset < hdr->sh_size; offset += entry_size) {
        unsigned char *code = offset + hdr->sh_offset + (unsigned char*)base;
        if (!is_amd64_entry(code)) {
            fprintf(stderr, "Bad PLT entry at offset %u: %02x %02x %02x %02x %02x %02x %02x %02x...\n",
                offset, code[0], code[1], code[2], code[3], code[4], code[5], code[6], code[7]);
            exit(EXIT_FAILURE);
        }

        unsigned index = 0; // TODO

        // jmp *%gs:NUM
        code[4] = 0x65;
        code[5] = 0xff;
        code[6] = 0x24;
        code[7] = 0x25;
        code[8] = (index * 8);
        code[9] = (index * 8) >> 8;
        code[10] = (index * 8) >> 16;
        code[11] = (index * 8) >> 24;

        // nopl (%rax,%rax,1)
        code[12] = 0x0f;
        code[13] = 0x1f;
        code[14] = 0x04;
        code[15] = 0x00;

        num_links++;
    }
}

//
// Detect PLT entry for arm64 machine, for example:
//      90000110  adrp x16, 20000
//      f9400211  ldr  x17, [x16]
//      91000210  add  x16, x16, #0x0
//      d61f0220  br   x17
//
bool is_arm64_entry(const uint32_t *code)
{
    return (code[0] & 0x9f000000) == 0x90000000 && // adrp x16, NUM
           code[1] == 0xf9400211 &&                // ldr x17, [x16]
           (code[2] & 0xff800000) == 0x91000000 && // add x16, x16, #NUM
           code[3] == 0xd61f0220;                  // br x17
}

//
// ARM-64 machine: process the Procedure Linkage Table.
//
void process_arm64_plt(const Elf64_Shdr *hdr)
{
    // Example of .plt section on arm64 architecture:
    //
    // 0000000000000220 <.plt>:
    //  220: a9bf7bf0  stp  x16, x30, [sp, #-16]!
    //  224: f00000f0  adrp x16, 1f000 <main+0x1edb0>
    //  228: f947fe11  ldr  x17, [x16, #4088]
    //  22c: 913fe210  add  x16, x16, #0xff8
    //  230: d61f0220  br   x17
    //  234: d503201f  nop
    //  238: d503201f  nop
    //  23c: d503201f  nop
    //
    // 0000000000000240 <rpm_puts@plt>:
    //  240: 90000110  adrp x16, 20000 <rpm_puts>
    //  244: f9400211  ldr  x17, [x16]
    //  248: 91000210  add  x16, x16, #0x0
    //  24c: d61f0220  br   x17

    //
    // Scan contents of the PLT section, find entries and replace
    // with jumps via a table pointed by TPIDR_EL0 register.
    // For example:
    //      d53bd050  mrs x16, tpidr_el0
    //      91002210  add x16, x16, #0x8
    //      f9400211  ldr x17, [x16]
    //      d61f0220  br  x17
    //
    unsigned const entry_size = 16;
    for (unsigned offset = 32; offset < hdr->sh_size; offset += entry_size) {
        uint32_t *code = (uint32_t *)(offset + hdr->sh_offset + (char*)base);
        if (!is_arm64_entry(code)) {
            fprintf(stderr, "Bad PLT entry at offset %u: %08x %08x %08x %08x\n",
                offset, code[0], code[1], code[2], code[3]);
            exit(EXIT_FAILURE);
        }

        unsigned index = 0; // TODO

        code[0] = 0xd53bd050;                 // mrs x16, tpidr_el0
        code[1] = 0x91000210 | (index << 13); // add x16, x16, #NUM
        code[2] = 0xf9400211;                 // ldr x17, [x16]
        code[3] = 0xd61f0220;                 // br  x17

        num_links++;
    }
}

//
// Detect PLT entry for arm32 machine, for example:
//      e28fc600  add ip, pc, #0, 12
//      e28cca01  add ip, ip, #4096
//      e5bcfe90  ldr pc, [ip, #3728]!
//
bool is_arm32_entry(const uint32_t *code)
{
    return (code[0] & 0xfffff000) == 0xe28fc000 && // add ip, pc, #NUM, 12
           (code[1] & 0xfffff000) == 0xe28cc000 && // add ip, ip, #NUM
           (code[2] & 0xffff0000) == 0xe5bc0000;   // ldr pc, [ip, #NUM]!
}

//
// ARM-32 machine: process the Procedure Linkage Table.
//
void process_arm32_plt(const Elf32_Shdr *hdr)
{
    // Disassembly of section .plt:
    //
    // 00000160 <.plt>:
    //  160: e52de004  push  {lr}             @ (str lr, [sp, #-4]!)
    //  164: e59fe004  ldr   lr, [pc, #4]     @ 170 <.plt+0x10>
    //  168: e08fe00e  add   lr, pc, lr
    //  16c: e5bef008  ldr   pc, [lr, #8]!
    //  170: 00001e90  .word 0x00001e90
    //
    // 00000174 <rpm_puts@plt>:
    //  174: e28fc600  add ip, pc, #0, 12
    //  178: e28cca01  add ip, ip, #4096    @ 0x1000
    //  17c: e5bcfe90  ldr pc, [ip, #3728]! @ 0xe90

    //
    // Scan contents of the PLT section, find entries and replace
    // with jumps via a table pointed by TPIDRURW register.
    // For example:
    //      ee1dcf50  mrc 15, 0, ip, cr13, cr0, 2   -- read TPIDRURW
    //      e59cf001  ldr pc, [ip, #1]
    //      e320f000  nop
    //

    // Skip preamble.
    unsigned offset = 0;
    for (; offset < hdr->sh_size; offset += 4) {
        uint32_t *code = (uint32_t *)(offset + hdr->sh_offset + (char*)base);
        if (is_arm32_entry(code)) {
            break;
        }
    }
    for (; offset < hdr->sh_size; offset += 12) {
        uint32_t *code = (uint32_t *)(offset + hdr->sh_offset + (char*)base);
        if (!is_arm32_entry(code)) {
            fprintf(stderr, "Bad PLT entry at offset %u: %08x %08x %08x\n",
                offset, code[0], code[1], code[2]);
            exit(EXIT_FAILURE);
        }

        unsigned index = 0; // TODO

        code[0] = 0xee1dcf50;         // mrc p15, 0, ip, c13, c0, 2
        code[1] = 0xe59cf000 | index; // ldr pc, [ip, #NUM]
        code[2] = 0xe320f000;         // nop

        num_links++;
    }
}

//
// RISC-V 64-bit machine: process the Procedure Linkage Table.
//
void process_riscv64_plt(const Elf64_Shdr *hdr)
{
    //TODO
    fprintf(stderr, "RISC-V machine is not supported yet\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
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
    check_file_format();

    if (class_type == ELFCLASS64) {
        //
        // 64-bit architecture.
        //
        const Elf64_Shdr *plt = find_elf64_section(".plt.sec");
        if (plt == NULL) {
            plt = find_elf64_section(".plt");
            if (plt == NULL) {
                fprintf(stderr, "%s: No procedure linkage table\n", filename);
                exit(EXIT_FAILURE);
            }
        }
        parse_elf64_relocation();
        switch (machine_type) {
        case EM_X86_64:
            process_amd64_plt(plt);
            break;
        case EM_AARCH64:
            process_arm64_plt(plt);
            break;
        case EM_RISCV:
            process_riscv64_plt(plt);
            break;
        default:
            fprintf(stderr, "%s: Unsupported 64-bit machine: %s\n", filename, machine_name());
            exit(EXIT_FAILURE);
        }
    } else {
        //
        // 32-bit architecture.
        //
        const Elf32_Shdr *plt = find_elf32_section(".plt");
        if (plt == NULL) {
            fprintf(stderr, "%s: No procedure linkage table\n", filename);
            exit(EXIT_FAILURE);
        }
        parse_elf32_relocation();
        switch (machine_type) {
        case EM_ARM:
            process_arm32_plt(plt);
            break;
        default:
            fprintf(stderr, "%s: Unsupported 32-bit machine: %s\n", filename, machine_name());
            exit(EXIT_FAILURE);
        }
    }

    if (num_links > 0) {
        printf("%s: %u linked procedures\n", filename, num_links);

        // Mark this file as ready for RP/M.
        char *id = base;
        id[EI_OSABI] = ELFOSABI_STANDALONE;
    }
    close_file();
}
