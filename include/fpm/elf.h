/*-
 * Copyright (c) 2010,2021 Joseph Koshy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * These definitions are based on:
 * - The public specification of the ELF format as defined in the
 *   October 2009 draft of System V ABI.
 *   See: http://www.sco.com/developers/gabi/latest/ch4.intro.html
 * - The May 1998 (version 1.5) draft of "The ELF-64 object format".
 * - Processor-specific ELF ABI definitions for sparc, i386, amd64, mips,
 *   ia64, powerpc, and RISC-V processors.
 * - The "Linkers and Libraries Guide", from Sun Microsystems.
 */

#ifndef FPM_ELF_H
#define FPM_ELF_H

#include <stdint.h>

/*
 * Offsets in the ei_ident[] field of an ELF executable header.
 */

#define EI_MAG0       0
#define EI_MAG1       1
#define EI_MAG2       2
#define EI_MAG3       3
#define EI_CLASS      4
#define EI_DATA       5
#define EI_VERSION    6
#define EI_OSABI      7
#define EI_ABIVERSION 8
#define EI_PAD        9
#define EI_NIDENT     16

/*
 * The ELF class of an object.
 */

#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

/*
 * Endianness of data in an ELF object.
 */

#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

/*
 * The magic numbers used in the initial four bytes of an ELF object.
 *
 * These numbers are: 0x7F, 'E', 'L' and 'F'.
 */

#define ELFMAG0 0x7FU
#define ELFMAG1 0x45U
#define ELFMAG2 0x4CU
#define ELFMAG3 0x46U

/* Additional magic-related constants. */

#define ELFMAG  "\177ELF"
#define SELFMAG 4

/*
 * ELF OS ABI field.
 */

#define ELFOSABI_NONE       0
#define ELFOSABI_SYSV       0
#define ELFOSABI_HPUX       1
#define ELFOSABI_NETBSD     2
#define ELFOSABI_GNU        3
#define ELFOSABI_HURD       4
#define ELFOSABI_86OPEN     5
#define ELFOSABI_SOLARIS    6
#define ELFOSABI_AIX        7
#define ELFOSABI_IRIX       8
#define ELFOSABI_FREEBSD    9
#define ELFOSABI_TRU64      10
#define ELFOSABI_MODESTO    11
#define ELFOSABI_OPENBSD    12
#define ELFOSABI_OPENVMS    13
#define ELFOSABI_NSK        14
#define ELFOSABI_AROS       15
#define ELFOSABI_FENIXOS    16
#define ELFOSABI_CLOUDABI   17
#define ELFOSABI_OPENVOS    18
#define ELFOSABI_ARM_AEABI  64
#define ELFOSABI_ARM        97
#define ELFOSABI_STANDALONE 255

/* OS ABI Aliases. */

#define ELFOSABI_LINUX ELFOSABI_GNU

/*
 * ELF Machine types: (EM_*).
 */

#define EM_NONE        0  /* No machine */
#define EM_M32         1  /* AT&T WE 32100 */
#define EM_SPARC       2  /* SPARC */
#define EM_386         3  /* Intel 80386 */
#define EM_68K         4  /* Motorola 68000 */
#define EM_88K         5  /* Motorola 88000 */
#define EM_IAMCU       6  /* Intel MCU */
#define EM_860         7  /* Intel 80860 */
#define EM_MIPS        8  /* MIPS I Architecture */
#define EM_S370        9  /* IBM System/370 Processor */
#define EM_MIPS_RS3_LE 10 /* MIPS RS3000 Little-endian */
                          /* 11-14 Reserved for future use */
#define EM_PARISC 15      /* Hewlett-Packard PA-RISC */
                          /* 16 Reserved for future use */
#define EM_VPP500      17 /* Fujitsu VPP500 */
#define EM_SPARC32PLUS 18 /* Enhanced instruction set SPARC */
#define EM_960         19 /* Intel 80960 */
#define EM_PPC         20 /* PowerPC */
#define EM_PPC64       21 /* 64-bit PowerPC */
#define EM_S390        22 /* IBM System/390 Processor */
#define EM_SPU         23 /* IBM SPU/SPC */
                          /* 24-35 Reserved for future use */
#define EM_V800     36    /* NEC V800 */
#define EM_FR20     37    /* Fujitsu FR20 */
#define EM_RH32     38    /* TRW RH-32 */
#define EM_RCE      39    /* Motorola RCE */
#define EM_ARM      40    /* ARM 32-bit architecture (AARCH32) */
#define EM_ALPHA    41    /* Digital Alpha */
#define EM_SH       42    /* Hitachi SH */
#define EM_SPARCV9  43    /* SPARC Version 9 */
#define EM_TRICORE  44    /* Siemens TriCore embedded processor */
#define EM_ARC      45    /* Argonaut RISC Core, Argonaut Technologies Inc. */
#define EM_H8_300   46    /* Hitachi H8/300 */
#define EM_H8_300H  47    /* Hitachi H8/300H */
#define EM_H8S      48    /* Hitachi H8S */
#define EM_H8_500   49    /* Hitachi H8/500 */
#define EM_IA_64    50    /* Intel IA-64 processor architecture */
#define EM_MIPS_X   51    /* Stanford MIPS-X */
#define EM_COLDFIRE 52    /* Motorola ColdFire */
#define EM_68HC12   53    /* Motorola M68HC12 */
#define EM_MMA      54    /* Fujitsu MMA Multimedia Accelerator */
#define EM_PCP      55    /* Siemens PCP */
#define EM_NCPU     56    /* Sony nCPU embedded RISC processor */
#define EM_NDR1     57    /* Denso NDR1 microprocessor */
#define EM_STARCORE 58    /* Motorola Star*Core processor */
#define EM_ME16     59    /* Toyota ME16 processor */
#define EM_ST100    60    /* STMicroelectronics ST100 processor */
#define EM_TINYJ    61    /* Advanced Logic Corp. TinyJ embedded processor family */
#define EM_X86_64   62    /* AMD x86-64 architecture */
#define EM_PDSP     63    /* Sony DSP Processor */
#define EM_PDP10    64    /* Digital Equipment Corp. PDP-10 */
#define EM_PDP11    65    /* Digital Equipment Corp. PDP-11 */
#define EM_FX66     66    /* Siemens FX66 microcontroller */
#define EM_ST9PLUS  67    /* STMicroelectronics ST9+ 8/16 bit microcontroller */
#define EM_ST7      68    /* STMicroelectronics ST7 8-bit microcontroller */
#define EM_68HC16   69    /* Motorola MC68HC16 Microcontroller */
#define EM_68HC11   70    /* Motorola MC68HC11 Microcontroller */
#define EM_68HC08   71    /* Motorola MC68HC08 Microcontroller */
#define EM_68HC05   72    /* Motorola MC68HC05 Microcontroller */
#define EM_SVX      73    /* Silicon Graphics SVx */
#define EM_ST19     74    /* STMicroelectronics ST19 8-bit microcontroller */
#define EM_VAX      75    /* Digital VAX */
#define EM_CRIS     76    /* Axis Communications 32-bit embedded processor */
#define EM_JAVELIN  77    /* Infineon Technologies 32-bit embedded processor */
#define EM_FIREPATH 78    /* Element 14 64-bit DSP Processor */
#define EM_ZSP      79    /* LSI Logic 16-bit DSP Processor */
#define EM_MMIX     80    /* Donald Knuth's educational 64-bit processor */
#define EM_HUANY    81    /* Harvard University machine-independent object files */
#define EM_PRISM    82    /* SiTera Prism */
#define EM_AVR      83    /* Atmel AVR 8-bit microcontroller */
#define EM_FR30     84    /* Fujitsu FR30 */
#define EM_D10V     85    /* Mitsubishi D10V */
#define EM_D30V     86    /* Mitsubishi D30V */
#define EM_V850     87    /* NEC v850 */
#define EM_M32R     88    /* Mitsubishi M32R */
#define EM_MN10300  89    /* Matsushita MN10300 */
#define EM_MN10200  90    /* Matsushita MN10200 */
#define EM_PJ       91    /* picoJava */
#define EM_OPENRISC 92    /* OpenRISC 32-bit embedded processor */
#define EM_ARC_COMPACT \
    93 /* ARC International ARCompact processor (old spelling/synonym: EM_ARC_A5) */
#define EM_XTENSA    94  /* Tensilica Xtensa Architecture */
#define EM_VIDEOCORE 95  /* Alphamosaic VideoCore processor */
#define EM_TMM_GPP   96  /* Thompson Multimedia General Purpose Processor */
#define EM_NS32K     97  /* National Semiconductor 32000 series */
#define EM_TPC       98  /* Tenor Network TPC processor */
#define EM_SNP1K     99  /* Trebia SNP 1000 processor */
#define EM_ST200     100 /* STMicroelectronics (www.st.com) ST200 microcontroller */
#define EM_IP2K      101 /* Ubicom IP2xxx microcontroller family */
#define EM_MAX       102 /* MAX Processor */
#define EM_CR        103 /* National Semiconductor CompactRISC microprocessor */
#define EM_F2MC16    104 /* Fujitsu F2MC16 */
#define EM_MSP430    105 /* Texas Instruments embedded microcontroller msp430 */
#define EM_BLACKFIN  106 /* Analog Devices Blackfin (DSP) processor */
#define EM_SE_C33    107 /* S1C33 Family of Seiko Epson processors */
#define EM_SEP       108 /* Sharp embedded microprocessor */
#define EM_ARCA      109 /* Arca RISC Microprocessor */
#define EM_UNICORE   110 /* Microprocessor series from PKU-Unity Ltd. and MPRC of Peking University \
                          */
#define EM_EXCESS       111  /* eXcess: 16/32/64-bit configurable embedded CPU */
#define EM_DXP          112  /* Icera Semiconductor Inc. Deep Execution Processor */
#define EM_ALTERA_NIOS2 113  /* Altera Nios II soft-core processor */
#define EM_CRX          114  /* National Semiconductor CompactRISC CRX microprocessor */
#define EM_XGATE        115  /* Motorola XGATE embedded processor */
#define EM_C166         116  /* Infineon C16x/XC16x processor */
#define EM_M16C         117  /* Renesas M16C series microprocessors */
#define EM_DSPIC30F     118  /* Microchip Technology dsPIC30F Digital Signal Controller */
#define EM_CE           119  /* Freescale Communication Engine RISC core */
#define EM_M32C         120  /* Renesas M32C series microprocessors */
                             /* 121-130 Reserved for future use */
#define EM_TSK3000       131 /* Altium TSK3000 core */
#define EM_RS08          132 /* Freescale RS08 embedded processor */
#define EM_SHARC         133 /* Analog Devices SHARC family of 32-bit DSP processors */
#define EM_ECOG2         134 /* Cyan Technology eCOG2 microprocessor */
#define EM_SCORE7        135 /* Sunplus S+core7 RISC processor */
#define EM_DSP24         136 /* New Japan Radio (NJR) 24-bit DSP Processor */
#define EM_VIDEOCORE3    137 /* Broadcom VideoCore III processor */
#define EM_LATTICEMICO32 138 /* RISC processor for Lattice FPGA architecture */
#define EM_SE_C17        139 /* Seiko Epson C17 family */
#define EM_TI_C6000      140 /* The Texas Instruments TMS320C6000 DSP family */
#define EM_TI_C2000      141 /* The Texas Instruments TMS320C2000 DSP family */
#define EM_TI_C5500      142 /* The Texas Instruments TMS320C55x DSP family */
#define EM_TI_ARP32      143 /* Texas Instruments Application Specific RISC Processor, 32bit fetch */
#define EM_TI_PRU        144 /* Texas Instruments Programmable Realtime Unit */
                             /* 145-159 Reserved for future use */
#define EM_MMDSP_PLUS  160   /* STMicroelectronics 64bit VLIW Data Signal Processor */
#define EM_CYPRESS_M8C 161   /* Cypress M8C microprocessor */
#define EM_R32C        162   /* Renesas R32C series microprocessors */
#define EM_TRIMEDIA    163   /* NXP Semiconductors TriMedia architecture family */
#define EM_QDSP6       164   /* QUALCOMM DSP6 Processor */
#define EM_8051        165   /* Intel 8051 and variants */
#define EM_STXP7X \
    166 /* STMicroelectronics STxP7x family of configurable and extensible RISC processors */
#define EM_NDS32       167  /* Andes Technology compact code size embedded RISC processor family */
#define EM_ECOG1       168  /* Cyan Technology eCOG1X family */
#define EM_ECOG1X      168  /* Cyan Technology eCOG1X family */
#define EM_MAXQ30      169  /* Dallas Semiconductor MAXQ30 Core Micro-controllers */
#define EM_XIMO16      170  /* New Japan Radio (NJR) 16-bit DSP Processor */
#define EM_MANIK       171  /* M2000 Reconfigurable RISC Microprocessor */
#define EM_CRAYNV2     172  /* Cray Inc. NV2 vector architecture */
#define EM_RX          173  /* Renesas RX family */
#define EM_METAG       174  /* Imagination Technologies META processor architecture */
#define EM_MCST_ELBRUS 175  /* MCST Elbrus general purpose hardware architecture */
#define EM_ECOG16      176  /* Cyan Technology eCOG16 family */
#define EM_CR16        177  /* National Semiconductor CompactRISC CR16 16-bit microprocessor */
#define EM_ETPU        178  /* Freescale Extended Time Processing Unit */
#define EM_SLE9X       179  /* Infineon Technologies SLE9X core */
#define EM_L10M        180  /* Intel L10M */
#define EM_K10M        181  /* Intel K10M */
                            /* 182 Reserved for future Intel use */
#define EM_AARCH64 183      /* ARM 64-bit architecture (AARCH64) */
                            /* 184 Reserved for future ARM use */
#define EM_AVR32        185 /* Atmel Corporation 32-bit microprocessor family */
#define EM_STM8         186 /* STMicroeletronics STM8 8-bit microcontroller */
#define EM_TILE64       187 /* Tilera TILE64 multicore architecture family */
#define EM_TILEPRO      188 /* Tilera TILEPro multicore architecture family */
#define EM_MICROBLAZE   189 /* Xilinx MicroBlaze 32-bit RISC soft processor core */
#define EM_CUDA         190 /* NVIDIA CUDA architecture */
#define EM_TILEGX       191 /* Tilera TILE-Gx multicore architecture family */
#define EM_CLOUDSHIELD  192 /* CloudShield architecture family */
#define EM_COREA_1ST    193 /* KIPO-KAIST Core-A 1st generation processor family */
#define EM_COREA_2ND    194 /* KIPO-KAIST Core-A 2nd generation processor family */
#define EM_ARC_COMPACT2 195 /* Synopsys ARCompact V2 */
#define EM_OPEN8        196 /* Open8 8-bit RISC soft processor core */
#define EM_RL78         197 /* Renesas RL78 family */
#define EM_VIDEOCORE5   198 /* Broadcom VideoCore V processor */
#define EM_78KOR        199 /* Renesas 78KOR family */
#define EM_56800EX      200 /* Freescale 56800EX Digital Signal Controller (DSC) */
#define EM_BA1          201 /* Beyond BA1 CPU architecture */
#define EM_BA2          202 /* Beyond BA2 CPU architecture */
#define EM_XCORE        203 /* XMOS xCORE processor family */
#define EM_MCHP_PIC     204 /* Microchip 8-bit PIC(r) family */
#define EM_INTEL205     205 /* Reserved by Intel */
#define EM_INTEL206     206 /* Reserved by Intel */
#define EM_INTEL207     207 /* Reserved by Intel */
#define EM_INTEL208     208 /* Reserved by Intel */
#define EM_INTEL209     209 /* Reserved by Intel */
#define EM_KM32         210 /* KM211 KM32 32-bit processor */
#define EM_KMX32        211 /* KM211 KMX32 32-bit processor */
#define EM_KMX16        212 /* KM211 KMX16 16-bit processor */
#define EM_KMX8         213 /* KM211 KMX8 8-bit processor */
#define EM_KVARC        214 /* KM211 KVARC processor */
#define EM_CDP          215 /* Paneve CDP architecture family */
#define EM_COGE         216 /* Cognitive Smart Memory Processor */
#define EM_COOL         217 /* Bluechip Systems CoolEngine */
#define EM_NORC         218 /* Nanoradio Optimized RISC */
#define EM_CSR_KALIMBA  219 /* CSR Kalimba architecture family */
#define EM_Z80          220 /* Zilog Z80 */
#define EM_VISIUM       221 /* Controls and Data Services VISIUMcore processor */
#define EM_FT32         222 /* FTDI Chip FT32 high performance 32-bit RISC architecture */
#define EM_MOXIE        223 /* Moxie processor family */
#define EM_AMDGPU       224 /* AMD GPU architecture */
                            /* 225-242 Reserved for future use */
#define EM_RISCV           243 /* RISC-V */
#define EM_LANAI           244 /* Lanai processor */
#define EM_CEVA            245 /* CEVA Processor Architecture Family */
#define EM_CEVA_X2         246 /* CEVA X2 Processor Family */
#define EM_BPF             247 /* Linux BPF – in-kernel virtual machine */
#define EM_GRAPHCORE_IPU   248 /* Graphcore Intelligent Processing Unit */
#define EM_IMG1            249 /* Imagination Technologies */
#define EM_NFP             250 /* Netronome Flow Processor (NFP) */
#define EM_VE              251 /* NEC Vector Engine */
#define EM_CSKY            252 /* C-SKY processor family */
#define EM_ARC_COMPACT3_64 253 /* Synopsys ARCv2.3 64-bit */
#define EM_MCS6502         254 /* MOS Technology MCS 6502 processor */
#define EM_ARC_COMPACT3    255 /* Synopsys ARCv2.3 32-bit */
#define EM_KVX             256 /* Kalray VLIW core of the MPPA processor family */
#define EM_65816           257 /* WDC 65816/65C816 */
#define EM_LOONGARCH       258 /* Loongson Loongarch */
#define EM_KF32            259 /* ChipON KungFu32 */
#define EM_U16_U8CORE      260 /* LAPIS nX-U16/U8 */
#define EM_TACHYUM         261 /* Reserved for Tachyum processor */
#define EM_56800EF         262 /* NXP 56800EF Digital Signal Controller (DSC) */
#define EM_SBF             263 /* Solana Bytecode Format */
#define EM_AIENGINE        264 /* AMD/Xilinx AIEngine architecture */
#define EM_SIMA_MLA        265 /* SiMa MLA */
#define EM_BANG            266 /* Cambricon BANG */
#define EM_LOONGGPU        267 /* Loongson Loongarch */

/* Other synonyms. */

#define EM_AMD64  EM_X86_64
#define EM_ARC_A5 EM_ARC_COMPACT

/*
 * ELF file types: (ET_*).
 */

#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4
#define ET_LOOS   0xFE00U
#define ET_HIOS   0xFEFFU
#define ET_LOPROC 0xFF00U
#define ET_HIPROC 0xFFFFU

/* ELF file format version numbers. */

#define EV_NONE    0
#define EV_CURRENT 1


/**
 ** ELF Types.
 **/

typedef uint32_t Elf32_Addr;    /* Program address. */
typedef uint8_t Elf32_Byte;     /* Unsigned tiny integer. */
typedef uint16_t Elf32_Half;    /* Unsigned medium integer. */
typedef uint32_t Elf32_Off;     /* File offset. */
typedef uint16_t Elf32_Section; /* Section index. */
typedef int32_t Elf32_Sword;    /* Signed integer. */
typedef uint32_t Elf32_Word;    /* Unsigned integer. */
typedef uint64_t Elf32_Lword;   /* Unsigned long integer. */

typedef uint64_t Elf64_Addr;    /* Program address. */
typedef uint8_t Elf64_Byte;     /* Unsigned tiny integer. */
typedef uint16_t Elf64_Half;    /* Unsigned medium integer. */
typedef uint64_t Elf64_Off;     /* File offset. */
typedef uint16_t Elf64_Section; /* Section index. */
typedef int32_t Elf64_Sword;    /* Signed integer. */
typedef uint32_t Elf64_Word;    /* Unsigned integer. */
typedef uint64_t Elf64_Lword;   /* Unsigned long integer. */
typedef uint64_t Elf64_Xword;   /* Unsigned long integer. */
typedef int64_t Elf64_Sxword;   /* Signed long integer. */

/*
 * Capability descriptors.
 */

/* 32-bit capability descriptor. */
typedef struct {
    Elf32_Word c_tag; /* Type of entry. */
    union {
        Elf32_Word c_val; /* Integer value. */
        Elf32_Addr c_ptr; /* Pointer value. */
    } c_un;
} Elf32_Cap;

/* 64-bit capability descriptor. */
typedef struct {
    Elf64_Xword c_tag; /* Type of entry. */
    union {
        Elf64_Xword c_val; /* Integer value. */
        Elf64_Addr c_ptr;  /* Pointer value. */
    } c_un;
} Elf64_Cap;

/*
 * MIPS .conflict section entries.
 */

/* 32-bit entry. */
typedef struct {
    Elf32_Addr c_index;
} Elf32_Conflict;

/* 64-bit entry. */
typedef struct {
    Elf64_Addr c_index;
} Elf64_Conflict;

/*
 * Dynamic section entries.
 */

/* 32-bit entry. */
typedef struct {
    Elf32_Sword d_tag; /* Type of entry. */
    union {
        Elf32_Word d_val; /* Integer value. */
        Elf32_Addr d_ptr; /* Pointer value. */
    } d_un;
} Elf32_Dyn;

/* 64-bit entry. */
typedef struct {
    Elf64_Sxword d_tag; /* Type of entry. */
    union {
        Elf64_Xword d_val; /* Integer value. */
        Elf64_Addr d_ptr;  /* Pointer value; */
    } d_un;
} Elf64_Dyn;

/*
 * The executable header (EHDR).
 */
#define EI_NIDENT 16

/* 32 bit EHDR. */
typedef struct {
    unsigned char e_ident[EI_NIDENT]; /* ELF identification. */
    Elf32_Half e_type;                /* Object file type (ET_*). */
    Elf32_Half e_machine;             /* Machine type (EM_*). */
    Elf32_Word e_version;             /* File format version (EV_*). */
    Elf32_Addr e_entry;               /* Start address. */
    Elf32_Off e_phoff;                /* File offset to the PHDR table. */
    Elf32_Off e_shoff;                /* File offset to the SHDRheader. */
    Elf32_Word e_flags;               /* Flags (EF_*). */
    Elf32_Half e_ehsize;              /* Elf header size in bytes. */
    Elf32_Half e_phentsize;           /* PHDR table entry size in bytes. */
    Elf32_Half e_phnum;               /* Number of PHDR entries. */
    Elf32_Half e_shentsize;           /* SHDR table entry size in bytes. */
    Elf32_Half e_shnum;               /* Number of SHDR entries. */
    Elf32_Half e_shstrndx;            /* Index of section name string table. */
} Elf32_Ehdr;

/* 64 bit EHDR. */
typedef struct {
    unsigned char e_ident[EI_NIDENT]; /* ELF identification. */
    Elf64_Half e_type;                /* Object file type (ET_*). */
    Elf64_Half e_machine;             /* Machine type (EM_*). */
    Elf64_Word e_version;             /* File format version (EV_*). */
    Elf64_Addr e_entry;               /* Start address. */
    Elf64_Off e_phoff;                /* File offset to the PHDR table. */
    Elf64_Off e_shoff;                /* File offset to the SHDRheader. */
    Elf64_Word e_flags;               /* Flags (EF_*). */
    Elf64_Half e_ehsize;              /* Elf header size in bytes. */
    Elf64_Half e_phentsize;           /* PHDR table entry size in bytes. */
    Elf64_Half e_phnum;               /* Number of PHDR entries. */
    Elf64_Half e_shentsize;           /* SHDR table entry size in bytes. */
    Elf64_Half e_shnum;               /* Number of SHDR entries. */
    Elf64_Half e_shstrndx;            /* Index of section name string table. */
} Elf64_Ehdr;

/*
 * Shared object information.
 */

/* 32-bit entry. */
typedef struct {
    Elf32_Word l_name;       /* The name of a shared object. */
    Elf32_Word l_time_stamp; /* 32-bit timestamp. */
    Elf32_Word l_checksum;   /* Checksum of visible symbols, sizes. */
    Elf32_Word l_version;    /* Interface version string index. */
    Elf32_Word l_flags;      /* Flags (LL_*). */
} Elf32_Lib;

/* 64-bit entry. */
typedef struct {
    Elf64_Word l_name;       /* The name of a shared object. */
    Elf64_Word l_time_stamp; /* 32-bit timestamp. */
    Elf64_Word l_checksum;   /* Checksum of visible symbols, sizes. */
    Elf64_Word l_version;    /* Interface version string index. */
    Elf64_Word l_flags;      /* Flags (LL_*). */
} Elf64_Lib;

/*
 * Note descriptors.
 */

typedef struct {
    uint32_t n_namesz; /* Length of note's name. */
    uint32_t n_descsz; /* Length of note's value. */
    uint32_t n_type;   /* Type of note. */
} Elf_Note;

typedef Elf_Note Elf32_Nhdr; /* 32-bit note header. */
typedef Elf_Note Elf64_Nhdr; /* 64-bit note header. */

/*
 * MIPS ELF options descriptor header.
 */

typedef struct {
    Elf64_Byte kind;    /* Type of options. */
    Elf64_Byte size;    /* Size of option descriptor. */
    Elf64_Half section; /* Index of section affected. */
    Elf64_Word info;    /* Kind-specific information. */
} Elf_Options;

/*
 * MIPS ELF register info descriptor.
 */

/* 32 bit RegInfo entry. */
typedef struct {
    Elf32_Word ri_gprmask;    /* Mask of general register used. */
    Elf32_Word ri_cprmask[4]; /* Mask of coprocessor register used. */
    Elf32_Addr ri_gp_value;   /* GP register value. */
} Elf32_RegInfo;

/* 64 bit RegInfo entry. */
typedef struct {
    Elf64_Word ri_gprmask;    /* Mask of general register used. */
    Elf64_Word ri_pad;        /* Padding. */
    Elf64_Word ri_cprmask[4]; /* Mask of coprocessor register used. */
    Elf64_Addr ri_gp_value;   /* GP register value. */
} Elf64_RegInfo;

/*
 * Program Header Table (PHDR) entries.
 */

/* 32 bit PHDR entry. */
typedef struct {
    Elf32_Word p_type;   /* Type of segment. */
    Elf32_Off p_offset;  /* File offset to segment. */
    Elf32_Addr p_vaddr;  /* Virtual address in memory. */
    Elf32_Addr p_paddr;  /* Physical address (if relevant). */
    Elf32_Word p_filesz; /* Size of segment in file. */
    Elf32_Word p_memsz;  /* Size of segment in memory. */
    Elf32_Word p_flags;  /* Segment flags. */
    Elf32_Word p_align;  /* Alignment constraints. */
} Elf32_Phdr;

/* 64 bit PHDR entry. */
typedef struct {
    Elf64_Word p_type;    /* Type of segment. */
    Elf64_Word p_flags;   /* Segment flags. */
    Elf64_Off p_offset;   /* File offset to segment. */
    Elf64_Addr p_vaddr;   /* Virtual address in memory. */
    Elf64_Addr p_paddr;   /* Physical address (if relevant). */
    Elf64_Xword p_filesz; /* Size of segment in file. */
    Elf64_Xword p_memsz;  /* Size of segment in memory. */
    Elf64_Xword p_align;  /* Alignment constraints. */
} Elf64_Phdr;

/*
 * Move entries, for describing data in COMMON blocks in a compact
 * manner.
 */

/* 32-bit move entry. */
typedef struct {
    Elf32_Lword m_value;  /* Initialization value. */
    Elf32_Word m_info;    /* Encoded size and index. */
    Elf32_Word m_poffset; /* Offset relative to symbol. */
    Elf32_Half m_repeat;  /* Repeat count. */
    Elf32_Half m_stride;  /* Number of units to skip. */
} Elf32_Move;

/* 64-bit move entry. */
typedef struct {
    Elf64_Lword m_value;   /* Initialization value. */
    Elf64_Xword m_info;    /* Encoded size and index. */
    Elf64_Xword m_poffset; /* Offset relative to symbol. */
    Elf64_Half m_repeat;   /* Repeat count. */
    Elf64_Half m_stride;   /* Number of units to skip. */
} Elf64_Move;

#define ELF32_M_SYM(I) ((I) >> 8)
#define ELF32_M_SIZE(I) ((unsigned char)(I))
#define ELF32_M_INFO(M, S) (((M) << 8) + (unsigned char)(S))

#define ELF64_M_SYM(I) ((I) >> 8)
#define ELF64_M_SIZE(I) ((unsigned char)(I))
#define ELF64_M_INFO(M, S) (((M) << 8) + (unsigned char)(S))

/*
 * Section Header Table (SHDR) entries.
 */

/* 32 bit SHDR */
typedef struct {
    Elf32_Word sh_name;      /* index of section name */
    Elf32_Word sh_type;      /* section type */
    Elf32_Word sh_flags;     /* section flags */
    Elf32_Addr sh_addr;      /* in-memory address of section */
    Elf32_Off sh_offset;     /* file offset of section */
    Elf32_Word sh_size;      /* section size in bytes */
    Elf32_Word sh_link;      /* section header table link */
    Elf32_Word sh_info;      /* extra information */
    Elf32_Word sh_addralign; /* alignment constraint */
    Elf32_Word sh_entsize;   /* size for fixed-size entries */
} Elf32_Shdr;

/* 64 bit SHDR */
typedef struct {
    Elf64_Word sh_name;       /* index of section name */
    Elf64_Word sh_type;       /* section type */
    Elf64_Xword sh_flags;     /* section flags */
    Elf64_Addr sh_addr;       /* in-memory address of section */
    Elf64_Off sh_offset;      /* file offset of section */
    Elf64_Xword sh_size;      /* section size in bytes */
    Elf64_Word sh_link;       /* section header table link */
    Elf64_Word sh_info;       /* extra information */
    Elf64_Xword sh_addralign; /* alignment constraint */
    Elf64_Xword sh_entsize;   /* size for fixed-size entries */
} Elf64_Shdr;

/*
 * Symbol table entries.
 */

typedef struct {
    Elf32_Word st_name;     /* index of symbol's name */
    Elf32_Addr st_value;    /* value for the symbol */
    Elf32_Word st_size;     /* size of associated data */
    unsigned char st_info;  /* type and binding attributes */
    unsigned char st_other; /* visibility */
    Elf32_Half st_shndx;    /* index of related section */
} Elf32_Sym;

typedef struct {
    Elf64_Word st_name;     /* index of symbol's name */
    unsigned char st_info;  /* type and binding attributes */
    unsigned char st_other; /* visibility */
    Elf64_Half st_shndx;    /* index of related section */
    Elf64_Addr st_value;    /* value for the symbol */
    Elf64_Xword st_size;    /* size of associated data */
} Elf64_Sym;

#define ELF32_ST_BIND(I) ((I) >> 4)
#define ELF32_ST_TYPE(I) ((I) & 0xFU)
#define ELF32_ST_INFO(B, T) (((B) << 4) + ((T) & 0xF))

#define ELF64_ST_BIND(I) ((I) >> 4)
#define ELF64_ST_TYPE(I) ((I) & 0xFU)
#define ELF64_ST_INFO(B, T) (((B) << 4) + ((T) & 0xF))

#define ELF32_ST_VISIBILITY(O) ((O) & 0x3)
#define ELF64_ST_VISIBILITY(O) ((O) & 0x3)

/*
 * Syminfo descriptors, containing additional symbol information.
 */

/* 32-bit entry. */
typedef struct {
    Elf32_Half si_boundto; /* Entry index with additional flags. */
    Elf32_Half si_flags;   /* Flags. */
} Elf32_Syminfo;

/* 64-bit entry. */
typedef struct {
    Elf64_Half si_boundto; /* Entry index with additional flags. */
    Elf64_Half si_flags;   /* Flags. */
} Elf64_Syminfo;

/*
 * Relocation descriptors.
 */

typedef struct {
    Elf32_Addr r_offset; /* location to apply relocation to */
    Elf32_Word r_info;   /* type+section for relocation */
} Elf32_Rel;

typedef struct {
    Elf32_Addr r_offset;  /* location to apply relocation to */
    Elf32_Word r_info;    /* type+section for relocation */
    Elf32_Sword r_addend; /* constant addend */
} Elf32_Rela;

typedef struct {
    Elf64_Addr r_offset; /* location to apply relocation to */
    Elf64_Xword r_info;  /* type+section for relocation */
} Elf64_Rel;

typedef struct {
    Elf64_Addr r_offset;   /* location to apply relocation to */
    Elf64_Xword r_info;    /* type+section for relocation */
    Elf64_Sxword r_addend; /* constant addend */
} Elf64_Rela;

#define ELF32_R_SYM(I) ((I) >> 8)
#define ELF32_R_TYPE(I) ((unsigned char)(I))
#define ELF32_R_INFO(S, T) (((S) << 8) + (unsigned char)(T))

#define ELF64_R_SYM(I) ((I) >> 32)
#define ELF64_R_TYPE(I) ((I) & 0xFFFFFFFFUL)
#define ELF64_R_INFO(S, T) (((Elf64_Xword)(S) << 32) + ((T) & 0xFFFFFFFFUL))

/*
 * Symbol versioning structures.
 */

/* 32-bit structures. */
typedef struct {
    Elf32_Word vda_name; /* Index to name. */
    Elf32_Word vda_next; /* Offset to next entry. */
} Elf32_Verdaux;

typedef struct {
    Elf32_Word vna_hash;  /* Hash value of dependency name. */
    Elf32_Half vna_flags; /* Flags. */
    Elf32_Half vna_other; /* Unused. */
    Elf32_Word vna_name;  /* Offset to dependency name. */
    Elf32_Word vna_next;  /* Offset to next vernaux entry. */
} Elf32_Vernaux;

typedef struct {
    Elf32_Half vd_version; /* Version information. */
    Elf32_Half vd_flags;   /* Flags. */
    Elf32_Half vd_ndx;     /* Index into the versym section. */
    Elf32_Half vd_cnt;     /* Number of aux entries. */
    Elf32_Word vd_hash;    /* Hash value of name. */
    Elf32_Word vd_aux;     /* Offset to aux entries. */
    Elf32_Word vd_next;    /* Offset to next version definition. */
} Elf32_Verdef;

typedef struct {
    Elf32_Half vn_version; /* Version number. */
    Elf32_Half vn_cnt;     /* Number of aux entries. */
    Elf32_Word vn_file;    /* Offset of associated file name. */
    Elf32_Word vn_aux;     /* Offset of vernaux array. */
    Elf32_Word vn_next;    /* Offset of next verneed entry. */
} Elf32_Verneed;

typedef Elf32_Half Elf32_Versym;

/* 64-bit structures. */

typedef struct {
    Elf64_Word vda_name; /* Index to name. */
    Elf64_Word vda_next; /* Offset to next entry. */
} Elf64_Verdaux;

typedef struct {
    Elf64_Word vna_hash;  /* Hash value of dependency name. */
    Elf64_Half vna_flags; /* Flags. */
    Elf64_Half vna_other; /* Unused. */
    Elf64_Word vna_name;  /* Offset to dependency name. */
    Elf64_Word vna_next;  /* Offset to next vernaux entry. */
} Elf64_Vernaux;

typedef struct {
    Elf64_Half vd_version; /* Version information. */
    Elf64_Half vd_flags;   /* Flags. */
    Elf64_Half vd_ndx;     /* Index into the versym section. */
    Elf64_Half vd_cnt;     /* Number of aux entries. */
    Elf64_Word vd_hash;    /* Hash value of name. */
    Elf64_Word vd_aux;     /* Offset to aux entries. */
    Elf64_Word vd_next;    /* Offset to next version definition. */
} Elf64_Verdef;

typedef struct {
    Elf64_Half vn_version; /* Version number. */
    Elf64_Half vn_cnt;     /* Number of aux entries. */
    Elf64_Word vn_file;    /* Offset of associated file name. */
    Elf64_Word vn_aux;     /* Offset of vernaux array. */
    Elf64_Word vn_next;    /* Offset of next verneed entry. */
} Elf64_Verneed;

typedef Elf64_Half Elf64_Versym;

/*
 * The header for GNU-style hash sections.
 */

typedef struct {
    uint32_t gh_nbuckets;  /* Number of hash buckets. */
    uint32_t gh_symndx;    /* First visible symbol in .dynsym. */
    uint32_t gh_maskwords; /* #maskwords used in bloom filter. */
    uint32_t gh_shift2;    /* Bloom filter shift count. */
} Elf_GNU_Hash_Header;

/*
 * Section types.
 */

#define SHT_NULL               0
#define SHT_PROGBITS           1
#define SHT_SYMTAB             2
#define SHT_STRTAB             3
#define SHT_RELA               4
#define SHT_HASH               5
#define SHT_DYNAMIC            6
#define SHT_NOTE               7
#define SHT_NOBITS             8
#define SHT_REL                9
#define SHT_SHLIB              10
#define SHT_DYNSYM             11
#define SHT_INIT_ARRAY         14
#define SHT_FINI_ARRAY         15
#define SHT_PREINIT_ARRAY      16
#define SHT_GROUP              17
#define SHT_SYMTAB_SHNDX       18
#define SHT_LOOS               0x60000000UL
#define SHT_SUNW_dof           0x6FFFFFF4UL
#define SHT_SUNW_cap           0x6FFFFFF5UL
#define SHT_GNU_ATTRIBUTES     0x6FFFFFF5UL
#define SHT_SUNW_SIGNATURE     0x6FFFFFF6UL
#define SHT_GNU_HASH           0x6FFFFFF6UL
#define SHT_GNU_LIBLIST        0x6FFFFFF7UL
#define SHT_SUNW_ANNOTATE      0x6FFFFFF7UL
#define SHT_SUNW_DEBUGSTR      0x6FFFFFF8UL
#define SHT_CHECKSUM           0x6FFFFFF8UL
#define SHT_SUNW_DEBUG         0x6FFFFFF9UL
#define SHT_SUNW_move          0x6FFFFFFAUL
#define SHT_SUNW_COMDAT        0x6FFFFFFBUL
#define SHT_SUNW_syminfo       0x6FFFFFFCUL
#define SHT_SUNW_verdef        0x6FFFFFFDUL
#define SHT_SUNW_verneed       0x6FFFFFFEUL
#define SHT_SUNW_versym        0x6FFFFFFFUL
#define SHT_HIOS               0x6FFFFFFFUL
#define SHT_LOPROC             0x70000000UL
#define SHT_ARM_EXIDX          0x70000001UL
#define SHT_ARM_PREEMPTMAP     0x70000002UL
#define SHT_ARM_ATTRIBUTES     0x70000003UL
#define SHT_ARM_DEBUGOVERLAY   0x70000004UL
#define SHT_ARM_OVERLAYSECTION 0x70000005UL
#define SHT_MIPS_LIBLIST       0x70000000UL
#define SHT_MIPS_MSYM          0x70000001UL
#define SHT_MIPS_CONFLICT      0x70000002UL
#define SHT_MIPS_GPTAB         0x70000003UL
#define SHT_MIPS_UCODE         0x70000004UL
#define SHT_MIPS_DEBUG         0x70000005UL
#define SHT_MIPS_REGINFO       0x70000006UL
#define SHT_MIPS_PACKAGE       0x70000007UL
#define SHT_MIPS_PACKSYM       0x70000008UL
#define SHT_MIPS_RELD          0x70000009UL
#define SHT_MIPS_IFACE         0x7000000BUL
#define SHT_MIPS_CONTENT       0x7000000CUL
#define SHT_MIPS_OPTIONS       0x7000000DUL
#define SHT_MIPS_DELTASYM      0x7000001BUL
#define SHT_MIPS_DELTAINST     0x7000001CUL
#define SHT_MIPS_DELTACLASS    0x7000001DUL
#define SHT_MIPS_DWARF         0x7000001EUL
#define SHT_MIPS_DELTADECL     0x7000001FUL
#define SHT_MIPS_SYMBOL_LIB    0x70000020UL
#define SHT_MIPS_EVENTS        0x70000021UL
#define SHT_MIPS_TRANSLATE     0x70000022UL
#define SHT_MIPS_PIXIE         0x70000023UL
#define SHT_MIPS_XLATE         0x70000024UL
#define SHT_MIPS_XLATE_DEBUG   0x70000025UL
#define SHT_MIPS_WHIRL         0x70000026UL
#define SHT_MIPS_EH_REGION     0x70000027UL
#define SHT_MIPS_XLATE_OLD     0x70000028UL
#define SHT_MIPS_PDR_EXCEPTION 0x70000029UL
#define SHT_MIPS_ABIFLAGS      0x7000002AUL
#define SHT_SPARC_GOTDATA      0x70000000UL
#define SHT_X86_64_UNWIND      0x70000001UL
#define SHT_ORDERED            0x7FFFFFFFUL
#define SHT_HIPROC             0x7FFFFFFFUL
#define SHT_LOUSER             0x80000000UL
#define SHT_HIUSER             0xFFFFFFFFUL

#endif /* FPM_ELF_H */
