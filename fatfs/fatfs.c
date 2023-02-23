/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem Module  R0.15 w/patch1                      /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2022, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:
/
/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/
/----------------------------------------------------------------------------*/

#include <string.h>
#include <rpm/diskio.h> /* Declarations of device I/O functions */
#include <rpm/api.h> /* Declaration of rpm_get_datetime() */
#include "fatfs.h" /* Definitions of FatFs structures */

//-------------------------------------------------------------------------
// Module Private Definitions
//

//
// Limits and boundaries
//
#define MAX_DIR 0x200000      /* Max size of FAT directory */
#define MAX_DIR_EX 0x10000000 /* Max size of exFAT directory */
#define MAX_FAT12 0xFF5       /* Max FAT12 clusters (differs from specs, */
                              /* but right for real DOS/Windows behavior) */
#define MAX_FAT16 0xFFF5      /* Max FAT16 clusters (differs from specs, */
                              /* but right for real DOS/Windows behavior) */
#define MAX_FAT32 0x0FFFFFF5  /* Max FAT32 clusters (not specified, practical limit) */
#define MAX_EXFAT 0x7FFFFFFD  /* Max exFAT clusters (differs from specs, implementation limit) */

//
// Character code support macros
//
#define IsUpper(c) ((c) >= 'A' && (c) <= 'Z')
#define IsLower(c) ((c) >= 'a' && (c) <= 'z')
#define IsDigit(c) ((c) >= '0' && (c) <= '9')
#define IsSeparator(c) ((c) == '/' || (c) == '\\')
#define IsTerminator(c) ((unsigned)(c) < ' ')
#define IsSurrogate(c) ((c) >= 0xD800 && (c) <= 0xDFFF)
#define IsSurrogateH(c) ((c) >= 0xD800 && (c) <= 0xDBFF)
#define IsSurrogateL(c) ((c) >= 0xDC00 && (c) <= 0xDFFF)

//
// Additional file access control and file status flags for internal use
//
#define FA_SEEKEND 0x20  /* Seek to end of the file on file open */
#define FA_MODIFIED 0x40 /* File has been modified */
#define FA_DIRTY 0x80    /* file_t.buf[] needs to be written-back */

//
// Additional file attribute bits for internal use
//
#define AM_VOL 0x08      /* Volume label */
#define AM_LFN 0x0F      /* LFN entry */
#define AM_MASK 0x3F     /* Mask of defined bits in FAT */
#define AM_MASKX 0x37    /* Mask of defined bits in exFAT */

//
// Name status flags in fn[11]
//
#define NSFLAG 11        /* Index of the name status byte */
#define NS_LOSS 0x01     /* Out of 8.3 format */
#define NS_LFN 0x02      /* Force to create LFN entry */
#define NS_LAST 0x04     /* Last segment */
#define NS_BODY 0x08     /* Lower case flag (body) */
#define NS_EXT 0x10      /* Lower case flag (ext) */
#define NS_DOT 0x20      /* Dot entry */
#define NS_NOLFN 0x40    /* Do not find LFN */
#define NS_NONAME 0x80   /* Not followed */

//
// exFAT directory entry types
//
#define ET_BITMAP 0x81   /* Allocation bitmap */
#define ET_UPCASE 0x82   /* Up-case table */
#define ET_VLABEL 0x83   /* Volume label */
#define ET_FILEDIR 0x85  /* File and directory */
#define ET_STREAM 0xC0   /* Stream extension */
#define ET_FILENAME 0xC1 /* Name extension */

//
// FatFs refers the FAT structure as simple byte array instead of structure member
// because the C structure is not binary compatible between different platforms
//
#define BS_JmpBoot 0      /* x86 jump instruction (3-byte) */
#define BS_OEMName 3      /* OEM name (8-byte) */
#define BPB_BytsPerSec 11 /* Sector size [byte] (uint16_t) */
#define BPB_SecPerClus 13 /* Cluster size [sector] (uint8_t) */
#define BPB_RsvdSecCnt 14 /* Size of reserved area [sector] (uint16_t) */
#define BPB_NumFATs 16    /* Number of FATs (uint8_t) */
#define BPB_RootEntCnt 17 /* Size of root directory area for FAT [entry] (uint16_t) */
#define BPB_TotSec16 19   /* Volume size (16-bit) [sector] (uint16_t) */
#define BPB_Media 21      /* Media descriptor byte (uint8_t) */
#define BPB_FATSz16 22    /* FAT size (16-bit) [sector] (uint16_t) */
#define BPB_SecPerTrk 24  /* Number of sectors per track for int13h [sector] (uint16_t) */
#define BPB_NumHeads 26   /* Number of heads for int13h (uint16_t) */
#define BPB_HiddSec 28    /* Volume offset from top of the drive (uint32_t) */
#define BPB_TotSec32 32   /* Volume size (32-bit) [sector] (uint32_t) */
#define BS_DrvNum 36      /* Physical drive number for int13h (uint8_t) */
#define BS_NTres 37       /* WindowsNT error flag (uint8_t) */
#define BS_BootSig 38     /* Extended boot signature (uint8_t) */
#define BS_VolID 39       /* Volume serial number (uint32_t) */
#define BS_VolLab 43      /* Volume label string (8-byte) */
#define BS_FilSysType 54  /* Filesystem type string (8-byte) */
#define BS_BootCode 62    /* Boot code (448-byte) */
#define BS_55AA 510       /* Signature word (uint16_t) */

#define BPB_FATSz32 36     /* FAT32: FAT size [sector] (uint32_t) */
#define BPB_ExtFlags32 40  /* FAT32: Extended flags (uint16_t) */
#define BPB_FSVer32 42     /* FAT32: Filesystem version (uint16_t) */
#define BPB_RootClus32 44  /* FAT32: Root directory cluster (uint32_t) */
#define BPB_FSInfo32 48    /* FAT32: Offset of FSINFO sector (uint16_t) */
#define BPB_BkBootSec32 50 /* FAT32: Offset of backup boot sector (uint16_t) */
#define BS_DrvNum32 64     /* FAT32: Physical drive number for int13h (uint8_t) */
#define BS_NTres32 65      /* FAT32: Error flag (uint8_t) */
#define BS_BootSig32 66    /* FAT32: Extended boot signature (uint8_t) */
#define BS_VolID32 67      /* FAT32: Volume serial number (uint32_t) */
#define BS_VolLab32 71     /* FAT32: Volume label string (8-byte) */
#define BS_FilSysType32 82 /* FAT32: Filesystem type string (8-byte) */
#define BS_BootCode32 90   /* FAT32: Boot code (420-byte) */

#define BPB_ZeroedEx 11      /* exFAT: MBZ field (53-byte) */
#define BPB_VolOfsEx 64      /* exFAT: Volume offset from top of the drive [sector] (uint64_t) */
#define BPB_TotSecEx 72      /* exFAT: Volume size [sector] (uint64_t) */
#define BPB_FatOfsEx 80      /* exFAT: FAT offset from top of the volume [sector] (uint32_t) */
#define BPB_FatSzEx 84       /* exFAT: FAT size [sector] (uint32_t) */
#define BPB_DataOfsEx 88     /* exFAT: Data offset from top of the volume [sector] (uint32_t) */
#define BPB_NumClusEx 92     /* exFAT: Number of clusters (uint32_t) */
#define BPB_RootClusEx 96    /* exFAT: Root directory start cluster (uint32_t) */
#define BPB_VolIDEx 100      /* exFAT: Volume serial number (uint32_t) */
#define BPB_FSVerEx 104      /* exFAT: Filesystem version (uint16_t) */
#define BPB_VolFlagEx 106    /* exFAT: Volume flags (uint16_t) */
#define BPB_BytsPerSecEx 108 /* exFAT: Log2 of sector size in unit of byte (uint8_t) */
#define BPB_SecPerClusEx 109 /* exFAT: Log2 of cluster size in unit of sector (uint8_t) */
#define BPB_NumFATsEx 110    /* exFAT: Number of FATs (uint8_t) */
#define BPB_DrvNumEx 111     /* exFAT: Physical drive number for int13h (uint8_t) */
#define BPB_PercInUseEx 112  /* exFAT: Percent in use (uint8_t) */
#define BPB_RsvdEx 113       /* exFAT: Reserved (7-byte) */
#define BS_BootCodeEx 120    /* exFAT: Boot code (390-byte) */

#define DIR_Name 0            /* Short file name (11-byte) */
#define DIR_Attr 11           /* Attribute (uint8_t) */
#define DIR_NTres 12          /* Lower case flag (uint8_t) */
#define DIR_CrtTime10 13      /* Created time sub-second (uint8_t) */
#define DIR_CrtTime 14        /* Created time (uint32_t) */
#define DIR_LstAccDate 18     /* Last accessed date (uint16_t) */
#define DIR_FstClusHI 20      /* Higher 16-bit of first cluster (uint16_t) */
#define DIR_ModTime 22        /* Modified time (uint32_t) */
#define DIR_FstClusLO 26      /* Lower 16-bit of first cluster (uint16_t) */
#define DIR_FileSize 28       /* File size (uint32_t) */
#define LDIR_Ord 0            /* LFN: LFN order and LLE flag (uint8_t) */
#define LDIR_Attr 11          /* LFN: LFN attribute (uint8_t) */
#define LDIR_Type 12          /* LFN: Entry type (uint8_t) */
#define LDIR_Chksum 13        /* LFN: Checksum of the SFN (uint8_t) */
#define LDIR_FstClusLO 26     /* LFN: MBZ field (uint16_t) */
#define XDIR_Type 0           /* exFAT: Type of exFAT directory entry (uint8_t) */
#define XDIR_NumLabel 1       /* exFAT: Number of volume label characters (uint8_t) */
#define XDIR_Label 2          /* exFAT: Volume label (11-uint16_t) */
#define XDIR_CaseSum 4        /* exFAT: Sum of case conversion table (uint32_t) */
#define XDIR_NumSec 1         /* exFAT: Number of secondary entries (uint8_t) */
#define XDIR_SetSum 2         /* exFAT: Sum of the set of directory entries (uint16_t) */
#define XDIR_Attr 4           /* exFAT: File attribute (uint16_t) */
#define XDIR_CrtTime 8        /* exFAT: Created time (uint32_t) */
#define XDIR_ModTime 12       /* exFAT: Modified time (uint32_t) */
#define XDIR_AccTime 16       /* exFAT: Last accessed time (uint32_t) */
#define XDIR_CrtTime10 20     /* exFAT: Created time subsecond (uint8_t) */
#define XDIR_ModTime10 21     /* exFAT: Modified time subsecond (uint8_t) */
#define XDIR_CrtTZ 22         /* exFAT: Created timezone (uint8_t) */
#define XDIR_ModTZ 23         /* exFAT: Modified timezone (uint8_t) */
#define XDIR_AccTZ 24         /* exFAT: Last accessed timezone (uint8_t) */
#define XDIR_GenFlags 33      /* exFAT: General secondary flags (uint8_t) */
#define XDIR_NumName 35       /* exFAT: Number of file name characters (uint8_t) */
#define XDIR_NameHash 36      /* exFAT: Hash of file name (uint16_t) */
#define XDIR_ValidFileSize 40 /* exFAT: Valid file size (uint64_t) */
#define XDIR_FstClus 52       /* exFAT: First cluster of the file data (uint32_t) */
#define XDIR_FileSize 56      /* exFAT: File/Directory size (uint64_t) */

#define SZDIRE 32  /* Size of a directory entry */
#define DDEM 0xE5  /* Deleted directory entry mark set to DIR_Name[0] */
#define RDDEM 0x05 /* Replacement of the character collides with DDEM */
#define LLEF 0x40  /* Last long entry flag in LDIR_Ord */

#define FSI_LeadSig 0      /* FAT32 FSI: Leading signature (uint32_t) */
#define FSI_StrucSig 484   /* FAT32 FSI: Structure signature (uint32_t) */
#define FSI_Free_Count 488 /* FAT32 FSI: Number of free clusters (uint32_t) */
#define FSI_Nxt_Free 492   /* FAT32 FSI: Last allocated cluster (uint32_t) */

#define MBR_Table 446 /* MBR: Offset of partition table in the MBR */
#define SZ_PTE 16     /* MBR: Size of a partition table entry */
#define PTE_Boot 0    /* MBR PTE: Boot indicator */
#define PTE_StHead 1  /* MBR PTE: Start head */
#define PTE_StSec 2   /* MBR PTE: Start sector */
#define PTE_StCyl 3   /* MBR PTE: Start cylinder */
#define PTE_System 4  /* MBR PTE: System ID */
#define PTE_EdHead 5  /* MBR PTE: End head */
#define PTE_EdSec 6   /* MBR PTE: End sector */
#define PTE_EdCyl 7   /* MBR PTE: End cylinder */
#define PTE_StLba 8   /* MBR PTE: Start in LBA */
#define PTE_SizLba 12 /* MBR PTE: Size in LBA */

#define GPTH_Sign 0     /* GPT HDR: Signature (8-byte) */
#define GPTH_Rev 8      /* GPT HDR: Revision (uint32_t) */
#define GPTH_Size 12    /* GPT HDR: Header size (uint32_t) */
#define GPTH_Bcc 16     /* GPT HDR: Header BCC (uint32_t) */
#define GPTH_CurLba 24  /* GPT HDR: This header LBA (uint64_t) */
#define GPTH_BakLba 32  /* GPT HDR: Another header LBA (uint64_t) */
#define GPTH_FstLba 40  /* GPT HDR: First LBA for partition data (uint64_t) */
#define GPTH_LstLba 48  /* GPT HDR: Last LBA for partition data (uint64_t) */
#define GPTH_DskGuid 56 /* GPT HDR: Disk GUID (16-byte) */
#define GPTH_PtOfs 72   /* GPT HDR: Partition table LBA (uint64_t) */
#define GPTH_PtNum 80   /* GPT HDR: Number of table entries (uint32_t) */
#define GPTH_PteSize 84 /* GPT HDR: Size of table entry (uint32_t) */
#define GPTH_PtBcc 88   /* GPT HDR: Partition table BCC (uint32_t) */
#define SZ_GPTE 128     /* GPT PTE: Size of partition table entry */
#define GPTE_PtGuid 0   /* GPT PTE: Partition type GUID (16-byte) */
#define GPTE_UpGuid 16  /* GPT PTE: Partition unique GUID (16-byte) */
#define GPTE_FstLba 32  /* GPT PTE: First LBA of partition (uint64_t) */
#define GPTE_LstLba 40  /* GPT PTE: Last LBA of partition (uint64_t) */
#define GPTE_Flags 48   /* GPT PTE: Partition flags (uint64_t) */
#define GPTE_Name 56    /* GPT PTE: Partition name */

//
// Post process on fatal error in the file operations
//
#define ABORT(fs, res)         \
    {                          \
        fp->err = (uint8_t)(res); \
        LEAVE_FF(fs, res);     \
    }

//
// Re-entrancy related
//
#if FF_FS_REENTRANT
#if FF_USE_LFN == 1
#error Static LFN work area cannot be used in thread-safe configuration
#endif
#define LEAVE_FF(fs, res)       \
    {                           \
        unlock_volume(fs, res); \
        return res;             \
    }
#else
#define LEAVE_FF(fs, res) return res
#endif

//
// Definitions of logical drive - physical location conversion
//
#define LD2PD(vol) (uint8_t)(vol) // Each logical drive is associated with
                               // the same physical drive number
#define LD2PT(vol) 0           // Auto partition search

//
// Definitions of sector size
//
#if (FF_MAX_SS < FF_MIN_SS) ||                                                           \
    (FF_MAX_SS != 512 && FF_MAX_SS != 1024 && FF_MAX_SS != 2048 && FF_MAX_SS != 4096) || \
    (FF_MIN_SS != 512 && FF_MIN_SS != 1024 && FF_MIN_SS != 2048 && FF_MIN_SS != 4096)
#error Wrong sector size configuration
#endif
#if FF_MAX_SS == FF_MIN_SS
#define SS(fs) ((unsigned)FF_MAX_SS) /* Fixed sector size */
#else
#define SS(fs) ((fs)->ssize) /* Variable sector size */
#endif

//
// Timestamp
//
#if FF_FS_NORTC == 1
#if FF_NORTC_YEAR < 1980 || FF_NORTC_YEAR > 2107 || FF_NORTC_MON < 1 || FF_NORTC_MON > 12 || \
    FF_NORTC_MDAY < 1 || FF_NORTC_MDAY > 31
#error Invalid FF_FS_NORTC settings
#endif
#define GET_FATTIME() ((uint32_t)(FF_NORTC_YEAR - 1980) << 25 |
                       (uint32_t)FF_NORTC_MON << 21 | (uint32_t)FF_NORTC_MDAY << 16)
#else
#define GET_FATTIME() get_fattime()
#endif

//
// File lock controls
//
#if FF_FS_LOCK
#if FF_FS_READONLY
#error FF_FS_LOCK must be 0 at read-only configuration
#endif
typedef struct {
    filesystem_t *fs; /* Object ID 1, volume (NULL:blank entry) */
    uint32_t clu; /* Object ID 2, containing directory (0:root) */
    uint32_t ofs; /* Object ID 3, offset in the directory */
    unsigned ctr;  /* Object open counter, 0:none, 0x01..0xFF:read mode open
                  count, 0x100:write mode */
} FILESEM;
#endif

/* Macros for table definitions */
#define CODEPAGE 0 // No code pages, only simple ASCII

//-------------------------------------------------------------------------
// Module Private Work Area
//

// Remark: Variables defined here without initial value shall be guaranteed
// zero/null at start-up. If not, the linker option or start-up routine is
// not compliance with C standard.

//
// File/Volume controls
//

#if DISK_VOLUMES < 1 || DISK_VOLUMES > 10
#error Wrong DISK_VOLUMES setting
#endif
static filesystem_t *FatFs[DISK_VOLUMES]; // Pointer to the filesystem objects (logical drives)
static uint16_t Fsid;                     // Filesystem mount ID

#if FF_FS_RPATH != 0
static uint8_t CurrVol; /* Current drive set by f_chdrive() */
#endif

#if FF_FS_LOCK != 0
static FILESEM Files[FF_FS_LOCK]; /* Open object lock semaphores */
#if FF_FS_REENTRANT
static uint8_t SysLock; /* System lock flag (0:no mutex, 1:unlocked, 2:locked) */
#endif
#endif

/*--------------------------------*/
/* LFN/Directory working buffer   */
/*--------------------------------*/

#if FF_USE_LFN == 0 /* Non-LFN configuration */
#error LFN must be enabled
#endif
#if FF_MAX_LFN < 12 || FF_MAX_LFN > 255
#error Wrong setting of FF_MAX_LFN
#endif
#if FF_LFN_BUF < FF_SFN_BUF || FF_SFN_BUF < 12
#error Wrong setting of FF_LFN_BUF or FF_SFN_BUF
#endif

/* FAT: Offset of LFN characters in the directory entry */
static const uint8_t LfnOfs[] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };

/* exFAT: Size of directory entry block scratchpad buffer needed for the name length */
#define MAXDIRB(nc) ((nc + 44U) / 15 * SZDIRE)

//
// LFN enabled with static working buffer
//
#if FF_USE_LFN == 1

static uint8_t DirBuf[MAXDIRB(FF_MAX_LFN)]; /* Directory entry block scratchpad buffer */
static uint16_t LfnBuf[FF_MAX_LFN + 1];     /* LFN working buffer */
#define DEF_NAMBUF
#define INIT_NAMBUF(fs)
#define FREE_NAMBUF()
#define LEAVE_MKFS(res) return res

//
// LFN enabled with dynamic working buffer on the stack
//
#elif FF_USE_LFN == 2

/* LFN working buffer and directory entry block scratchpad buffer */
#define DEF_NAMBUF              \
    uint16_t lbuf[FF_MAX_LFN + 1]; \
    uint8_t dbuf[MAXDIRB(FF_MAX_LFN)];
#define INIT_NAMBUF(fs)      \
    {                        \
        (fs)->lfnbuf = lbuf; \
        (fs)->dirbuf = dbuf; \
    }
#define FREE_NAMBUF()
#define LEAVE_MKFS(res) return res

//
// LFN enabled with dynamic working buffer on the heap
//
#elif FF_USE_LFN == 3

/* Pointer to LFN working buffer and directory entry block scratchpad buffer */
#define DEF_NAMBUF uint16_t *lfn;

#define INIT_NAMBUF(fs)                                                \
    {                                                                  \
        lfn = ff_memalloc((FF_MAX_LFN + 1) * 2 + MAXDIRB(FF_MAX_LFN)); \
        if (!lfn)                                                      \
            LEAVE_FF(fs, FR_NOT_ENOUGH_CORE);                          \
        (fs)->lfnbuf = lfn;                                            \
        (fs)->dirbuf = (uint8_t *)(lfn + FF_MAX_LFN + 1);                 \
    }
#define FREE_NAMBUF() ff_memfree(lfn)
#define LEAVE_MKFS(res)      \
    {                        \
        if (!work)           \
            ff_memfree(buf); \
        return res;          \
    }
#define MAX_MALLOC 0x8000 /* Must be >=FF_MAX_SS */

#else
#error Wrong setting of FF_USE_LFN
#endif /* FF_USE_LFN == 1 */

//-------------------------------------------------------------------------
// Module Private Functions
//

/*-----------------------------------------------------------------------*/
/* Load/Store multi-byte word in the FAT structure                       */
/*-----------------------------------------------------------------------*/

static uint16_t ld_word(const uint8_t *ptr) /*	 Load a 2-byte little-endian word */
{
    uint16_t rv;

    rv = ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}

static uint32_t ld_dword(const uint8_t *ptr) /* Load a 4-byte little-endian word */
{
    uint32_t rv;

    rv = ptr[3];
    rv = rv << 8 | ptr[2];
    rv = rv << 8 | ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}

static uint64_t ld_qword(const uint8_t *ptr) /* Load an 8-byte little-endian word */
{
    uint64_t rv;

    rv = ptr[7];
    rv = rv << 8 | ptr[6];
    rv = rv << 8 | ptr[5];
    rv = rv << 8 | ptr[4];
    rv = rv << 8 | ptr[3];
    rv = rv << 8 | ptr[2];
    rv = rv << 8 | ptr[1];
    rv = rv << 8 | ptr[0];
    return rv;
}

#if !FF_FS_READONLY
static void st_word(uint8_t *ptr, uint16_t val) /* Store a 2-byte word in little-endian */
{
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
}

static void st_dword(uint8_t *ptr, uint32_t val) /* Store a 4-byte word in little-endian */
{
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
}

static void st_qword(uint8_t *ptr, uint64_t val) /* Store an 8-byte word in little-endian */
{
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
    val >>= 8;
    *ptr++ = (uint8_t)val;
}
#endif /* !FF_FS_READONLY */

/*-----------------------------------------------------------------------*/
/* String functions                                                      */
/*-----------------------------------------------------------------------*/

/* Test if the byte is DBC 1st byte */
static int dbc_1st(uint8_t c)
{
    if (c != 0)
        return 0; /* Always false */
    return 0;
}

/* Test if the byte is DBC 2nd byte */
static int dbc_2nd(uint8_t c)
{
    if (c != 0)
        return 0; /* Always false */
    return 0;
}

//
// Get a Unicode code point from the char string in defined API encoding.
// Returns a character in UTF-16 encoding (>=0x10000 on surrogate pair,
// 0xFFFFFFFF on decode error).
//
static uint32_t tchar2uni(const char **str) // Pointer to pointer to char string in configured encoding
{
    uint32_t uc;
    const char *p = *str;

    /* UTF-8 input */
    uint8_t b;
    int nf;

    uc = (uint8_t)*p++;               /* Get an encoding unit */
    if (uc & 0x80) {               /* Multiple byte code? */
        if ((uc & 0xE0) == 0xC0) { /* 2-byte sequence? */
            uc &= 0x1F;
            nf = 1;
        } else if ((uc & 0xF0) == 0xE0) { /* 3-byte sequence? */
            uc &= 0x0F;
            nf = 2;
        } else if ((uc & 0xF8) == 0xF0) { /* 4-byte sequence? */
            uc &= 0x07;
            nf = 3;
        } else { /* Wrong sequence */
            return 0xFFFFFFFF;
        }
        do { /* Get trailing bytes */
            b = (uint8_t)*p++;
            if ((b & 0xC0) != 0x80)
                return 0xFFFFFFFF; /* Wrong sequence? */
            uc = uc << 6 | (b & 0x3F);
        } while (--nf != 0);
        if (uc < 0x80 || IsSurrogate(uc) || uc >= 0x110000)
            return 0xFFFFFFFF; /* Wrong code? */
        if (uc >= 0x010000)
            uc = 0xD800DC00 | ((uc - 0x10000) << 6 & 0x3FF0000) |
                 (uc & 0x3FF); /* Make a surrogate pair if needed */
    }

    *str = p; /* Next read pointer */
    return uc;
}

//
// Store a Unicode char in defined API encoding.
// Returns number of encoding units written
// (0:buffer overflow or wrong encoding).
//
static unsigned put_utf(uint32_t chr,  // UTF-16 encoded character (Surrogate pair if >=0x10000)
                    char *buf, // Output buffer
                    unsigned szb)   // Size of the buffer
{
    /* UTF-8 output */
    uint32_t hc;

    if (chr < 0x80) { /* Single byte code? */
        if (szb < 1)
            return 0; /* Buffer overflow? */
        *buf = (char)chr;
        return 1;
    }
    if (chr < 0x800) { /* 2-byte sequence? */
        if (szb < 2)
            return 0; /* Buffer overflow? */
        *buf++ = (char)(0xC0 | (chr >> 6 & 0x1F));
        *buf++ = (char)(0x80 | (chr >> 0 & 0x3F));
        return 2;
    }
    if (chr < 0x10000) { /* 3-byte sequence? */
        if (szb < 3 || IsSurrogate(chr))
            return 0; /* Buffer overflow or wrong code? */
        *buf++ = (char)(0xE0 | (chr >> 12 & 0x0F));
        *buf++ = (char)(0x80 | (chr >> 6 & 0x3F));
        *buf++ = (char)(0x80 | (chr >> 0 & 0x3F));
        return 3;
    }
    /* 4-byte sequence */
    if (szb < 4)
        return 0;                                /* Buffer overflow? */
    hc = ((chr & 0xFFFF0000) - 0xD8000000) >> 6; /* Get high 10 bits */
    chr = (chr & 0xFFFF) - 0xDC00;               /* Get low 10 bits */
    if (hc >= 0x100000 || chr >= 0x400)
        return 0; /* Wrong surrogate? */
    chr = (hc | chr) + 0x10000;
    *buf++ = (char)(0xF0 | (chr >> 18 & 0x07));
    *buf++ = (char)(0x80 | (chr >> 12 & 0x3F));
    *buf++ = (char)(0x80 | (chr >> 6 & 0x3F));
    *buf++ = (char)(0x80 | (chr >> 0 & 0x3F));
    return 4;
}

#if FF_FS_REENTRANT
/*-----------------------------------------------------------------------*/
/* Request/Release grant to access the volume                            */
/*-----------------------------------------------------------------------*/
/* 1:Ok, 0:timeout */
static int lock_volume(filesystem_t *fs,   /* Filesystem object to lock */
                       int syslock) /* System lock required */
{
    int rv;

#if FF_FS_LOCK
    rv = ff_mutex_take(fs->ldrv);       /* Lock the volume */
    if (rv && syslock) {                /* System lock reqiered? */
        rv = ff_mutex_take(DISK_VOLUMES); /* Lock the system */
        if (rv) {
            SysLock = 2; /* System lock succeeded */
        } else {
            ff_mutex_give(fs->ldrv); /* Failed system lock */
        }
    }
#else
    rv =
        syslock ? ff_mutex_take(fs->ldrv) : ff_mutex_take(fs->ldrv); /* Lock the volume (this is to
                                                                        prevent compiler warning) */
#endif
    return rv;
}

static void unlock_volume(filesystem_t *fs,   /* Filesystem object */
                          fs_result_t res) /* Result code to be returned */
{
    if (fs && res != FR_NOT_ENABLED && res != FR_INVALID_DRIVE && res != FR_TIMEOUT) {
#if FF_FS_LOCK
        if (SysLock == 2) { /* Is the system locked? */
            SysLock = 1;
            ff_mutex_give(DISK_VOLUMES);
        }
#endif
        ff_mutex_give(fs->ldrv); /* Unlock the volume */
    }
}

#endif

#if FF_FS_LOCK
/*-----------------------------------------------------------------------*/
/* File shareing control functions                                       */
/*-----------------------------------------------------------------------*/

/* Check if the file can be accessed */
static fs_result_t chk_share(directory_t *dp, // Directory object pointing the file to be checked
                         int acc) // Desired access type (0:Read mode open,
                                  // 1:Write mode open,  2:Delete or rename) */
{
    unsigned i, be;

    /* Search open object table for the object */
    be = 0;
    for (i = 0; i < FF_FS_LOCK; i++) {
        if (Files[i].fs) {                   /* Existing entry */
            if (Files[i].fs == dp->obj.fs && /* Check if the object matches
                                                with an open object */
                Files[i].clu == dp->obj.sclust && Files[i].ofs == dp->dptr)
                break;
        } else { /* Blank entry */
            be = 1;
        }
    }
    if (i == FF_FS_LOCK) { /* The object has not been opened */
        return (!be && acc != 2) ? FR_TOO_MANY_OPEN_FILES
                                 : FR_OK; /* Is there a blank entry for new object? */
    }

    /* The object was opened. Reject any open against writing file and all
     * write mode open */
    return (acc != 0 || Files[i].ctr == 0x100) ? FR_LOCKED : FR_OK;
}

static int enq_share(void) /* Check if an entry is available for a new object */
{
    unsigned i;

    for (i = 0; i < FF_FS_LOCK && Files[i].fs; i++)
        ; /* Find a free entry */
    return (i == FF_FS_LOCK) ? 0 : 1;
}

//
// Increment object open counter and returns its index.
// (0:Internal error)
//
static unsigned inc_share(directory_t *dp, // Directory object pointing the file to register or increment
                      int acc) // Desired access (0:Read, 1:Write, 2:Delete/Rename)
{
    unsigned i;

    for (i = 0; i < FF_FS_LOCK; i++) { /* Find the object */
        if (Files[i].fs == dp->obj.fs && Files[i].clu == dp->obj.sclust && Files[i].ofs == dp->dptr)
            break;
    }

    if (i == FF_FS_LOCK) { /* Not opened. Register it as new. */
        for (i = 0; i < FF_FS_LOCK && Files[i].fs; i++)
            ; /* Find a free entry */
        if (i == FF_FS_LOCK)
            return 0; /* No free entry to register (int err) */
        Files[i].fs = dp->obj.fs;
        Files[i].clu = dp->obj.sclust;
        Files[i].ofs = dp->dptr;
        Files[i].ctr = 0;
    }

    if (acc >= 1 && Files[i].ctr)
        return 0; /* Access violation (int err) */

    Files[i].ctr = acc ? 0x100 : Files[i].ctr + 1; /* Set semaphore value */

    return i + 1; /* Index number origin from 1 */
}

//
// Decrement object open counter
//
static fs_result_t dec_share(unsigned i) /* Semaphore index (1..) */
{
    unsigned n;
    fs_result_t res;

    if (--i < FF_FS_LOCK) { /* Index number origin from 0 */
        n = Files[i].ctr;
        if (n == 0x100)
            n = 0; /* If write mode open, delete the object semaphore */
        if (n > 0)
            n--; /* Decrement read mode open count */
        Files[i].ctr = n;
        if (n == 0) {        /* Delete the object semaphore if open count becomes zero */
            Files[i].fs = 0; /* Free the entry <<<If this memory write operation is not
                                in atomic, FF_FS_REENTRANT == 1 and DISK_VOLUMES > 1,
                                there is a potential error in this process >>> */
        }
        res = FR_OK;
    } else {
        res = FR_INT_ERR; /* Invalid index number */
    }
    return res;
}

static void clear_share(/* Clear all lock entries of the volume */
                        filesystem_t *fs)
{
    unsigned i;

    for (i = 0; i < FF_FS_LOCK; i++) {
        if (Files[i].fs == fs)
            Files[i].fs = 0;
    }
}

#endif /* FF_FS_LOCK */

/*-----------------------------------------------------------------------*/
/* Move/Flush disk access window in the filesystem object                */
/*-----------------------------------------------------------------------*/
#if !FF_FS_READONLY

/* Returns FR_OK or FR_DISK_ERR */
static fs_result_t sync_window(filesystem_t *fs) /* Filesystem object */
{
    fs_result_t res = FR_OK;

    if (fs->wflag) { /* Is the disk access window dirty? */
        if (disk_write(fs->pdrv, fs->win, fs->winsect, 1) ==
            DISK_OK) {                                    /* Write it back into the volume */
            fs->wflag = 0;                               /* Clear window dirty flag */
            if (fs->winsect - fs->fatbase < fs->fsize) { /* Is it in the 1st FAT? */
                if (fs->n_fats == 2)
                    disk_write(fs->pdrv, fs->win, fs->winsect + fs->fsize,
                               1); /* Reflect it to 2nd FAT if needed */
            }
        } else {
            res = FR_DISK_ERR;
        }
    }
    return res;
}
#endif

/* Returns FR_OK or FR_DISK_ERR */
static fs_result_t move_window(filesystem_t *fs,  /* Filesystem object */
                           fs_lba_t sect) /* Sector LBA to make appearance in the fs->win[] */
{
    fs_result_t res = FR_OK;

    if (sect != fs->winsect) { /* Window offset changed? */
#if !FF_FS_READONLY
        res = sync_window(fs); /* Flush the window */
#endif
        if (res == FR_OK) { /* Fill sector window with new data */
            if (disk_read(fs->pdrv, fs->win, sect, 1) != DISK_OK) {
                sect = (fs_lba_t)0 - 1; /* Invalidate window if read data is not valid */
                res = FR_DISK_ERR;
            }
            fs->winsect = sect;
        }
    }
    return res;
}

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Synchronize filesystem and data on the storage                        */
/*-----------------------------------------------------------------------*/

/* Returns FR_OK or FR_DISK_ERR */
static fs_result_t sync_fs(filesystem_t *fs) /* Filesystem object */
{
    fs_result_t res;

    res = sync_window(fs);
    if (res == FR_OK) {
        if (fs->fs_type == FS_FAT32 &&
            fs->fsi_flag == 1) { /* FAT32: Update FSInfo sector if needed */
            /* Create FSInfo structure */
            memset(fs->win, 0, sizeof fs->win);
            st_word(fs->win + BS_55AA, 0xAA55);                /* Boot signature */
            st_dword(fs->win + FSI_LeadSig, 0x41615252);       /* Leading signature */
            st_dword(fs->win + FSI_StrucSig, 0x61417272);      /* Structure signature */
            st_dword(fs->win + FSI_Free_Count, fs->free_clst); /* Number of free clusters */
            st_dword(fs->win + FSI_Nxt_Free, fs->last_clst);   /* Last allocated culuster */
            fs->winsect = fs->volbase + 1; /* Write it into the FSInfo sector (Next to VBR) */
            disk_write(fs->pdrv, fs->win, fs->winsect, 1);
            fs->fsi_flag = 0;
        }
        /* Make sure that no pending write process in the lower layer */
        if (disk_ioctl(fs->pdrv, CTRL_SYNC, 0) != DISK_OK)
            res = FR_DISK_ERR;
    }

    return res;
}

#endif

/*-----------------------------------------------------------------------*/
/* Get physical sector number from cluster number                        */
/*-----------------------------------------------------------------------*/

/* !=0:Sector number, 0:Failed (invalid cluster#) */
static fs_lba_t clst2sect(filesystem_t *fs,  /* Filesystem object */
                       uint32_t clst) /* Cluster# to be converted */
{
    clst -= 2; /* Cluster number is origin from 2 */
    if (clst >= fs->n_fatent - 2)
        return 0;                                  /* Is it invalid cluster number? */
    return fs->database + (fs_lba_t)fs->csize * clst; /* Start sector number of the cluster */
}

/*-----------------------------------------------------------------------*/
/* FAT access - Read value of an FAT entry                               */
/*-----------------------------------------------------------------------*/

/* 0xFFFFFFFF:Disk error, 1:Internal error, 2..0x7FFFFFFF:Cluster status */
static uint32_t get_fat(obj_id_t *obj, /* Corresponding object */
                     uint32_t clst)   /* Cluster number to get the value */
{
    unsigned wc, bc;
    uint32_t val;
    filesystem_t *fs = obj->fs;

    if (clst < 2 || clst >= fs->n_fatent) { /* Check if in valid range */
        val = 1;                            /* Internal error */

    } else {
        val = 0xFFFFFFFF; /* Default value falls on disk error */

        switch (fs->fs_type) {
        case FS_FAT12:
            bc = (unsigned)clst;
            bc += bc / 2;
            if (move_window(fs, fs->fatbase + (bc / SS(fs))) != FR_OK)
                break;
            wc = fs->win[bc++ % SS(fs)]; /* Get 1st byte of the entry */
            if (move_window(fs, fs->fatbase + (bc / SS(fs))) != FR_OK)
                break;
            wc |= fs->win[bc % SS(fs)] << 8;             /* Merge 2nd byte of the entry */
            val = (clst & 1) ? (wc >> 4) : (wc & 0xFFF); /* Adjust bit position */
            break;

        case FS_FAT16:
            if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 2))) != FR_OK)
                break;
            val = ld_word(fs->win + clst * 2 % SS(fs)); /* Simple uint16_t array */
            break;

        case FS_FAT32:
            if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 4))) != FR_OK)
                break;
            val = ld_dword(fs->win + clst * 4 % SS(fs)) &
                  0x0FFFFFFF; /* Simple uint32_t array but mask out upper 4 bits
                               */
            break;

        case FS_EXFAT:
            if ((obj->objsize != 0 && obj->sclust != 0) ||
                obj->stat == 0) {                /* Object except root dir must have valid
                                                    data length */
                uint32_t cofs = clst - obj->sclust; /* Offset from start cluster */
                uint32_t clen = (uint32_t)((fs_lba_t)((obj->objsize - 1) / SS(fs)) /
                                     fs->csize); /* Number of clusters - 1 */

                if (obj->stat == 2 && cofs <= clen) {             /* Is it a contiguous chain? */
                    val = (cofs == clen) ? 0x7FFFFFFF : clst + 1; /* No data on the FAT,
                                                                     generate the value */
                    break;
                }
                if (obj->stat == 3 && cofs < obj->n_cont) { /* Is it in the 1st fragment? */
                    val = clst + 1;                         /* Generate the value */
                    break;
                }
                if (obj->stat != 2) {       /* Get value from FAT if FAT chain is valid */
                    if (obj->n_frag != 0) { /* Is it on the growing edge? */
                        val = 0x7FFFFFFF;   /* Generate EOC */
                    } else {
                        if (move_window(fs, fs->fatbase + (clst / (SS(fs) / 4))) != FR_OK)
                            break;
                        val = ld_dword(fs->win + clst * 4 % SS(fs)) & 0x7FFFFFFF;
                    }
                    break;
                }
            }
            val = 1; /* Internal error */
            break;

        default:
            val = 1; /* Internal error */
        }
    }

    return val;
}

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* FAT access - Change value of an FAT entry                             */
/*-----------------------------------------------------------------------*/

/* FR_OK(0):succeeded, !=0:error */
static fs_result_t put_fat(filesystem_t *fs,  /* Corresponding filesystem object */
                       uint32_t clst, /* FAT index number (cluster number) to be changed */
                       uint32_t val)  /* New value to be set to the entry */
{
    unsigned bc;
    uint8_t *p;
    fs_result_t res = FR_INT_ERR;

    if (clst >= 2 && clst < fs->n_fatent) { /* Check if in valid range */
        switch (fs->fs_type) {
        case FS_FAT12:
            bc = (unsigned)clst;
            bc += bc / 2; /* bc: byte offset of the entry */
            res = move_window(fs, fs->fatbase + (bc / SS(fs)));
            if (res != FR_OK)
                break;
            p = fs->win + bc++ % SS(fs);
            *p = (clst & 1) ? ((*p & 0x0F) | ((uint8_t)val << 4)) : (uint8_t)val; /* Update 1st byte */
            fs->wflag = 1;
            res = move_window(fs, fs->fatbase + (bc / SS(fs)));
            if (res != FR_OK)
                break;
            p = fs->win + bc % SS(fs);
            *p = (clst & 1) ? (uint8_t)(val >> 4)
                            : ((*p & 0xF0) | ((uint8_t)(val >> 8) & 0x0F)); /* Update 2nd byte */
            fs->wflag = 1;
            break;

        case FS_FAT16:
            res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 2)));
            if (res != FR_OK)
                break;
            st_word(fs->win + clst * 2 % SS(fs), (uint16_t)val); /* Simple uint16_t array */
            fs->wflag = 1;
            break;

        case FS_FAT32:
        case FS_EXFAT:
            res = move_window(fs, fs->fatbase + (clst / (SS(fs) / 4)));
            if (res != FR_OK)
                break;
            if (fs->fs_type != FS_EXFAT) {
                val = (val & 0x0FFFFFFF) | (ld_dword(fs->win + clst * 4 % SS(fs)) & 0xF0000000);
            }
            st_dword(fs->win + clst * 4 % SS(fs), val);
            fs->wflag = 1;
            break;
        }
    }
    return res;
}

#endif /* !FF_FS_READONLY */

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* exFAT: Accessing FAT and Allocation Bitmap                            */
/*-----------------------------------------------------------------------*/

/*--------------------------------------*/
/* Find a contiguous free cluster block */
/*--------------------------------------*/

static uint32_t find_bitmap(/* 0:Not found, 2..:Cluster block found, 0xFFFFFFFF:Disk error */
                         filesystem_t *fs,  /* Filesystem object */
                         uint32_t clst, /* Cluster number to scan from */
                         uint32_t ncl   /* Number of contiguous clusters to find (1..) */
)
{
    uint8_t bm, bv;
    unsigned i;
    uint32_t val, scl, ctr;

    clst -= 2; /* The first bit in the bitmap corresponds to cluster #2 */
    if (clst >= fs->n_fatent - 2)
        clst = 0;
    scl = val = clst;
    ctr = 0;
    for (;;) {
        if (move_window(fs, fs->bitbase + val / 8 / SS(fs)) != FR_OK)
            return 0xFFFFFFFF;
        i = val / 8 % SS(fs);
        bm = 1 << (val % 8);
        do {
            do {
                bv = fs->win[i] & bm;
                bm <<= 1;                        /* Get bit value */
                if (++val >= fs->n_fatent - 2) { /* Next cluster (with wrap-around) */
                    val = 0;
                    bm = 0;
                    i = SS(fs);
                }
                if (bv == 0) { /* Is it a free cluster? */
                    if (++ctr == ncl)
                        return scl + 2; /* Check if run length is sufficient
                                           for required */
                } else {
                    scl = val;
                    ctr = 0; /* Encountered a cluster in-use, restart to scan
                              */
                }
                if (val == clst)
                    return 0; /* All cluster scanned? */
            } while (bm != 0);
            bm = 1;
        } while (++i < SS(fs));
    }
}

/*----------------------------------------*/
/* Set/Clear a block of allocation bitmap */
/*----------------------------------------*/

static fs_result_t change_bitmap(filesystem_t *fs,  /* Filesystem object */
                             uint32_t clst, /* Cluster number to change from */
                             uint32_t ncl,  /* Number of clusters to be changed */
                             int bv      /* bit value to be set (0 or 1) */
)
{
    uint8_t bm;
    unsigned i;
    fs_lba_t sect;

    clst -= 2;                              /* The first bit corresponds to cluster #2 */
    sect = fs->bitbase + clst / 8 / SS(fs); /* Sector address */
    i = clst / 8 % SS(fs);                  /* Byte offset in the sector */
    bm = 1 << (clst % 8);                   /* Bit mask in the byte */
    for (;;) {
        if (move_window(fs, sect++) != FR_OK)
            return FR_DISK_ERR;
        do {
            do {
                if (bv == (int)((fs->win[i] & bm) != 0))
                    return FR_INT_ERR; /* Is the bit expected value? */
                fs->win[i] ^= bm;      /* Flip the bit */
                fs->wflag = 1;
                if (--ncl == 0)
                    return FR_OK; /* All bits processed? */
            } while (bm <<= 1);   /* Next bit */
            bm = 1;
        } while (++i < SS(fs)); /* Next byte */
        i = 0;
    }
}

/*---------------------------------------------*/
/* Fill the first fragment of the FAT chain    */
/*---------------------------------------------*/

static fs_result_t fill_first_frag(obj_id_t *obj /* Pointer to the corresponding object */
)
{
    fs_result_t res;
    uint32_t cl, n;

    if (obj->stat == 3) { /* Has the object been changed 'fragmented' in this session? */
        for (cl = obj->sclust, n = obj->n_cont; n;
             cl++, n--) { /* Create cluster chain on the FAT */
            res = put_fat(obj->fs, cl, cl + 1);
            if (res != FR_OK)
                return res;
        }
        obj->stat = 0; /* Change status 'FAT chain is valid' */
    }
    return FR_OK;
}

/*---------------------------------------------*/
/* Fill the last fragment of the FAT chain     */
/*---------------------------------------------*/

static fs_result_t fill_last_frag(obj_id_t *obj, /* Pointer to the corresponding object */
                              uint32_t lcl,    /* Last cluster of the fragment */
                              uint32_t term    /* Value to set the last FAT entry */
)
{
    fs_result_t res;

    while (obj->n_frag > 0) { /* Create the chain of last fragment */
        res = put_fat(obj->fs, lcl - obj->n_frag + 1,
                      (obj->n_frag > 1) ? lcl - obj->n_frag + 2 : term);
        if (res != FR_OK)
            return res;
        obj->n_frag--;
    }
    return FR_OK;
}

#endif /* !FF_FS_READONLY */

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* FAT handling - Remove a cluster chain                                 */
/*-----------------------------------------------------------------------*/

static fs_result_t remove_chain(              /* FR_OK(0):succeeded, !=0:error */
                            obj_id_t *obj, /* Corresponding object */
                            uint32_t clst,   /* Cluster to remove a chain from */
                            uint32_t pclst   /* Previous cluster of clst (0 if entire chain) */
)
{
    fs_result_t res = FR_OK;
    uint32_t nxt;
    filesystem_t *fs = obj->fs;
    uint32_t scl = clst, ecl = clst;
#if FF_USE_TRIM
    fs_lba_t rt[2];
#endif

    if (clst < 2 || clst >= fs->n_fatent)
        return FR_INT_ERR; /* Check if in valid range */

    /* Mark the previous cluster 'EOC' on the FAT if it exists */
    if (pclst != 0 && (fs->fs_type != FS_EXFAT || obj->stat != 2)) {
        res = put_fat(fs, pclst, 0xFFFFFFFF);
        if (res != FR_OK)
            return res;
    }

    /* Remove the chain */
    do {
        nxt = get_fat(obj, clst); /* Get cluster status */
        if (nxt == 0)
            break; /* Empty cluster? */
        if (nxt == 1)
            return FR_INT_ERR; /* Internal error? */
        if (nxt == 0xFFFFFFFF)
            return FR_DISK_ERR; /* Disk error? */
        if (fs->fs_type != FS_EXFAT) {
            res = put_fat(fs, clst, 0); /* Mark the cluster 'free' on the FAT */
            if (res != FR_OK)
                return res;
        }
        if (fs->free_clst < fs->n_fatent - 2) { /* Update FSINFO */
            fs->free_clst++;
            fs->fsi_flag |= 1;
        }
        if (ecl + 1 == nxt) { /* Is next cluster contiguous? */
            ecl = nxt;
        } else { /* End of contiguous cluster block */
            if (fs->fs_type == FS_EXFAT) {
                res = change_bitmap(fs, scl, ecl - scl + 1,
                                    0); /* Mark the cluster block 'free' on the bitmap */
                if (res != FR_OK)
                    return res;
            }
#if FF_USE_TRIM
            rt[0] = clst2sect(fs, scl);                 /* Start of data area to be freed */
            rt[1] = clst2sect(fs, ecl) + fs->csize - 1; /* End of data area to be freed */
            disk_ioctl(fs->pdrv, CTRL_TRIM, rt); /* Inform storage device that the data in the
                                                    block may be erased */
#endif
            scl = ecl = nxt;
        }
        clst = nxt;                /* Next cluster */
    } while (clst < fs->n_fatent); /* Repeat while not the last link */

    /* Some post processes for chain status */
    if (fs->fs_type == FS_EXFAT) {
        if (pclst == 0) {  /* Has the entire chain been removed? */
            obj->stat = 0; /* Change the chain status 'initial' */
        } else {
            if (obj->stat == 0) {   /* Is it a fragmented chain from the
                                       beginning of this session? */
                clst = obj->sclust; /* Follow the chain to check if it gets
                                       contiguous */
                while (clst != pclst) {
                    nxt = get_fat(obj, clst);
                    if (nxt < 2)
                        return FR_INT_ERR;
                    if (nxt == 0xFFFFFFFF)
                        return FR_DISK_ERR;
                    if (nxt != clst + 1)
                        break; /* Not contiguous? */
                    clst++;
                }
                if (clst == pclst) { /* Has the chain got contiguous again? */
                    obj->stat = 2;   /* Change the chain status 'contiguous' */
                }
            } else {
                if (obj->stat == 3 && pclst >= obj->sclust &&
                    pclst <= obj->sclust + obj->n_cont) { /* Was the chain fragmented
                                                             in this session and got
                                                             contiguous again? */
                    obj->stat = 2;                        /* Change the chain status 'contiguous' */
                }
            }
        }
    }
    return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* FAT handling - Stretch a chain or Create a new chain                  */
/*-----------------------------------------------------------------------*/

static uint32_t create_chain(/* 0:No free cluster, 1:Internal error, 0xFFFFFFFF:Disk error,
                             >=2:New cluster# */
                          obj_id_t *obj, /* Corresponding object */
                          uint32_t clst    /* Cluster# to stretch, 0:Create a new chain */
)
{
    uint32_t cs, ncl, scl;
    fs_result_t res;
    filesystem_t *fs = obj->fs;

    if (clst == 0) {         /* Create a new chain */
        scl = fs->last_clst; /* Suggested cluster to start to find */
        if (scl == 0 || scl >= fs->n_fatent)
            scl = 1;
    } else {                     /* Stretch a chain */
        cs = get_fat(obj, clst); /* Check the cluster status */
        if (cs < 2)
            return 1; /* Test for insanity */
        if (cs == 0xFFFFFFFF)
            return cs; /* Test for disk error */
        if (cs < fs->n_fatent)
            return cs; /* It is already followed by next cluster */
        scl = clst;    /* Cluster to start to find */
    }
    if (fs->free_clst == 0)
        return 0; /* No free cluster */

    if (fs->fs_type == FS_EXFAT) {     /* On the exFAT volume */
        ncl = find_bitmap(fs, scl, 1); /* Find a free cluster */
        if (ncl == 0 || ncl == 0xFFFFFFFF)
            return ncl;                     /* No free cluster or hard error? */
        res = change_bitmap(fs, ncl, 1, 1); /* Mark the cluster 'in use' */
        if (res == FR_INT_ERR)
            return 1;
        if (res == FR_DISK_ERR)
            return 0xFFFFFFFF;
        if (clst == 0) {                            /* Is it a new chain? */
            obj->stat = 2;                          /* Set status 'contiguous' */
        } else {                                    /* It is a stretched chain */
            if (obj->stat == 2 && ncl != scl + 1) { /* Is the chain got fragmented? */
                obj->n_cont = scl - obj->sclust;    /* Set size of the contiguous part */
                obj->stat = 3;                      /* Change status 'just fragmented' */
            }
        }
        if (obj->stat != 2) {      /* Is the file non-contiguous? */
            if (ncl == clst + 1) { /* Is the cluster next to previous one? */
                obj->n_frag =
                    obj->n_frag ? obj->n_frag + 1 : 2; /* Increment size of last framgent */
            } else {                                   /* New fragment */
                if (obj->n_frag == 0)
                    obj->n_frag = 1;
                res = fill_last_frag(obj, clst, ncl); /* Fill last fragment on the FAT
                                                         and link it to new one */
                if (res == FR_OK)
                    obj->n_frag = 1;
            }
        }
    } else {
        /* On the FAT/FAT32 volume */
        ncl = 0;
        if (scl == clst) { /* Stretching an existing chain? */
            ncl = scl + 1; /* Test if next cluster is free */
            if (ncl >= fs->n_fatent)
                ncl = 2;
            cs = get_fat(obj, ncl); /* Get next cluster status */
            if (cs == 1 || cs == 0xFFFFFFFF)
                return cs;          /* Test for error */
            if (cs != 0) {          /* Not free? */
                cs = fs->last_clst; /* Start at suggested cluster if it is
                                       valid */
                if (cs >= 2 && cs < fs->n_fatent)
                    scl = cs;
                ncl = 0;
            }
        }
        if (ncl == 0) { /* The new cluster cannot be contiguous and find
                           another fragment */
            ncl = scl;  /* Start cluster */
            for (;;) {
                ncl++;                     /* Next cluster */
                if (ncl >= fs->n_fatent) { /* Check wrap-around */
                    ncl = 2;
                    if (ncl > scl)
                        return 0; /* No free cluster found? */
                }
                cs = get_fat(obj, ncl); /* Get the cluster status */
                if (cs == 0)
                    break; /* Found a free cluster? */
                if (cs == 1 || cs == 0xFFFFFFFF)
                    return cs; /* Test for error */
                if (ncl == scl)
                    return 0; /* No free cluster found? */
            }
        }
        res = put_fat(fs, ncl, 0xFFFFFFFF); /* Mark the new cluster 'EOC' */
        if (res == FR_OK && clst != 0) {
            res = put_fat(fs, clst, ncl); /* Link it from the previous one if needed */
        }
    }

    if (res == FR_OK) { /* Update FSINFO if function succeeded. */
        fs->last_clst = ncl;
        if (fs->free_clst <= fs->n_fatent - 2)
            fs->free_clst--;
        fs->fsi_flag |= 1;
    } else {
        ncl = (res == FR_DISK_ERR) ? 0xFFFFFFFF : 1; /* Failed. Generate error status */
    }

    return ncl; /* Return new cluster number or error status */
}

#endif /* !FF_FS_READONLY */

#if FF_USE_FASTSEEK
/*-----------------------------------------------------------------------*/
/* FAT handling - Convert offset into cluster with link map table        */
/*-----------------------------------------------------------------------*/

static uint32_t clmt_clust(            /* <2:Error, >=2:Cluster number */
                        file_t *fp,    /* Pointer to the file object */
                        fs_size_t ofs /* File offset to be converted to cluster# */
)
{
    uint32_t cl, ncl;
    uint32_t *tbl;
    filesystem_t *fs = fp->obj.fs;

    tbl = fp->cltbl + 1;                    /* Top of CLMT */
    cl = (uint32_t)(ofs / SS(fs) / fs->csize); /* Cluster order from top of the file */
    for (;;) {
        ncl = *tbl++; /* Number of cluters in the fragment */
        if (ncl == 0)
            return 0; /* End of table? (error) */
        if (cl < ncl)
            break; /* In this fragment? */
        cl -= ncl;
        tbl++; /* Next fragment */
    }
    return cl + *tbl; /* Return the cluster number */
}

#endif /* FF_USE_FASTSEEK */

/*-----------------------------------------------------------------------*/
/* Directory handling - Fill a cluster with zeros                        */
/*-----------------------------------------------------------------------*/

#if !FF_FS_READONLY
static fs_result_t dir_clear(           /* Returns FR_OK or FR_DISK_ERR */
                         filesystem_t *fs, /* Filesystem object */
                         uint32_t clst /* Directory table to clear */
)
{
    fs_lba_t sect;
    unsigned n, szb;
    uint8_t *ibuf;

    if (sync_window(fs) != FR_OK)
        return FR_DISK_ERR;             /* Flush disk access window */
    sect = clst2sect(fs, clst);         /* Top of the cluster */
    fs->winsect = sect;                 /* Set window to top of the cluster */
    memset(fs->win, 0, sizeof fs->win); /* Clear window buffer */
#if FF_USE_LFN == 3                     /* Quick table clear by using multi-secter write */
    /* Allocate a temporary buffer */
    for (szb = ((uint32_t)fs->csize * SS(fs) >= MAX_MALLOC) ? MAX_MALLOC : fs->csize * SS(fs),
        ibuf = 0;
         szb > SS(fs) && (ibuf = ff_memalloc(szb)) == 0; szb /= 2)
        ;
    if (szb > SS(fs)) { /* Buffer allocated? */
        memset(ibuf, 0, szb);
        szb /= SS(fs); /* Bytes -> Sectors */
        for (n = 0; n < fs->csize && disk_write(fs->pdrv, ibuf, sect + n, szb) == DISK_OK; n += szb)
            ; /* Fill the cluster with 0 */
        ff_memfree(ibuf);
    } else
#endif
    {
        ibuf = fs->win;
        szb = 1; /* Use window buffer (many single-sector writes may take a
                    time) */
        for (n = 0; n < fs->csize && disk_write(fs->pdrv, ibuf, sect + n, szb) == DISK_OK; n += szb)
            ; /* Fill the cluster with 0 */
    }
    return (n == fs->csize) ? FR_OK : FR_DISK_ERR;
}
#endif /* !FF_FS_READONLY */

/*-----------------------------------------------------------------------*/
/* Directory handling - Set directory index                              */
/*-----------------------------------------------------------------------*/

static fs_result_t dir_sdi(          /* FR_OK(0):succeeded, !=0:error */
                       directory_t *dp,  /* Pointer to directory object */
                       uint32_t ofs /* Offset of directory table */
)
{
    uint32_t csz, clst;
    filesystem_t *fs = dp->obj.fs;

    if (ofs >= (uint32_t)((fs->fs_type == FS_EXFAT) ? MAX_DIR_EX : MAX_DIR) ||
        ofs % SZDIRE) { /* Check range of offset and alignment */
        return FR_INT_ERR;
    }
    dp->dptr = ofs;                             /* Set current offset */
    clst = dp->obj.sclust;                      /* Table start cluster (0:root) */
    if (clst == 0 && fs->fs_type >= FS_FAT32) { /* Replace cluster# 0 with root cluster# */
        clst = (uint32_t)fs->dirbase;
        dp->obj.stat = 0; /* exFAT: Root dir has an FAT chain */
    }

    if (clst == 0) { /* Static table (root-directory on the FAT volume) */
        if (ofs / SZDIRE >= fs->n_rootdir)
            return FR_INT_ERR; /* Is index out of range? */
        dp->sect = fs->dirbase;

    } else {                                /* Dynamic table (sub-directory or root-directory on the
                                               FAT32/exFAT volume) */
        csz = (uint32_t)fs->csize * SS(fs);    /* Bytes per cluster */
        while (ofs >= csz) {                /* Follow cluster chain */
            clst = get_fat(&dp->obj, clst); /* Get next cluster */
            if (clst == 0xFFFFFFFF)
                return FR_DISK_ERR; /* Disk error */
            if (clst < 2 || clst >= fs->n_fatent)
                return FR_INT_ERR; /* Reached to end of table or internal
                                      error */
            ofs -= csz;
        }
        dp->sect = clst2sect(fs, clst);
    }
    dp->clust = clst; /* Current cluster# */
    if (dp->sect == 0)
        return FR_INT_ERR;
    dp->sect += ofs / SS(fs);           /* Sector# of the directory entry */
    dp->dir = fs->win + (ofs % SS(fs)); /* Pointer to the entry in the win[] */

    return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Directory handling - Move directory table index next                  */
/*-----------------------------------------------------------------------*/

static fs_result_t dir_next(            /* FR_OK(0):succeeded, FR_NO_FILE:End of table,
                                       FR_DENIED:Could not stretch */
                        directory_t *dp,    /* Pointer to the directory object */
                        int stretch /* 0: Do not stretch table, 1: Stretch
                                       table if needed */
)
{
    uint32_t ofs, clst;
    filesystem_t *fs = dp->obj.fs;

    ofs = dp->dptr + SZDIRE; /* Next entry */
    if (ofs >= (uint32_t)((fs->fs_type == FS_EXFAT) ? MAX_DIR_EX : MAX_DIR))
        dp->sect = 0; /* Disable it if the offset reached the max value */
    if (dp->sect == 0)
        return FR_NO_FILE; /* Report EOT if it has been disabled */

    if (ofs % SS(fs) == 0) { /* Sector changed? */
        dp->sect++;          /* Next sector */

        if (dp->clust == 0) {                    /* Static table */
            if (ofs / SZDIRE >= fs->n_rootdir) { /* Report EOT if it reached
                                                    end of static table */
                dp->sect = 0;
                return FR_NO_FILE;
            }
        } else {                                         /* Dynamic table */
            if ((ofs / SS(fs) & (fs->csize - 1)) == 0) { /* Cluster changed? */
                clst = get_fat(&dp->obj, dp->clust);     /* Get next cluster */
                if (clst <= 1)
                    return FR_INT_ERR; /* Internal error */
                if (clst == 0xFFFFFFFF)
                    return FR_DISK_ERR;     /* Disk error */
                if (clst >= fs->n_fatent) { /* It reached end of dynamic table */
#if !FF_FS_READONLY
                    if (!stretch) { /* If no stretch, report EOT */
                        dp->sect = 0;
                        return FR_NO_FILE;
                    }
                    clst = create_chain(&dp->obj, dp->clust); /* Allocate a cluster */
                    if (clst == 0)
                        return FR_DENIED; /* No free cluster */
                    if (clst == 1)
                        return FR_INT_ERR; /* Internal error */
                    if (clst == 0xFFFFFFFF)
                        return FR_DISK_ERR; /* Disk error */
                    if (dir_clear(fs, clst) != FR_OK)
                        return FR_DISK_ERR; /* Clean up the stretched table
                                             */
                    dp->obj.stat |= 4; /* exFAT: The directory has been stretched */
#else
                    if (!stretch)
                        dp->sect = 0; /* (this line is to suppress compiler warning) */
                    dp->sect = 0;
                    return FR_NO_FILE; /* Report EOT */
#endif
                }
                dp->clust = clst; /* Initialize data for new cluster */
                dp->sect = clst2sect(fs, clst);
            }
        }
    }
    dp->dptr = ofs;                   /* Current entry */
    dp->dir = fs->win + ofs % SS(fs); /* Pointer to the entry in the win[] */

    return FR_OK;
}

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Directory handling - Reserve a block of directory entries             */
/*-----------------------------------------------------------------------*/

static fs_result_t dir_alloc(           /* FR_OK(0):succeeded, !=0:error */
                         directory_t *dp,   /* Pointer to the directory object */
                         unsigned n_ent /* Number of contiguous entries to allocate */
)
{
    fs_result_t res;
    unsigned n;
    filesystem_t *fs = dp->obj.fs;

    res = dir_sdi(dp, 0);
    if (res == FR_OK) {
        n = 0;
        do {
            res = move_window(fs, dp->sect);
            if (res != FR_OK)
                break;

            if ((fs->fs_type == FS_EXFAT)
                    ? (int)((dp->dir[XDIR_Type] & 0x80) == 0)
                    : (int)(dp->dir[DIR_Name] == DDEM ||
                            dp->dir[DIR_Name] == 0)) { /* Is the entry free? */
                if (++n == n_ent)
                    break; /* Is a block of contiguous free entries found? */
            } else {
                n = 0; /* Not a free entry, restart to search */
            }
            res = dir_next(dp, 1); /* Next entry with table stretch enabled */
        } while (res == FR_OK);
    }

    if (res == FR_NO_FILE)
        res = FR_DENIED; /* No directory entry to allocate */
    return res;
}

#endif /* !FF_FS_READONLY */

/*-----------------------------------------------------------------------*/
/* FAT: Directory handling - Load/Store start cluster number             */
/*-----------------------------------------------------------------------*/

static uint32_t ld_clust(                /* Returns the top cluster value of the SFN entry */
                      filesystem_t *fs,      /* Pointer to the fs object */
                      const uint8_t *dir /* Pointer to the key entry */
)
{
    uint32_t cl;

    cl = ld_word(dir + DIR_FstClusLO);
    if (fs->fs_type == FS_FAT32) {
        cl |= (uint32_t)ld_word(dir + DIR_FstClusHI) << 16;
    }

    return cl;
}

#if !FF_FS_READONLY
static void st_clust(filesystem_t *fs, /* Pointer to the fs object */
                     uint8_t *dir, /* Pointer to the key entry */
                     uint32_t cl   /* Value to be set */
)
{
    st_word(dir + DIR_FstClusLO, (uint16_t)cl);
    if (fs->fs_type == FS_FAT32) {
        st_word(dir + DIR_FstClusHI, (uint16_t)(cl >> 16));
    }
}
#endif

/*--------------------------------------------------------*/
/* FAT-LFN: Compare a part of file name with an LFN entry */
/*--------------------------------------------------------*/

static int cmp_lfn(                     /* 1:matched, 0:not matched */
                   const uint16_t *lfnbuf, /* Pointer to the LFN working buffer
                                           to be compared */
                   uint8_t *dir            /* Pointer to the directory entry containing the
                                           part of LFN */
)
{
    unsigned i, s;
    uint16_t wc, uc;

    if (ld_word(dir + LDIR_FstClusLO) != 0)
        return 0; /* Check LDIR_FstClusLO */

    i = ((dir[LDIR_Ord] & 0x3F) - 1) * 13; /* Offset in the LFN buffer */

    for (wc = 1, s = 0; s < 13; s++) { /* Process all characters in the entry */
        uc = ld_word(dir + LfnOfs[s]); /* Pick an LFN character */
        if (wc != 0) {
            if (i >= FF_MAX_LFN + 1 ||
                ff_wtoupper(uc) != ff_wtoupper(lfnbuf[i++])) { /* Compare it */
                return 0;                                      /* Not matched */
            }
            wc = uc;
        } else {
            if (uc != 0xFFFF)
                return 0; /* Check filler */
        }
    }

    if ((dir[LDIR_Ord] & LLEF) && wc && lfnbuf[i])
        return 0; /* Last segment matched but different length */

    return 1; /* The part of LFN matched */
}

/*-----------------------------------------------------*/
/* FAT-LFN: Pick a part of file name from an LFN entry */
/*-----------------------------------------------------*/

static int pick_lfn(               /* 1:succeeded, 0:buffer overflow or invalid LFN entry */
                    uint16_t *lfnbuf, /* Pointer to the LFN working buffer */
                    uint8_t *dir      /* Pointer to the LFN entry */
)
{
    unsigned i, s;
    uint16_t wc, uc;

    if (ld_word(dir + LDIR_FstClusLO) != 0)
        return 0; /* Check LDIR_FstClusLO is 0 */

    i = ((dir[LDIR_Ord] & ~LLEF) - 1) * 13; /* Offset in the LFN buffer */

    for (wc = 1, s = 0; s < 13; s++) { /* Process all characters in the entry */
        uc = ld_word(dir + LfnOfs[s]); /* Pick an LFN character */
        if (wc != 0) {
            if (i >= FF_MAX_LFN + 1)
                return 0;          /* Buffer overflow? */
            lfnbuf[i++] = wc = uc; /* Store it */
        } else {
            if (uc != 0xFFFF)
                return 0; /* Check filler */
        }
    }

    if (dir[LDIR_Ord] & LLEF && wc != 0) { /* Put terminator if it is the last LFN part and not
                                              terminated */
        if (i >= FF_MAX_LFN + 1)
            return 0; /* Buffer overflow? */
        lfnbuf[i] = 0;
    }

    return 1; /* The part of LFN is valid */
}

#if !FF_FS_READONLY
/*-----------------------------------------*/
/* FAT-LFN: Create an entry of LFN entries */
/*-----------------------------------------*/

static void put_lfn(const uint16_t *lfn, /* Pointer to the LFN */
                    uint8_t *dir,        /* Pointer to the LFN entry to be created */
                    uint8_t ord,         /* LFN order (1-20) */
                    uint8_t sum          /* Checksum of the corresponding SFN */
)
{
    unsigned i, s;
    uint16_t wc;

    dir[LDIR_Chksum] = sum;  /* Set checksum */
    dir[LDIR_Attr] = AM_LFN; /* Set attribute. LFN entry */
    dir[LDIR_Type] = 0;
    st_word(dir + LDIR_FstClusLO, 0);

    i = (ord - 1) * 13; /* Get offset in the LFN working buffer */
    s = wc = 0;
    do {
        if (wc != 0xFFFF)
            wc = lfn[i++];            /* Get an effective character */
        st_word(dir + LfnOfs[s], wc); /* Put it */
        if (wc == 0)
            wc = 0xFFFF; /* Padding characters for following items */
    } while (++s < 13);
    if (wc == 0xFFFF || !lfn[i])
        ord |= LLEF;     /* Last LFN part is the start of LFN sequence */
    dir[LDIR_Ord] = ord; /* Set the LFN order */
}

#endif /* !FF_FS_READONLY */

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* FAT-LFN: Create a Numbered SFN                                        */
/*-----------------------------------------------------------------------*/

static void gen_numname(uint8_t *dst,        /* Pointer to the buffer to store numbered SFN */
                        const uint8_t *src,  /* Pointer to SFN in directory form */
                        const uint16_t *lfn, /* Pointer to LFN */
                        unsigned seq          /* Sequence number */
)
{
    uint8_t ns[8], c;
    unsigned i, j;
    uint16_t wc;
    uint32_t sreg;

    memcpy(dst, src, 11); /* Prepare the SFN to be modified */

    if (seq > 5) { /* In case of many collisions, generate a hash number
                      instead of sequential number */
        sreg = seq;
        while (*lfn) { /* Create a CRC as hash value */
            wc = *lfn++;
            for (i = 0; i < 16; i++) {
                sreg = (sreg << 1) + (wc & 1);
                wc >>= 1;
                if (sreg & 0x10000)
                    sreg ^= 0x11021;
            }
        }
        seq = (unsigned)sreg;
    }

    /* Make suffix (~ + hexadecimal) */
    i = 7;
    do {
        c = (uint8_t)((seq % 16) + '0');
        seq /= 16;
        if (c > '9')
            c += 7;
        ns[i--] = c;
    } while (i && seq);
    ns[i] = '~';

    /* Append the suffix to the SFN body */
    for (j = 0; j < i && dst[j] != ' '; j++) { /* Find the offset to append */
        if (dbc_1st(dst[j])) {                 /* To avoid DBC break up */
            if (j == i - 1)
                break;
            j++;
        }
    }
    do { /* Append the suffix */
        dst[j++] = (i < 8) ? ns[i++] : ' ';
    } while (j < 8);
}
#endif /* !FF_FS_READONLY */

/*-----------------------------------------------------------------------*/
/* FAT-LFN: Calculate checksum of an SFN entry                           */
/*-----------------------------------------------------------------------*/

static uint8_t sum_sfn(const uint8_t *dir /* Pointer to the SFN entry */
)
{
    uint8_t sum = 0;
    unsigned n = 11;

    do {
        sum = (sum >> 1) + (sum << 7) + *dir++;
    } while (--n);
    return sum;
}

/*-----------------------------------------------------------------------*/
/* exFAT: Checksum                                                       */
/*-----------------------------------------------------------------------*/

static uint16_t xdir_sum(                /* Get checksum of the directoly entry block */
                     const uint8_t *dir /* Directory entry block to be calculated */
)
{
    unsigned i, szblk;
    uint16_t sum;

    szblk = (dir[XDIR_NumSec] + 1) * SZDIRE; /* Number of bytes of the entry block */
    for (i = sum = 0; i < szblk; i++) {
        if (i == XDIR_SetSum) { /* Skip 2-byte sum field */
            i++;
        } else {
            sum = ((sum & 1) ? 0x8000 : 0) + (sum >> 1) + dir[i];
        }
    }
    return sum;
}

static uint16_t xname_sum(                  /* Get check sum (to be used as hash) of the file name */
                      const uint16_t *name /* File name to be calculated */
)
{
    uint16_t chr;
    uint16_t sum = 0;

    while ((chr = *name++) != 0) {
        chr = (uint16_t)ff_wtoupper(chr); /* File name needs to be up-case converted */
        sum = ((sum & 1) ? 0x8000 : 0) + (sum >> 1) + (chr & 0xFF);
        sum = ((sum & 1) ? 0x8000 : 0) + (sum >> 1) + (chr >> 8);
    }
    return sum;
}

#if !FF_FS_READONLY && FF_USE_MKFS
static uint32_t xsum32(          /* Returns 32-bit checksum */
                    uint8_t dat, /* Byte to be calculated (byte-by-byte processing) */
                    uint32_t sum /* Previous sum value */
)
{
    sum = ((sum & 1) ? 0x80000000 : 0) + (sum >> 1) + dat;
    return sum;
}
#endif

/*------------------------------------*/
/* exFAT: Get a directory entry block */
/*------------------------------------*/

static fs_result_t load_xdir(        /* FR_INT_ERR: invalid entry block */
                         directory_t *dp /* Reading directory object pointing top of
                                    the entry block to load */
)
{
    fs_result_t res;
    unsigned i, sz_ent;
    uint8_t *dirb = dp->obj.fs->dirbuf; /* Pointer to the on-memory directory
                                        entry block 85+C0+C1s */

    /* Load file directory entry */
    res = move_window(dp->obj.fs, dp->sect);
    if (res != FR_OK)
        return res;
    if (dp->dir[XDIR_Type] != ET_FILEDIR)
        return FR_INT_ERR; /* Invalid order */
    memcpy(dirb + 0 * SZDIRE, dp->dir, SZDIRE);
    sz_ent = (dirb[XDIR_NumSec] + 1) * SZDIRE;
    if (sz_ent < 3 * SZDIRE || sz_ent > 19 * SZDIRE)
        return FR_INT_ERR;

    /* Load stream extension entry */
    res = dir_next(dp, 0);
    if (res == FR_NO_FILE)
        res = FR_INT_ERR; /* It cannot be */
    if (res != FR_OK)
        return res;
    res = move_window(dp->obj.fs, dp->sect);
    if (res != FR_OK)
        return res;
    if (dp->dir[XDIR_Type] != ET_STREAM)
        return FR_INT_ERR; /* Invalid order */
    memcpy(dirb + 1 * SZDIRE, dp->dir, SZDIRE);
    if (MAXDIRB(dirb[XDIR_NumName]) > sz_ent)
        return FR_INT_ERR;

    /* Load file name entries */
    i = 2 * SZDIRE; /* Name offset to load */
    do {
        res = dir_next(dp, 0);
        if (res == FR_NO_FILE)
            res = FR_INT_ERR; /* It cannot be */
        if (res != FR_OK)
            return res;
        res = move_window(dp->obj.fs, dp->sect);
        if (res != FR_OK)
            return res;
        if (dp->dir[XDIR_Type] != ET_FILENAME)
            return FR_INT_ERR; /* Invalid order */
        if (i < MAXDIRB(FF_MAX_LFN))
            memcpy(dirb + i, dp->dir, SZDIRE);
    } while ((i += SZDIRE) < sz_ent);

    /* Sanity check (do it for only accessible object) */
    if (i <= MAXDIRB(FF_MAX_LFN)) {
        if (xdir_sum(dirb) != ld_word(dirb + XDIR_SetSum))
            return FR_INT_ERR;
    }
    return FR_OK;
}

/*------------------------------------------------------------------*/
/* exFAT: Initialize object allocation info with loaded entry block */
/*------------------------------------------------------------------*/

static void init_alloc_info(filesystem_t *fs,   /* Filesystem object */
                            obj_id_t *obj /* Object allocation information to be initialized */
)
{
    obj->sclust = ld_dword(fs->dirbuf + XDIR_FstClus);   /* Start cluster */
    obj->objsize = ld_qword(fs->dirbuf + XDIR_FileSize); /* Size */
    obj->stat = fs->dirbuf[XDIR_GenFlags] & 2;           /* Allocation status */
    obj->n_frag = 0;                                     /* No last fragment info */
}

#if !FF_FS_READONLY || FF_FS_RPATH != 0
/*------------------------------------------------*/
/* exFAT: Load the object's directory entry block */
/*------------------------------------------------*/

static fs_result_t load_obj_xdir(
    directory_t *dp,           /* Blank directory object to be used to access containing
                          directory */
    const obj_id_t *obj /* Object with its containing directory information */
)
{
    fs_result_t res;

    /* Open object containing directory */
    dp->obj.fs = obj->fs;
    dp->obj.sclust = obj->c_scl;
    dp->obj.stat = (uint8_t)obj->c_size;
    dp->obj.objsize = obj->c_size & 0xFFFFFF00;
    dp->obj.n_frag = 0;
    dp->blk_ofs = obj->c_ofs;

    res = dir_sdi(dp, dp->blk_ofs); /* Goto object's entry block */
    if (res == FR_OK) {
        res = load_xdir(dp); /* Load the object's entry block */
    }
    return res;
}
#endif

#if !FF_FS_READONLY
/*----------------------------------------*/
/* exFAT: Store the directory entry block */
/*----------------------------------------*/

static fs_result_t store_xdir(directory_t *dp /* Pointer to the directory object */
)
{
    fs_result_t res;
    unsigned nent;
    uint8_t *dirb = dp->obj.fs->dirbuf; /* Pointer to the directory entry block 85+C0+C1s */

    /* Create set sum */
    st_word(dirb + XDIR_SetSum, xdir_sum(dirb));
    nent = dirb[XDIR_NumSec] + 1;

    /* Store the directory entry block to the directory */
    res = dir_sdi(dp, dp->blk_ofs);
    while (res == FR_OK) {
        res = move_window(dp->obj.fs, dp->sect);
        if (res != FR_OK)
            break;
        memcpy(dp->dir, dirb, SZDIRE);
        dp->obj.fs->wflag = 1;
        if (--nent == 0)
            break;
        dirb += SZDIRE;
        res = dir_next(dp, 0);
    }
    return (res == FR_OK || res == FR_DISK_ERR) ? res : FR_INT_ERR;
}

/*-------------------------------------------*/
/* exFAT: Create a new directory entry block */
/*-------------------------------------------*/

static void create_xdir(uint8_t *dirb,      /* Pointer to the directory entry block buffer */
                        const uint16_t *lfn /* Pointer to the object name */
)
{
    unsigned i;
    uint8_t nc1, nlen;
    uint16_t wc;

    /* Create file-directory and stream-extension entry */
    memset(dirb, 0, 2 * SZDIRE);
    dirb[0 * SZDIRE + XDIR_Type] = ET_FILEDIR;
    dirb[1 * SZDIRE + XDIR_Type] = ET_STREAM;

    /* Create file-name entries */
    i = SZDIRE * 2; /* Top of file_name entries */
    nlen = nc1 = 0;
    wc = 1;
    do {
        dirb[i++] = ET_FILENAME;
        dirb[i++] = 0;
        do { /* Fill name field */
            if (wc != 0 && (wc = lfn[nlen]) != 0)
                nlen++;            /* Get a character if exist */
            st_word(dirb + i, wc); /* Store it */
            i += 2;
        } while (i % SZDIRE != 0);
        nc1++;
    } while (lfn[nlen]); /* Fill next entry if any char follows */

    dirb[XDIR_NumName] = nlen;                     /* Set name length */
    dirb[XDIR_NumSec] = 1 + nc1;                   /* Set secondary count (C0 + C1s) */
    st_word(dirb + XDIR_NameHash, xname_sum(lfn)); /* Set name hash */
}

#endif /* !FF_FS_READONLY */

/*-----------------------------------------------------------------------*/
/* Read an object from the directory                                     */
/*-----------------------------------------------------------------------*/

#define DIR_READ_FILE(dp) dir_read(dp, 0)
#define DIR_READ_LABEL(dp) dir_read(dp, 1)

static fs_result_t dir_read(directory_t *dp, /* Pointer to the directory object */
                        int vol  /* Filtered by 0:file/directory or 1:volume label */
)
{
    fs_result_t res = FR_NO_FILE;
    filesystem_t *fs = dp->obj.fs;
    uint8_t attr, b;
    uint8_t ord = 0xFF, sum = 0xFF;

    while (dp->sect) {
        res = move_window(fs, dp->sect);
        if (res != FR_OK)
            break;
        b = dp->dir[DIR_Name]; /* Test for the entry type */
        if (b == 0) {
            res = FR_NO_FILE;
            break; /* Reached to end of the directory */
        }
        if (fs->fs_type == FS_EXFAT) { /* On the exFAT volume */
            if (FF_USE_LABEL && vol) {
                if (b == ET_VLABEL)
                    break; /* Volume label entry? */
            } else {
                if (b == ET_FILEDIR) {      /* Start of the file entry block? */
                    dp->blk_ofs = dp->dptr; /* Get location of the block */
                    res = load_xdir(dp);    /* Load the entry block */
                    if (res == FR_OK) {
                        dp->obj.attr = fs->dirbuf[XDIR_Attr] & AM_MASK; /* Get attribute */
                    }
                    break;
                }
            }
        } else {
            /* On the FAT/FAT32 volume */
            dp->obj.attr = attr = dp->dir[DIR_Attr] & AM_MASK; /* Get attribute */
            if (b == DDEM || b == '.' ||
                (int)((attr & ~AM_ARC) == AM_VOL) != vol) { /* An entry without valid data */
                ord = 0xFF;
            } else {
                if (attr == AM_LFN) { /* An LFN entry is found */
                    if (b & LLEF) {   /* Is it start of an LFN sequence? */
                        sum = dp->dir[LDIR_Chksum];
                        b &= (uint8_t)~LLEF;
                        ord = b;
                        dp->blk_ofs = dp->dptr;
                    }
                    /* Check LFN validity and capture it */
                    ord = (b == ord && sum == dp->dir[LDIR_Chksum] && pick_lfn(fs->lfnbuf, dp->dir))
                              ? ord - 1
                              : 0xFF;
                } else {                                       /* An SFN entry is found */
                    if (ord != 0 || sum != sum_sfn(dp->dir)) { /* Is there a valid LFN? */
                        dp->blk_ofs = 0xFFFFFFFF;              /* It has no LFN. */
                    }
                    break;
                }
            }
        }
        res = dir_next(dp, 0); /* Next entry */
        if (res != FR_OK)
            break;
    }

    if (res != FR_OK)
        dp->sect = 0; /* Terminate the read operation on error or EOT */
    return res;
}

/*-----------------------------------------------------------------------*/
/* Directory handling - Find an object in the directory                  */
/*-----------------------------------------------------------------------*/

static fs_result_t dir_find(        /* FR_OK(0):succeeded, !=0:error */
                        directory_t *dp /* Pointer to the directory object with the file name */
)
{
    fs_result_t res;
    filesystem_t *fs = dp->obj.fs;
    uint8_t c;
    uint8_t a, ord, sum;

    res = dir_sdi(dp, 0); /* Rewind directory object */
    if (res != FR_OK)
        return res;

    if (fs->fs_type == FS_EXFAT) { /* On the exFAT volume */
        uint8_t nc;
        unsigned di, ni;
        uint16_t hash = xname_sum(fs->lfnbuf); /* Hash value of the name to find */

        while ((res = DIR_READ_FILE(dp)) == FR_OK) { /* Read an item */
#if FF_MAX_LFN < 255
            if (fs->dirbuf[XDIR_NumName] > FF_MAX_LFN)
                continue; /* Skip comparison if inaccessible object name */
#endif
            if (ld_word(fs->dirbuf + XDIR_NameHash) != hash)
                continue; /* Skip comparison if hash mismatched */
            for (nc = fs->dirbuf[XDIR_NumName], di = SZDIRE * 2, ni = 0; nc;
                 nc--, di += 2, ni++) { /* Compare the name */
                if ((di % SZDIRE) == 0)
                    di += 2;
                if (ff_wtoupper(ld_word(fs->dirbuf + di)) != ff_wtoupper(fs->lfnbuf[ni]))
                    break;
            }
            if (nc == 0 && !fs->lfnbuf[ni])
                break; /* Name matched? */
        }
        return res;
    }

    /* On the FAT/FAT32 volume */
    ord = sum = 0xFF;
    dp->blk_ofs = 0xFFFFFFFF; /* Reset LFN sequence */
    do {
        res = move_window(fs, dp->sect);
        if (res != FR_OK)
            break;
        c = dp->dir[DIR_Name];
        if (c == 0) {
            res = FR_NO_FILE;
            break;
        }      /* Reached to end of table */
        dp->obj.attr = a = dp->dir[DIR_Attr] & AM_MASK;
        if (c == DDEM || ((a & AM_VOL) && a != AM_LFN)) { /* An entry without valid data */
            ord = 0xFF;
            dp->blk_ofs = 0xFFFFFFFF; /* Reset LFN sequence */
        } else {
            if (a == AM_LFN) { /* An LFN entry is found */
                if (!(dp->fn[NSFLAG] & NS_NOLFN)) {
                    if (c & LLEF) { /* Is it start of LFN sequence? */
                        sum = dp->dir[LDIR_Chksum];
                        c &= (uint8_t)~LLEF;
                        ord = c;                /* LFN start order */
                        dp->blk_ofs = dp->dptr; /* Start offset of LFN */
                    }
                    /* Check validity of the LFN entry and compare it with
                     * given name */
                    ord = (c == ord && sum == dp->dir[LDIR_Chksum] && cmp_lfn(fs->lfnbuf, dp->dir))
                              ? ord - 1
                              : 0xFF;
                }
            } else { /* An SFN entry is found */
                if (ord == 0 && sum == sum_sfn(dp->dir))
                    break; /* LFN matched? */
                if (!(dp->fn[NSFLAG] & NS_LOSS) && !memcmp(dp->dir, dp->fn, 11))
                    break; /* SFN matched? */
                ord = 0xFF;
                dp->blk_ofs = 0xFFFFFFFF; /* Reset LFN sequence */
            }
        }
        res = dir_next(dp, 0); /* Next entry */
    } while (res == FR_OK);

    return res;
}

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Register an object to the directory                                   */
/*-----------------------------------------------------------------------*/

static fs_result_t dir_register(        /* FR_OK:succeeded, FR_DENIED:no free entry or too many SFN
                                       collision, FR_DISK_ERR:disk error */
                            directory_t *dp /* Target directory with object name to be created */
)
{
    fs_result_t res;
    filesystem_t *fs = dp->obj.fs;
    unsigned n, len, n_ent;
    uint8_t sn[12], sum;

    if (dp->fn[NSFLAG] & (NS_DOT | NS_NONAME))
        return FR_INVALID_NAME; /* Check name validity */
    for (len = 0; fs->lfnbuf[len]; len++)
        ; /* Get lfn length */

    if (fs->fs_type == FS_EXFAT) {   /* On the exFAT volume */
        n_ent = (len + 14) / 15 + 2; /* Number of entries to allocate (85+C0+C1s) */
        res = dir_alloc(dp, n_ent);  /* Allocate directory entries */
        if (res != FR_OK)
            return res;
        dp->blk_ofs = dp->dptr - SZDIRE * (n_ent - 1); /* Set the allocated entry block offset */

        if (dp->obj.stat & 4) { /* Has the directory been stretched by new allocation? */
            dp->obj.stat &= ~4;
            res = fill_first_frag(&dp->obj); /* Fill the first fragment on the FAT if needed */
            if (res != FR_OK)
                return res;
            res = fill_last_frag(&dp->obj, dp->clust, 0xFFFFFFFF); /* Fill the last fragment on
                                                                      the FAT if needed */
            if (res != FR_OK)
                return res;
            if (dp->obj.sclust != 0) { /* Is it a sub-directory? */
                directory_t dj;

                res = load_obj_xdir(&dj, &dp->obj); /* Load the object status */
                if (res != FR_OK)
                    return res;
                dp->obj.objsize +=
                    (uint32_t)fs->csize * SS(fs); /* Increase the directory size by cluster size */
                st_qword(fs->dirbuf + XDIR_FileSize, dp->obj.objsize);
                st_qword(fs->dirbuf + XDIR_ValidFileSize, dp->obj.objsize);
                fs->dirbuf[XDIR_GenFlags] = dp->obj.stat | 1; /* Update the allocation status */
                res = store_xdir(&dj);                        /* Store the object status */
                if (res != FR_OK)
                    return res;
            }
        }

        create_xdir(fs->dirbuf, fs->lfnbuf); /* Create on-memory directory
                                                block to be written later */
        return FR_OK;
    }

    /* On the FAT/FAT32 volume */
    memcpy(sn, dp->fn, 12);
    if (sn[NSFLAG] & NS_LOSS) {    /* When LFN is out of 8.3 format, generate a
                                      numbered name */
        dp->fn[NSFLAG] = NS_NOLFN; /* Find only SFN */
        for (n = 1; n < 100; n++) {
            gen_numname(dp->fn, sn, fs->lfnbuf, n); /* Generate a numbered name */
            res = dir_find(dp); /* Check if the name collides with existing SFN */
            if (res != FR_OK)
                break;
        }
        if (n == 100)
            return FR_DENIED; /* Abort if too many collisions */
        if (res != FR_NO_FILE)
            return res; /* Abort if the result is other than 'not collided'
                         */
        dp->fn[NSFLAG] = sn[NSFLAG];
    }

    /* Create an SFN with/without LFNs. */
    n_ent = (sn[NSFLAG] & NS_LFN) ? (len + 12) / 13 + 1 : 1; /* Number of entries to allocate */
    res = dir_alloc(dp, n_ent);                              /* Allocate entries */
    if (res == FR_OK && --n_ent) {                           /* Set LFN entry if needed */
        res = dir_sdi(dp, dp->dptr - n_ent * SZDIRE);
        if (res == FR_OK) {
            sum = sum_sfn(dp->fn); /* Checksum value of the SFN tied to the LFN */
            do {                   /* Store LFN entries in bottom first */
                res = move_window(fs, dp->sect);
                if (res != FR_OK)
                    break;
                put_lfn(fs->lfnbuf, dp->dir, (uint8_t)n_ent, sum);
                fs->wflag = 1;
                res = dir_next(dp, 0); /* Next entry */
            } while (res == FR_OK && --n_ent);
        }
    }

    /* Set SFN entry */
    if (res == FR_OK) {
        res = move_window(fs, dp->sect);
        if (res == FR_OK) {
            memset(dp->dir, 0, SZDIRE);             /* Clean the entry */
            memcpy(dp->dir + DIR_Name, dp->fn, 11); /* Put SFN */
            dp->dir[DIR_NTres] = dp->fn[NSFLAG] & (NS_BODY | NS_EXT); /* Put NT flag */
            fs->wflag = 1;
        }
    }

    return res;
}

#endif /* !FF_FS_READONLY */

#if !FF_FS_READONLY && FF_FS_MINIMIZE == 0
/*-----------------------------------------------------------------------*/
/* Remove an object from the directory                                   */
/*-----------------------------------------------------------------------*/

static fs_result_t dir_remove(        /* FR_OK:Succeeded, FR_DISK_ERR:A disk error */
                          directory_t *dp /* Directory object pointing the entry to be removed */
)
{
    fs_result_t res;
    filesystem_t *fs = dp->obj.fs;
    uint32_t last = dp->dptr;

    res =
        (dp->blk_ofs == 0xFFFFFFFF) ? FR_OK : dir_sdi(dp, dp->blk_ofs); /* Goto top of the entry
                                                                           block if LFN is exist */
    if (res == FR_OK) {
        do {
            res = move_window(fs, dp->sect);
            if (res != FR_OK)
                break;
            if (fs->fs_type == FS_EXFAT) { /* On the exFAT volume */
                dp->dir[XDIR_Type] &= 0x7F;               /* Clear the entry InUse flag. */
            } else {                                      /* On the FAT/FAT32 volume */
                dp->dir[DIR_Name] = DDEM;                 /* Mark the entry 'deleted'. */
            }
            fs->wflag = 1;
            if (dp->dptr >= last)
                break;             /* If reached last entry then all entries of the
                                      object has been deleted. */
            res = dir_next(dp, 0); /* Next entry */
        } while (res == FR_OK);
        if (res == FR_NO_FILE)
            res = FR_INT_ERR;
    }

    return res;
}

#endif /* !FF_FS_READONLY && FF_FS_MINIMIZE == 0 */

#if FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2
/*-----------------------------------------------------------------------*/
/* Get file information from directory entry                             */
/*-----------------------------------------------------------------------*/

static void get_fileinfo(directory_t *dp,     /* Pointer to the directory object */
                         file_info_t *fno /* Pointer to the file information to be filled */
)
{
    unsigned si, di;
    uint8_t lcf;
    uint16_t wc, hs;
    filesystem_t *fs = dp->obj.fs;
    unsigned nw;

    fno->fname[0] = 0; /* Invaidate file info */
    if (dp->sect == 0)
        return; /* Exit if read pointer has reached end of directory */

    if (fs->fs_type == FS_EXFAT) { /* exFAT volume */
        unsigned nc = 0;

        si = SZDIRE * 2;
        di = 0; /* 1st C1 entry in the entry block */
        hs = 0;
        while (nc < fs->dirbuf[XDIR_NumName]) {
            if (si >= MAXDIRB(FF_MAX_LFN)) { /* Truncated directory block? */
                di = 0;
                break;
            }
            if ((si % SZDIRE) == 0)
                si += 2; /* Skip entry type field */
            wc = ld_word(fs->dirbuf + si);
            si += 2;
            nc++;                             /* Get a character */
            if (hs == 0 && IsSurrogate(wc)) { /* Is it a surrogate? */
                hs = wc;
                continue; /* Get low surrogate */
            }
            nw = put_utf((uint32_t)hs << 16 | wc, &fno->fname[di],
                         FF_LFN_BUF - di); /* Store it in API encoding */
            if (nw == 0) {                 /* Buffer overflow or wrong char? */
                di = 0;
                break;
            }
            di += nw;
            hs = 0;
        }
        if (hs != 0)
            di = 0; /* Broken surrogate pair? */
        if (di == 0)
            fno->fname[di++] = '\?'; /* Inaccessible object name? */
        fno->fname[di] = 0;          /* Terminate the name */
        fno->altname[0] = 0;         /* exFAT does not support SFN */

        fno->fattrib = fs->dirbuf[XDIR_Attr] & AM_MASKX; /* Attribute */
        fno->fsize = (fno->fattrib & AM_DIR) ? 0 : ld_qword(fs->dirbuf + XDIR_FileSize); /* Size */
        fno->ftime = ld_word(fs->dirbuf + XDIR_ModTime + 0);                             /* Time */
        fno->fdate = ld_word(fs->dirbuf + XDIR_ModTime + 2);                             /* Date */
        return;
    } else {
        /* FAT/FAT32 volume */
        if (dp->blk_ofs != 0xFFFFFFFF) { /* Get LFN if available */
            si = di = 0;
            hs = 0;
            while (fs->lfnbuf[si] != 0) {
                wc = fs->lfnbuf[si++];            /* Get an LFN character (UTF-16) */
                if (hs == 0 && IsSurrogate(wc)) { /* Is it a surrogate? */
                    hs = wc;
                    continue; /* Get low surrogate */
                }
                nw = put_utf((uint32_t)hs << 16 | wc, &fno->fname[di],
                             FF_LFN_BUF - di); /* Store it in API encoding */
                if (nw == 0) {                 /* Buffer overflow or wrong char? */
                    di = 0;
                    break;
                }
                di += nw;
                hs = 0;
            }
            if (hs != 0)
                di = 0;         /* Broken surrogate pair? */
            fno->fname[di] = 0; /* Terminate the LFN (null string means LFN is invalid) */
        }
    }

    si = di = 0;
    while (si < 11) {       /* Get SFN from SFN entry */
        wc = dp->dir[si++]; /* Get a char */
        if (wc == ' ')
            continue; /* Skip padding spaces */
        if (wc == RDDEM)
            wc = DDEM; /* Restore replaced DDEM character */
        if (si == 9 && di < FF_SFN_BUF)
            fno->altname[di++] = '.'; /* Insert a . if extension is exist */

        /* Unicode output */
        if (dbc_1st((uint8_t)wc) && si != 8 && si != 11 &&
            dbc_2nd(dp->dir[si])) { /* Make a DBC if needed */
            wc = wc << 8 | dp->dir[si++];
        }
        wc = ff_oem2uni(wc, CODEPAGE); /* ANSI/OEM -> Unicode */
        if (wc == 0) { /* Wrong char in the current code page? */
            di = 0;
            break;
        }
        nw = put_utf(wc, &fno->altname[di], FF_SFN_BUF - di); /* Store it in API encoding */
        if (nw == 0) {                                        /* Buffer overflow? */
            di = 0;
            break;
        }
        di += nw;
    }
    fno->altname[di] = 0; /* Terminate the SFN  (null string means SFN is invalid) */

    if (fno->fname[0] == 0) { /* If LFN is invalid, altname[] needs to be copied to fname[] */
        if (di == 0) {        /* If LFN and SFN both are invalid, this object is
                                 inaccessible */
            fno->fname[di++] = '\?';
        } else {
            for (si = di = 0, lcf = NS_BODY; fno->altname[si];
                 si++, di++) { /* Copy altname[] to fname[] with case information */
                wc = (uint16_t)fno->altname[si];
                if (wc == '.')
                    lcf = NS_EXT;
                if (IsUpper(wc) && (dp->dir[DIR_NTres] & lcf))
                    wc += 0x20;
                fno->fname[di] = (char)wc;
            }
        }
        fno->fname[di] = 0; /* Terminate the LFN */
        if (!dp->dir[DIR_NTres])
            fno->altname[0] = 0; /* Altname is not needed if neither LFN nor
                                    case info is exist. */
    }

    fno->fattrib = dp->dir[DIR_Attr] & AM_MASK;      /* Attribute */
    fno->fsize = ld_dword(dp->dir + DIR_FileSize);   /* Size */
    fno->ftime = ld_word(dp->dir + DIR_ModTime + 0); /* Time */
    fno->fdate = ld_word(dp->dir + DIR_ModTime + 2); /* Date */
}

#endif /* FF_FS_MINIMIZE <= 1 || FF_FS_RPATH >= 2 */

#if FF_USE_FIND && FF_FS_MINIMIZE <= 1
/*-----------------------------------------------------------------------*/
/* Pattern matching                                                      */
/*-----------------------------------------------------------------------*/

#define FIND_RECURS 4 /* Maximum number of wildcard terms in the pattern to limit recursion */

static uint32_t get_achar(                  /* Get a character and advance ptr */
                       const char **ptr /* Pointer to pointer to the
                                            ANSI/OEM or Unicode string */
)
{
    uint32_t chr;

    /* Unicode input */
    chr = tchar2uni(ptr);
    if (chr == 0xFFFFFFFF)
        chr = 0; /* Wrong UTF encoding is recognized as end of the string */

    chr = ff_wtoupper(chr);
    return chr;
}

static int pattern_match(                  /* 0:mismatched, 1:matched */
                         const char *pat, /* Matching pattern */
                         const char *nam, /* String to be tested */
                         unsigned skip,        /* Number of pre-skip chars (number of
                                              ?s, b8:infinite (* specified)) */
                         unsigned recur        /* Recursion count */
)
{
    const char *pptr;
    const char *nptr;
    uint32_t pchr, nchr;
    unsigned sk;

    while ((skip & 0xFF) != 0) { /* Pre-skip name chars */
        if (!get_achar(&nam))
            return 0; /* Branch mismatched if less name chars */
        skip--;
    }
    if (*pat == 0 && skip)
        return 1; /* Matched? (short circuit) */

    do {
        pptr = pat;
        nptr = nam; /* Top of pattern and name to match */
        for (;;) {
            if (*pptr == '\?' || *pptr == '*') { /* Wildcard term? */
                if (recur == 0)
                    return 0; /* Too many wildcard terms? */
                sk = 0;
                do { /* Analyze the wildcard term */
                    if (*pptr++ == '\?') {
                        sk++;
                    } else {
                        sk |= 0x100;
                    }
                } while (*pptr == '\?' || *pptr == '*');
                if (pattern_match(pptr, nptr, sk, recur - 1))
                    return 1; /* Test new branch (recursive call) */
                nchr = *nptr;
                break; /* Branch mismatched */
            }
            pchr = get_achar(&pptr); /* Get a pattern char */
            nchr = get_achar(&nptr); /* Get a name char */
            if (pchr != nchr)
                break; /* Branch mismatched? */
            if (pchr == 0)
                return 1; /* Branch matched? (matched at end of both strings)
                           */
        }
        get_achar(&nam);    /* nam++ */
    } while (skip && nchr); /* Retry until end of name if infinite search is specified */

    return 0;
}

#endif /* FF_USE_FIND && FF_FS_MINIMIZE <= 1 */

/*-----------------------------------------------------------------------*/
/* Pick a top segment and create the object name in directory form       */
/*-----------------------------------------------------------------------*/

/* FR_OK: successful, FR_INVALID_NAME: could not create */
static fs_result_t create_name(directory_t *dp,   /* Pointer to the directory object */
                               const char **path) /* Pointer to pointer to the segment in the path string */
{
    uint8_t b, cf;
    uint16_t wc;
    uint16_t *lfn;
    const char *p;
    uint32_t uc;
    unsigned i, ni, si, di;

    /* Create LFN into LFN working buffer */
    p = *path;
    lfn = dp->obj.fs->lfnbuf;
    di = 0;
    for (;;) {
        uc = tchar2uni(&p); /* Get a character */
        if (uc == 0xFFFFFFFF)
            return FR_INVALID_NAME; /* Invalid code or UTF decode error */
        if (uc >= 0x10000)
            lfn[di++] = (uint16_t)(uc >> 16); /* Store high surrogate if needed */
        wc = (uint16_t)uc;
        if (wc < ' ' || IsSeparator(wc))
            break; /* Break if end of the path or a separator is found */
        if (wc < 0x80 && strchr("*:<>|\"\?\x7F", (int)wc))
            return FR_INVALID_NAME; /* Reject illegal characters for LFN */
        if (di >= FF_MAX_LFN)
            return FR_INVALID_NAME; /* Reject too long name */
        lfn[di++] = wc;             /* Store the Unicode character */
    }
    if (wc < ' ') {   /* Stopped at end of the path? */
        cf = NS_LAST; /* Last segment */
    } else {          /* Stopped at a separator */
        while (IsSeparator(*p))
            p++; /* Skip duplicated separators if exist */
        cf = 0;  /* Next segment may follow */
        if (IsTerminator(*p))
            cf = NS_LAST; /* Ignore terminating separator */
    }
    *path = p; /* Return pointer to the next segment */

#if FF_FS_RPATH != 0
    if ((di == 1 && lfn[di - 1] == '.') ||
        (di == 2 && lfn[di - 1] == '.' && lfn[di - 2] == '.')) { /* Is this segment a dot name? */
        lfn[di] = 0;
        for (i = 0; i < 11; i++) { /* Create dot name for SFN entry */
            dp->fn[i] = (i < di) ? '.' : ' ';
        }
        dp->fn[i] = cf | NS_DOT; /* This is a dot entry */
        return FR_OK;
    }
#endif
    while (di) { /* Snip off trailing spaces and dots if exist */
        wc = lfn[di - 1];
        if (wc != ' ' && wc != '.')
            break;
        di--;
    }
    lfn[di] = 0; /* LFN is created into the working buffer */
    if (di == 0)
        return FR_INVALID_NAME; /* Reject null name */

    /* Create SFN in directory form */
    for (si = 0; lfn[si] == ' '; si++)
        ; /* Remove leading spaces */
    if (si > 0 || lfn[si] == '.')
        cf |= NS_LOSS | NS_LFN; /* Is there any leading space or dot? */
    while (di > 0 && lfn[di - 1] != '.')
        di--; /* Find last dot (di<=si: no extension) */

    memset(dp->fn, ' ', 11);
    i = b = 0;
    ni = 8;
    for (;;) {
        wc = lfn[si++]; /* Get an LFN character */
        if (wc == 0)
            break;                                  /* Break on end of the LFN */
        if (wc == ' ' || (wc == '.' && si != di)) { /* Remove embedded spaces and dots */
            cf |= NS_LOSS | NS_LFN;
            continue;
        }

        if (i >= ni || si == di) { /* End of field? */
            if (ni == 11) {        /* Name extension overflow? */
                cf |= NS_LOSS | NS_LFN;
                break;
            }
            if (si != di)
                cf |= NS_LOSS | NS_LFN; /* Name body overflow? */
            if (si > di)
                break; /* No name extension? */
            si = di;
            i = 8;
            ni = 11;
            b <<= 2; /* Enter name extension */
            continue;
        }

        if (wc > '~' || wc == 0 || strchr("+,;=[]", (int)wc)) {
            /* Replace illegal characters for SFN */
            wc = '_';
            cf |= NS_LOSS | NS_LFN; /* Lossy conversion */
        } else {
            if (IsUpper(wc)) { /* ASCII upper case? */
                b |= 2;
            }
            if (IsLower(wc)) { /* ASCII lower case? */
                b |= 1;
                wc -= 0x20;
            }
        }
        dp->fn[i++] = (uint8_t)wc;
    }

    if (dp->fn[0] == DDEM)
        dp->fn[0] = RDDEM; /* If the first character collides with DDEM,
                              replace it with RDDEM */

    if (ni == 8)
        b <<= 2; /* Shift capital flags if no extension */
    if ((b & 0x0C) == 0x0C || (b & 0x03) == 0x03)
        cf |= NS_LFN;     /* LFN entry needs to be created if composite capitals */
    if (!(cf & NS_LFN)) { /* When LFN is in 8.3 format without extended
                             character, NT flags are created */
        if (b & 0x01)
            cf |= NS_EXT; /* NT flag (Extension has small capital letters
                             only) */
        if (b & 0x04)
            cf |= NS_BODY; /* NT flag (Body has small capital letters only) */
    }

    dp->fn[NSFLAG] = cf; /* SFN is created into dp->fn[] */

    return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Follow a file path                                                    */
/*-----------------------------------------------------------------------*/

static fs_result_t follow_path(                  /* FR_OK(0): successful, !=0: error code */
                           directory_t *dp,          /* Directory object to return last
                                                directory and found object */
                           const char *path /* Full-path string to find a
                                                file or directory */
)
{
    fs_result_t res;
    uint8_t ns;
    filesystem_t *fs = dp->obj.fs;

#if FF_FS_RPATH != 0
    if (!IsSeparator(*path) &&
        (FF_STR_VOLUME_ID != 2 || !IsTerminator(*path))) { /* Without heading separator */
        dp->obj.sclust = fs->cdir;                         /* Start at the current directory */
    } else
#endif
    { /* With heading separator */
        while (IsSeparator(*path))
            path++;         /* Strip separators */
        dp->obj.sclust = 0; /* Start from the root directory */
    }
    dp->obj.n_frag = 0; /* Invalidate last fragment counter of the object */
#if FF_FS_RPATH != 0
    if (fs->fs_type == FS_EXFAT &&
        dp->obj.sclust) { /* exFAT: Retrieve the sub-directory's status */
        directory_t dj;

        dp->obj.c_scl = fs->cdc_scl;
        dp->obj.c_size = fs->cdc_size;
        dp->obj.c_ofs = fs->cdc_ofs;
        res = load_obj_xdir(&dj, &dp->obj);
        if (res != FR_OK)
            return res;
        dp->obj.objsize = ld_dword(fs->dirbuf + XDIR_FileSize);
        dp->obj.stat = fs->dirbuf[XDIR_GenFlags] & 2;
    }
#endif

    if ((unsigned)*path < ' ') { /* Null path name is the origin directory itself */
        dp->fn[NSFLAG] = NS_NONAME;
        res = dir_sdi(dp, 0);

    } else { /* Follow path */
        for (;;) {
            res = create_name(dp, &path); /* Get a segment name of the path */
            if (res != FR_OK)
                break;
            res = dir_find(dp); /* Find an object with the segment name */
            ns = dp->fn[NSFLAG];
            if (res != FR_OK) {                         /* Failed to find the object */
                if (res == FR_NO_FILE) {                /* Object is not found */
                    if (FF_FS_RPATH && (ns & NS_DOT)) { /* If dot entry is not exist, stay
                                                           there */
                        if (!(ns & NS_LAST))
                            continue; /* Continue to follow if not last
                                         segment */
                        dp->fn[NSFLAG] = NS_NONAME;
                        res = FR_OK;
                    } else { /* Could not find the object */
                        if (!(ns & NS_LAST))
                            res = FR_NO_PATH; /* Adjust error code if not
                                                 last segment */
                    }
                }
                break;
            }
            if (ns & NS_LAST)
                break; /* Last segment matched. Function completed. */
            /* Get into the sub-directory */
            if (!(dp->obj.attr & AM_DIR)) { /* It is not a sub-directory and
                                               cannot follow */
                res = FR_NO_PATH;
                break;
            }
            if (fs->fs_type == FS_EXFAT) { /* Save containing directory
                                              information for next dir */
                dp->obj.c_scl = dp->obj.sclust;
                dp->obj.c_size = ((uint32_t)dp->obj.objsize & 0xFFFFFF00) | dp->obj.stat;
                dp->obj.c_ofs = dp->blk_ofs;
                init_alloc_info(fs, &dp->obj); /* Open next directory */
            } else {
                dp->obj.sclust =
                    ld_clust(fs, fs->win + dp->dptr % SS(fs)); /* Open next directory */
            }
        }
    }

    return res;
}

/*-----------------------------------------------------------------------*/
/* Get logical drive number from path name                               */
/*-----------------------------------------------------------------------*/

/* Returns logical drive number (-1:invalid drive number or null pointer) */
static int get_ldnumber(const char **path) /* Pointer to pointer to the path name */
{
    const char *tp;
    const char *tt;
    char tc;
    int i;
    int vol = -1;
#if FF_STR_VOLUME_ID /* Find string volume ID */
    const char *sp;
    char c;
#endif

    tt = tp = *path;
    if (!tp)
        return vol; /* Invalid path name? */
    do {            /* Find a colon in the path */
        tc = *tt++;
    } while (!IsTerminator(tc) && tc != ':');

    if (tc == ':') { /* DOS/Windows style volume ID? */
        i = DISK_VOLUMES;
        if (IsDigit(*tp) && tp + 2 == tt) { /* Is there a numeric volume ID + colon? */
            i = (int)*tp - '0';             /* Get the LD number */
        }
#if FF_STR_VOLUME_ID == 1 /* Arbitrary string is enabled */
        else {
            i = 0;
            do {
                sp = disk_name[i];
                tp = *path; /* This string volume ID and path name */
                do {        /* Compare the volume ID with path name */
                    c = *sp++;
                    tc = *tp++;
                    if (IsLower(c))
                        c -= 0x20;
                    if (IsLower(tc))
                        tc -= 0x20;
                } while (c && (char)c == tc);
            } while ((c || tp != tt) &&
                     ++i < DISK_VOLUMES); /* Repeat for each id until pattern match */
        }
#endif
        if (i < DISK_VOLUMES) { /* If a volume ID is found, get the drive
                                 number and strip it */
            vol = i;          /* Drive number */
            *path = tt;       /* Snip the drive prefix off */
        }
        return vol;
    }
#if FF_STR_VOLUME_ID == 2 /* Unix style volume ID is enabled */
    if (*tp == '/') {     /* Is there a volume ID? */
        while (*(tp + 1) == '/')
            tp++; /* Skip duplicated separator */
        i = 0;
        do {
            tt = tp;
            sp = disk_name[i]; /* Path name and this string volume ID */
            do {               /* Compare the volume ID with path name */
                c = *sp++;
                tc = *(++tt);
                if (IsLower(c))
                    c -= 0x20;
                if (IsLower(tc))
                    tc -= 0x20;
            } while (c && (char)c == tc);
        } while ((c || (tc != '/' && !IsTerminator(tc))) &&
                 ++i < DISK_VOLUMES); /* Repeat for each ID until pattern match */
        if (i < DISK_VOLUMES) {       /* If a volume ID is found, get the drive
                                       number and strip it */
            vol = i;                /* Drive number */
            *path = tt;             /* Snip the drive prefix off */
        }
        return vol;
    }
#endif
    /* No drive prefix is found */
#if FF_FS_RPATH != 0
    vol = CurrVol; /* Default drive is current drive */
#else
    vol = 0; /* Default drive is 0 */
#endif
    return vol; /* Return the default drive */
}

/*-----------------------------------------------------------------------*/
/* GPT support functions                                                 */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Load a sector and check if it is an FAT VBR                           */
/*-----------------------------------------------------------------------*/

/* Check what the sector is */

static unsigned check_fs(           /* 0:FAT/FAT32 VBR, 1:exFAT VBR, 2:Not FAT and valid BS, 3:Not FAT
                                   and invalid BS, 4:Disk error */
                     filesystem_t *fs, /* Filesystem object */
                     fs_lba_t sect /* Sector to load and check if it is an FAT-VBR or not */
)
{
    uint16_t w, sign;
    uint8_t b;

    fs->wflag = 0;
    fs->winsect = (fs_lba_t)0 - 1; /* Invaidate window */
    if (move_window(fs, sect) != FR_OK)
        return 4; /* Load the boot sector */
    sign = ld_word(fs->win + BS_55AA);
    if (sign == 0xAA55 && !memcmp(fs->win + BS_JmpBoot,
                                  "\xEB\x76\x90"
                                  "EXFAT   ",
                                  11)) {
        return 1; /* It is an exFAT VBR */
    }
    b = fs->win[BS_JmpBoot];
    if (b == 0xEB || b == 0xE9 || b == 0xE8) { /* Valid JumpBoot code? (short jump, near jump or
                                                  near call) */
        if (sign == 0xAA55 && !memcmp(fs->win + BS_FilSysType32, "FAT32   ", 8)) {
            return 0; /* It is an FAT32 VBR */
        }
        /* FAT volumes formatted with early MS-DOS lack BS_55AA and
         * BS_FilSysType, so FAT VBR needs to be identified without them. */
        w = ld_word(fs->win + BPB_BytsPerSec);
        b = fs->win[BPB_SecPerClus];
        if ((w & (w - 1)) == 0 && w >= FF_MIN_SS &&
            w <= FF_MAX_SS                  /* Properness of sector size (512-4096 and 2^n) */
            && b != 0 && (b & (b - 1)) == 0 /* Properness of cluster size (2^n) */
            && ld_word(fs->win + BPB_RsvdSecCnt) != 0 /* Properness of reserved sectors (MNBZ) */
            && (unsigned)fs->win[BPB_NumFATs] - 1 <= 1    /* Properness of FATs (1 or 2) */
            && ld_word(fs->win + BPB_RootEntCnt) != 0 /* Properness of root dir entries (MNBZ) */
            &&
            (ld_word(fs->win + BPB_TotSec16) >= 128 ||
             ld_dword(fs->win + BPB_TotSec32) >= 0x10000) /* Properness of volume sectors (>=128) */
            && ld_word(fs->win + BPB_FATSz16) != 0) {     /* Properness of FAT size (MNBZ) */
            return 0;                                     /* It can be presumed an FAT VBR */
        }
    }
    return sign == 0xAA55 ? 2 : 3; /* Not an FAT VBR (valid or invalid BS) */
}

/* Find an FAT volume */
/* (It supports only generic partitioning rules, MBR, GPT and SFD) */

static unsigned find_volume(           /* Returns BS status found in the hosting drive */
                        filesystem_t *fs, /* Filesystem object */
                        unsigned part  /* Partition to fined = 0:find as SFD and partitions,
                                      >0:forced  partition number */
)
{
    unsigned fmt, i;
    uint32_t mbr_pt[4];

    fmt = check_fs(fs, 0); /* Load sector 0 and check if it is an FAT VBR as SFD format */
    if (fmt != 2 && (fmt >= 3 || part == 0))
        return fmt; /* Returns if it is an FAT VBR as auto scan, not a BS or
                       disk error */

        /* Sector 0 is not an FAT VBR or forced partition number wants a
         * partition */

    for (i = 0; i < 4; i++) { /* Load partition offset in the MBR */
        mbr_pt[i] = ld_dword(fs->win + MBR_Table + i * SZ_PTE + PTE_StLba);
    }
    i = part ? part - 1 : 0;                           /* Table index to find first */
    do {                                               /* Find an FAT volume */
        fmt = mbr_pt[i] ? check_fs(fs, mbr_pt[i]) : 3; /* Check if the partition is FAT */
    } while (part == 0 && fmt >= 2 && ++i < 4);
    return fmt;
}

/*-----------------------------------------------------------------------*/
/* Determine logical drive number and mount the volume if needed         */
/*-----------------------------------------------------------------------*/

static fs_result_t mount_volume(                    /* FR_OK(0): successful, !=0: an error occurred */
                            const char **path, /* Pointer to pointer to the path name
                                                   (drive number) */
                            filesystem_t **rfs, /* Pointer to pointer to the found filesystem object */
                            uint8_t mode    /* Desiered access mode to check write protection */
)
{
    int vol;
    filesystem_t *fs;
    media_status_t stat;
    fs_lba_t bsect;
    uint32_t tsect, sysect, fasize, nclst, szbfat;
    uint16_t nrsv;
    unsigned fmt;

    /* Get logical drive number */
    *rfs = 0;
    vol = get_ldnumber(path);
    if (vol < 0)
        return FR_INVALID_DRIVE;

    /* Check if the filesystem object is valid or not */
    fs = FatFs[vol]; /* Get pointer to the filesystem object */
    if (!fs)
        return FR_NOT_ENABLED; /* Is the filesystem object available? */
#if FF_FS_REENTRANT
    if (!lock_volume(fs, 1))
        return FR_TIMEOUT; /* Lock the volume, and system if needed */
#endif
    *rfs = fs; /* Return pointer to the filesystem object */

    mode &= (uint8_t)~FA_READ; /* Desired access mode, write access or not */
    if (fs->fs_type != 0) { /* If the volume has been mounted */
        stat = disk_status(fs->pdrv);
        if (!(stat & MEDIA_NOINIT)) { /* and the physical drive is kept initialized */
            if (!FF_FS_READONLY && mode &&
                (stat & MEDIA_PROTECT)) { /* Check write protection if needed */
                return FR_WRITE_PROTECTED;
            }
            return FR_OK; /* The filesystem object is already valid */
        }
    }

    /* The filesystem object is not valid. */
    /* Following code attempts to mount the volume. (find an FAT volume,
     * analyze the BPB and initialize the filesystem object) */

    fs->fs_type = 0;                  /* Invalidate the filesystem object */
    stat = disk_initialize(fs->pdrv); /* Initialize the volume hosting physical drive */
    if (stat & MEDIA_NOINIT) {          /* Check if the initialization succeeded */
        return FR_NOT_READY;          /* Failed to initialize due to no medium or hard
                                         error */
    }
    if (!FF_FS_READONLY && mode &&
        (stat & MEDIA_PROTECT)) { /* Check disk write protection if needed */
        return FR_WRITE_PROTECTED;
    }
#if FF_MAX_SS != FF_MIN_SS /* Get sector size (multiple sector size cfg only) */
    if (disk_ioctl(fs->pdrv, GET_SECTOR_SIZE, &SS(fs)) != DISK_OK)
        return FR_DISK_ERR;
    if (SS(fs) > FF_MAX_SS || SS(fs) < FF_MIN_SS || (SS(fs) & (SS(fs) - 1)))
        return FR_DISK_ERR;
#endif

    /* Find an FAT volume on the hosting drive */
    fmt = find_volume(fs, LD2PT(vol));
    if (fmt == 4)
        return FR_DISK_ERR; /* An error occurred in the disk I/O layer */
    if (fmt >= 2)
        return FR_NO_FILESYSTEM; /* No FAT volume is found */
    bsect = fs->winsect;         /* Volume offset in the hosting physical drive */

    /* An FAT volume is found (bsect). Following code initializes the
     * filesystem object */

    if (fmt == 1) {
        uint64_t maxlba;
        uint32_t so, cv, bcl, i;

        for (i = BPB_ZeroedEx; i < BPB_ZeroedEx + 53 && fs->win[i] == 0; i++)
            ; /* Check zero filler */
        if (i < BPB_ZeroedEx + 53)
            return FR_NO_FILESYSTEM;

        if (ld_word(fs->win + BPB_FSVerEx) != 0x100)
            return FR_NO_FILESYSTEM; /* Check exFAT version (must be
                                        version 1.0) */

        if (1 << fs->win[BPB_BytsPerSecEx] != SS(fs)) { /* (BPB_BytsPerSecEx must be equal to the
                                                           physical sector size) */
            return FR_NO_FILESYSTEM;
        }

        maxlba = ld_qword(fs->win + BPB_TotSecEx) + bsect; /* Last LBA of the volume + 1 */
        if (maxlba >= 0x100000000) {
            // This volume cannot be accessed in 32-bit LBA.
            // No LBA64, sorry.
            return FR_NO_FILESYSTEM;
        }
        fs->fsize = ld_dword(fs->win + BPB_FatSzEx); /* Number of sectors per FAT */

        fs->n_fats = fs->win[BPB_NumFATsEx]; /* Number of FATs */
        if (fs->n_fats != 1)
            return FR_NO_FILESYSTEM; /* (Supports only 1 FAT) */

        fs->csize = 1 << fs->win[BPB_SecPerClusEx]; /* Cluster size */
        if (fs->csize == 0)
            return FR_NO_FILESYSTEM; /* (Must be 1..32768 sectors) */

        nclst = ld_dword(fs->win + BPB_NumClusEx); /* Number of clusters */
        if (nclst > MAX_EXFAT)
            return FR_NO_FILESYSTEM; /* (Too many clusters) */
        fs->n_fatent = nclst + 2;

        /* Boundaries and Limits */
        fs->volbase = bsect;
        fs->database = bsect + ld_dword(fs->win + BPB_DataOfsEx);
        fs->fatbase = bsect + ld_dword(fs->win + BPB_FatOfsEx);
        if (maxlba < (uint64_t)fs->database + nclst * fs->csize)
            return FR_NO_FILESYSTEM; /* (Volume size must not be smaller than
                                        the size required) */
        fs->dirbase = ld_dword(fs->win + BPB_RootClusEx);

        /* Get bitmap location and check if it is contiguous (implementation
         * assumption) */
        so = i = 0;
        for (;;) { /* Find the bitmap entry in the root directory (in only
                      first cluster) */
            if (i == 0) {
                if (so >= fs->csize)
                    return FR_NO_FILESYSTEM; /* Not found? */
                if (move_window(fs, clst2sect(fs, (uint32_t)fs->dirbase) + so) != FR_OK)
                    return FR_DISK_ERR;
                so++;
            }
            if (fs->win[i] == ET_BITMAP)
                break;                 /* Is it a bitmap entry? */
            i = (i + SZDIRE) % SS(fs); /* Next entry */
        }
        bcl = ld_dword(fs->win + i + 20); /* Bitmap cluster */
        if (bcl < 2 || bcl >= fs->n_fatent)
            return FR_NO_FILESYSTEM;                        /* (Wrong cluster#) */
        fs->bitbase = fs->database + fs->csize * (bcl - 2); /* Bitmap sector */
        for (;;) {                                          /* Check if bitmap is contiguous */
            if (move_window(fs, fs->fatbase + bcl / (SS(fs) / 4)) != FR_OK)
                return FR_DISK_ERR;
            cv = ld_dword(fs->win + bcl % (SS(fs) / 4) * 4);
            if (cv == 0xFFFFFFFF)
                break; /* Last link? */
            if (cv != ++bcl)
                return FR_NO_FILESYSTEM; /* Fragmented bitmap? */
        }

#if !FF_FS_READONLY
        fs->last_clst = fs->free_clst = 0xFFFFFFFF; /* Initialize cluster allocation information */
#endif
        fmt = FS_EXFAT; /* FAT sub-type */
    } else {
        if (ld_word(fs->win + BPB_BytsPerSec) != SS(fs))
            return FR_NO_FILESYSTEM; /* (BPB_BytsPerSec must be equal to the
                                      * physical sector size)
                                      */

        fasize = ld_word(fs->win + BPB_FATSz16); /* Number of sectors per FAT */
        if (fasize == 0)
            fasize = ld_dword(fs->win + BPB_FATSz32);
        fs->fsize = fasize;

        fs->n_fats = fs->win[BPB_NumFATs]; /* Number of FATs */
        if (fs->n_fats != 1 && fs->n_fats != 2)
            return FR_NO_FILESYSTEM; /* (Must be 1 or 2) */
        fasize *= fs->n_fats;        /* Number of sectors for FAT area */

        fs->csize = fs->win[BPB_SecPerClus]; /* Cluster size */
        if (fs->csize == 0 || (fs->csize & (fs->csize - 1)))
            return FR_NO_FILESYSTEM; /* (Must be power of 2) */

        fs->n_rootdir = ld_word(fs->win + BPB_RootEntCnt); /* Number of root directory entries */
        if (fs->n_rootdir % (SS(fs) / SZDIRE))
            return FR_NO_FILESYSTEM; /* (Must be sector aligned) */

        tsect = ld_word(fs->win + BPB_TotSec16); /* Number of sectors on the volume */
        if (tsect == 0)
            tsect = ld_dword(fs->win + BPB_TotSec32);

        nrsv = ld_word(fs->win + BPB_RsvdSecCnt); /* Number of reserved sectors */
        if (nrsv == 0)
            return FR_NO_FILESYSTEM; /* (Must not be 0) */

        /* Determine the FAT sub type */
        sysect = nrsv + fasize + fs->n_rootdir / (SS(fs) / SZDIRE); /* RSV + FAT + DIR */
        if (tsect < sysect)
            return FR_NO_FILESYSTEM;          /* (Invalid volume size) */
        nclst = (tsect - sysect) / fs->csize; /* Number of clusters */
        if (nclst == 0)
            return FR_NO_FILESYSTEM; /* (Invalid volume size) */
        fmt = 0;
        if (nclst <= MAX_FAT32)
            fmt = FS_FAT32;
        if (nclst <= MAX_FAT16)
            fmt = FS_FAT16;
        if (nclst <= MAX_FAT12)
            fmt = FS_FAT12;
        if (fmt == 0)
            return FR_NO_FILESYSTEM;

        /* Boundaries and Limits */
        fs->n_fatent = nclst + 2;      /* Number of FAT entries */
        fs->volbase = bsect;           /* Volume start sector */
        fs->fatbase = bsect + nrsv;    /* FAT start sector */
        fs->database = bsect + sysect; /* Data start sector */
        if (fmt == FS_FAT32) {
            if (ld_word(fs->win + BPB_FSVer32) != 0)
                return FR_NO_FILESYSTEM; /* (Must be FAT32 revision 0.0) */
            if (fs->n_rootdir != 0)
                return FR_NO_FILESYSTEM;                      /* (BPB_RootEntCnt must be 0) */
            fs->dirbase = ld_dword(fs->win + BPB_RootClus32); /* Root directory start cluster */
            szbfat = fs->n_fatent * 4;                        /* (Needed FAT size) */
        } else {
            if (fs->n_rootdir == 0)
                return FR_NO_FILESYSTEM;        /* (BPB_RootEntCnt must not be 0) */
            fs->dirbase = fs->fatbase + fasize; /* Root directory start sector */
            szbfat = (fmt == FS_FAT16) ?        /* (Needed FAT size) */
                         fs->n_fatent * 2
                                       : fs->n_fatent * 3 / 2 + (fs->n_fatent & 1);
        }
        if (fs->fsize < (szbfat + (SS(fs) - 1)) / SS(fs))
            return FR_NO_FILESYSTEM; /* (BPB_FATSz must not be less than the
                                        size needed) */

#if !FF_FS_READONLY
        /* Get FSInfo if available */
        fs->last_clst = fs->free_clst = 0xFFFFFFFF; /* Initialize cluster allocation information */
        fs->fsi_flag = 0x80;
#if (FF_FS_NOFSINFO & 3) != 3
        if (fmt == FS_FAT32 /* Allow to update FSInfo only if BPB_FSInfo32 ==
                               1 */
            && ld_word(fs->win + BPB_FSInfo32) == 1 && move_window(fs, bsect + 1) == FR_OK) {
            fs->fsi_flag = 0;
            if (ld_word(fs->win + BS_55AA) == 0xAA55 /* Load FSInfo data if available */
                && ld_dword(fs->win + FSI_LeadSig) == 0x41615252 &&
                ld_dword(fs->win + FSI_StrucSig) == 0x61417272) {
#if (FF_FS_NOFSINFO & 1) == 0
                fs->free_clst = ld_dword(fs->win + FSI_Free_Count);
#endif
#if (FF_FS_NOFSINFO & 2) == 0
                fs->last_clst = ld_dword(fs->win + FSI_Nxt_Free);
#endif
            }
        }
#endif /* (FF_FS_NOFSINFO & 3) != 3 */
#endif /* !FF_FS_READONLY */
    }

    fs->fs_type = (uint8_t)fmt; /* FAT sub-type (the filesystem object gets valid) */
    fs->id = ++Fsid;         /* Volume mount ID */
#if FF_USE_LFN == 1
    fs->lfnbuf = LfnBuf; /* Static LFN working buffer */
    fs->dirbuf = DirBuf; /* Static directory block scratchpad buuffer */
#endif
#if FF_FS_RPATH != 0
    fs->cdir = 0; /* Initialize current directory */
#endif
#if FF_FS_LOCK /* Clear file lock semaphores */
    clear_share(fs);
#endif
    return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Check if the file/directory object is valid or not                    */
/*-----------------------------------------------------------------------*/

static fs_result_t validate(              /* Returns FR_OK or FR_INVALID_OBJECT */
                        obj_id_t *obj, /* Pointer to the obj_id_t, the 1st member in the
                                         file_t/directory_t structure, to check validity */
                        filesystem_t **rfs   /* Pointer to pointer to the owner filesystem object to
                                         return */
)
{
    fs_result_t res = FR_INVALID_OBJECT;

    if (obj && obj->fs && obj->fs->fs_type &&
        obj->id == obj->fs->id) { /* Test if the object is valid */
#if FF_FS_REENTRANT
        if (lock_volume(obj->fs, 0)) { /* Take a grant to access the volume */
            if (!(disk_status(obj->fs->pdrv) & MEDIA_NOINIT)) { /* Test if the hosting phsical drive
                                                                 is kept initialized */
                res = FR_OK;
            } else {
                unlock_volume(obj->fs, FR_OK); /* Invalidated volume, abort to access */
            }
        } else { /* Could not take */
            res = FR_TIMEOUT;
        }
#else
        if (!(disk_status(obj->fs->pdrv) &
              MEDIA_NOINIT)) { /* Test if the hosting phsical drive is kept initialized */
            res = FR_OK;
        }
#endif
    }
    *rfs = (res == FR_OK) ? obj->fs : 0; /* Return corresponding filesystem object if it is valid */
    return res;
}

/*---------------------------------------------------------------------------

   Public Functions (FatFs API)

----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/* Mount/Unmount a Logical Drive                                         */
/*-----------------------------------------------------------------------*/

fs_result_t f_mount(filesystem_t *fs,         /* Pointer to the filesystem object to be registered
                                      (NULL:unmount)*/
                const char *path, /* Logical drive number to be mounted/unmounted */
                uint8_t opt           /* Mount option: 0=Do not mount (delayed mount), 1=Mount
                                      immediately */
)
{
    filesystem_t *cfs;
    int vol;
    fs_result_t res;
    const char *rp = path;

    /* Get volume ID (logical drive number) */
    vol = get_ldnumber(&rp);
    if (vol < 0)
        return FR_INVALID_DRIVE;
    cfs = FatFs[vol]; /* Pointer to the filesystem object of the volume */

    if (cfs) { /* Unregister current filesystem object if regsitered */
        FatFs[vol] = 0;
#if FF_FS_LOCK
        clear_share(cfs);
#endif
#if FF_FS_REENTRANT /* Discard mutex of the current volume */
        ff_mutex_delete(vol);
#endif
        cfs->fs_type = 0; /* Invalidate the filesystem object to be unregistered */
    }

    if (fs) {                  /* Register new filesystem object */
        fs->pdrv = LD2PD(vol); /* Volume hosting physical drive */
#if FF_FS_REENTRANT            /* Create a volume mutex */
        fs->ldrv = (uint8_t)vol;  /* Owner volume ID */
        if (!ff_mutex_create(vol))
            return FR_INT_ERR;
#if FF_FS_LOCK
        if (SysLock == 0) { /* Create a system mutex if needed */
            if (!ff_mutex_create(DISK_VOLUMES)) {
                ff_mutex_delete(vol);
                return FR_INT_ERR;
            }
            SysLock = 1; /* System mutex is ready */
        }
#endif
#endif
        fs->fs_type = 0; /* Invalidate the new filesystem object */
        FatFs[vol] = fs; /* Register new fs object */
    }

    if (opt == 0)
        return FR_OK; /* Do not mount now, it will be mounted in subsequent
                         file functions */

    res = mount_volume(&path, &fs, 0); /* Force mounted the volume */
    LEAVE_FF(fs, res);
}

#if !FF_FS_READONLY && !FF_FS_NORTC
//
// Get timestamp in FatFS format.
//
static uint32_t get_fattime(void)
{
    int year, month, day, dotw, hour, min, sec;
    rpm_get_datetime(&year, &month, &day, &dotw, &hour, &min, &sec);

    uint32_t fattime = 0;
    // bit31:25
    // Year origin from the 1980 (0..127, e.g. 37 for 2017)
    uint8_t yr = year - 1980;
    fattime |= (0b01111111 & yr) << 25;
    // bit24:21
    // Month (1..12)
    uint8_t mo = month;
    fattime |= (0b00001111 & mo) << 21;
    // bit20:16
    // Day of the month (1..31)
    uint8_t da = day;
    fattime |= (0b00011111 & da) << 16;
    // bit15:11
    // Hour (0..23)
    uint8_t hr = hour;
    fattime |= (0b00011111 & hr) << 11;
    // bit10:5
    // Minute (0..59)
    uint8_t mi = min;
    fattime |= (0b00111111 & mi) << 5;
    // bit4:0
    // Second / 2 (0..29, e.g. 25 for 50)
    uint8_t sd = sec / 2;
    fattime |= (0b00011111 & sd);
    return fattime;
}
#endif

/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

fs_result_t f_open(file_t *fp,           /* Pointer to the blank file object */
               const char *path, /* Pointer to the file name */
               uint8_t mode          /* Access mode and open mode flags */
)
{
    fs_result_t res;
    directory_t dj;
    filesystem_t *fs;
#if !FF_FS_READONLY
    uint32_t cl, bcs, clst, tm;
    fs_lba_t sc;
    fs_size_t ofs;
#endif
    DEF_NAMBUF

    if (!fp)
        return FR_INVALID_OBJECT;

    /* Get logical drive number */
    mode &= FF_FS_READONLY ? FA_READ
                           : FA_READ | FA_WRITE | FA_CREATE_ALWAYS | FA_CREATE_NEW |
                                 FA_OPEN_ALWAYS | FA_OPEN_APPEND;
    res = mount_volume(&path, &fs, mode);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path); /* Follow the file path */
#if !FF_FS_READONLY                   /* Read/Write configuration */
        if (res == FR_OK) {
            if (dj.fn[NSFLAG] & NS_NONAME) { /* Origin directory itself? */
                res = FR_INVALID_NAME;
            }
#if FF_FS_LOCK
            else {
                res = chk_share(&dj, (mode & ~FA_READ) ? 1 : 0); /* Check if the file can be used */
            }
#endif
        }
        /* Create or Open a file */
        if (mode & (FA_CREATE_ALWAYS | FA_OPEN_ALWAYS | FA_CREATE_NEW)) {
            if (res != FR_OK) {          /* No file, create new */
                if (res == FR_NO_FILE) { /* There is no file to open, create
                                            a new entry */
#if FF_FS_LOCK
                    res = enq_share() ? dir_register(&dj) : FR_TOO_MANY_OPEN_FILES;
#else
                    res = dir_register(&dj);
#endif
                }
                mode |= FA_CREATE_ALWAYS; /* File is created */
            } else {                      /* Any object with the same name is already existing */
                if (dj.obj.attr & (AM_RDO | AM_DIR)) { /* Cannot overwrite it (R/O or directory_t) */
                    res = FR_DENIED;
                } else {
                    if (mode & FA_CREATE_NEW)
                        res = FR_EXIST; /* Cannot create as new file */
                }
            }
            if (res == FR_OK && (mode & FA_CREATE_ALWAYS)) { /* Truncate the file if
                                                                overwrite mode */
                if (fs->fs_type == FS_EXFAT) {
                    /* Get current allocation info */
                    fp->obj.fs = fs;
                    init_alloc_info(fs, &fp->obj);
                    /* Set directory entry block initial state */
                    memset(fs->dirbuf + 2, 0, 30);  /* Clear 85 entry except for NumSec */
                    memset(fs->dirbuf + 38, 0, 26); /* Clear C0 entry except for NumName and
                                                       NameHash */
                    fs->dirbuf[XDIR_Attr] = AM_ARC;
                    st_dword(fs->dirbuf + XDIR_CrtTime, GET_FATTIME());
                    fs->dirbuf[XDIR_GenFlags] = 1;
                    res = store_xdir(&dj);
                    if (res == FR_OK &&
                        fp->obj.sclust != 0) { /* Remove the cluster chain if exist */
                        res = remove_chain(&fp->obj, fp->obj.sclust, 0);
                        fs->last_clst = fp->obj.sclust - 1; /* Reuse the cluster hole */
                    }
                } else {
                    /* Set directory entry initial state */
                    tm = GET_FATTIME(); /* Set created time */
                    st_dword(dj.dir + DIR_CrtTime, tm);
                    st_dword(dj.dir + DIR_ModTime, tm);
                    cl = ld_clust(fs, dj.dir); /* Get current cluster chain */
                    dj.dir[DIR_Attr] = AM_ARC; /* Reset attribute */
                    st_clust(fs, dj.dir, 0);   /* Reset file allocation info */
                    st_dword(dj.dir + DIR_FileSize, 0);
                    fs->wflag = 1;
                    if (cl != 0) { /* Remove the cluster chain if exist */
                        sc = fs->winsect;
                        res = remove_chain(&dj.obj, cl, 0);
                        if (res == FR_OK) {
                            res = move_window(fs, sc);
                            fs->last_clst = cl - 1; /* Reuse the cluster hole */
                        }
                    }
                }
            }
        } else {                            /* Open an existing file */
            if (res == FR_OK) {             /* Is the object exsiting? */
                if (dj.obj.attr & AM_DIR) { /* File open against a directory */
                    res = FR_NO_FILE;
                } else {
                    if ((mode & FA_WRITE) &&
                        (dj.obj.attr & AM_RDO)) { /* Write mode open against R/O file */
                        res = FR_DENIED;
                    }
                }
            }
        }
        if (res == FR_OK) {
            if (mode & FA_CREATE_ALWAYS)
                mode |= FA_MODIFIED;    /* Set file change flag if created or
                                           overwritten */
            fp->dir_sect = fs->winsect; /* Pointer to the directory entry */
            fp->dir_ptr = dj.dir;
#if FF_FS_LOCK
            fp->obj.lockid =
                inc_share(&dj, (mode & ~FA_READ) ? 1 : 0); /* Lock the file for this session */
            if (fp->obj.lockid == 0)
                res = FR_INT_ERR;
#endif
        }
#else /* R/O configuration */
        if (res == FR_OK) {
            if (dj.fn[NSFLAG] & NS_NONAME) { /* Is it origin directory itself? */
                res = FR_INVALID_NAME;
            } else {
                if (dj.obj.attr & AM_DIR) { /* Is it a directory? */
                    res = FR_NO_FILE;
                }
            }
        }
#endif

        if (res == FR_OK) {
            if (fs->fs_type == FS_EXFAT) {
                fp->obj.c_scl = dj.obj.sclust; /* Get containing directory info */
                fp->obj.c_size = ((uint32_t)dj.obj.objsize & 0xFFFFFF00) | dj.obj.stat;
                fp->obj.c_ofs = dj.blk_ofs;
                init_alloc_info(fs, &fp->obj);
            } else {
                fp->obj.sclust = ld_clust(fs, dj.dir); /* Get object allocation info */
                fp->obj.objsize = ld_dword(dj.dir + DIR_FileSize);
            }
#if FF_USE_FASTSEEK
            fp->cltbl = 0; /* Disable fast seek mode */
#endif
            fp->obj.fs = fs; /* Validate the file object */
            fp->obj.id = fs->id;
            fp->flag = mode; /* Set file access mode */
            fp->err = 0;     /* Clear error flag */
            fp->sect = 0;    /* Invalidate current data sector */
            fp->fptr = 0;    /* Set file pointer top of the file */
#if !FF_FS_READONLY
#if !FF_FS_TINY
            memset(fp->buf, 0, sizeof fp->buf); /* Clear sector buffer */
#endif
            if ((mode & FA_SEEKEND) && fp->obj.objsize > 0) { /* Seek to end of file if
                                                                 FA_OPEN_APPEND is specified */
                fp->fptr = fp->obj.objsize;                   /* Offset to seek */
                bcs = (uint32_t)fs->csize * SS(fs);              /* Cluster size in byte */
                clst = fp->obj.sclust;                        /* Follow the cluster chain */
                for (ofs = fp->obj.objsize; res == FR_OK && ofs > bcs; ofs -= bcs) {
                    clst = get_fat(&fp->obj, clst);
                    if (clst <= 1)
                        res = FR_INT_ERR;
                    if (clst == 0xFFFFFFFF)
                        res = FR_DISK_ERR;
                }
                fp->clust = clst;
                if (res == FR_OK && ofs % SS(fs)) { /* Fill sector buffer if not on the
                                                       sector boundary */
                    sc = clst2sect(fs, clst);
                    if (sc == 0) {
                        res = FR_INT_ERR;
                    } else {
                        fp->sect = sc + (uint32_t)(ofs / SS(fs));
#if !FF_FS_TINY
                        if (disk_read(fs->pdrv, fp->buf, fp->sect, 1) != DISK_OK)
                            res = FR_DISK_ERR;
#endif
                    }
                }
#if FF_FS_LOCK
                if (res != FR_OK)
                    dec_share(fp->obj.lockid); /* Decrement file open counter
                                                  if seek failed */
#endif
            }
#endif
        }

        FREE_NAMBUF();
    }

    if (res != FR_OK)
        fp->obj.fs = 0; /* Invalidate file object on error */

    LEAVE_FF(fs, res);
}

/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

fs_result_t f_read(file_t *fp,    /* Open file to be read */
               void *buff, /* Data buffer to store the read data */
               unsigned btr,   /* Number of bytes to read */
               unsigned *br    /* Number of bytes read */
)
{
    fs_result_t res;
    filesystem_t *fs;
    uint32_t clst;
    fs_lba_t sect;
    fs_size_t remain;
    unsigned rcnt, cc, csect;
    uint8_t *rbuff = (uint8_t *)buff;

    *br = 0;                       /* Clear read byte counter */
    res = validate(&fp->obj, &fs); /* Check validity of the file object */
    if (res != FR_OK || (res = (fs_result_t)fp->err) != FR_OK)
        LEAVE_FF(fs, res); /* Check validity */
    if (!(fp->flag & FA_READ))
        LEAVE_FF(fs, FR_DENIED); /* Check access mode */
    remain = fp->obj.objsize - fp->fptr;
    if (btr > remain)
        btr = (unsigned)remain; /* Truncate btr by remaining bytes */

    for (; btr > 0; btr -= rcnt, *br += rcnt, rbuff += rcnt,
                    fp->fptr += rcnt) {                          /* Repeat until btr bytes read */
        if (fp->fptr % SS(fs) == 0) {                            /* On the sector boundary? */
            csect = (unsigned)(fp->fptr / SS(fs) & (fs->csize - 1)); /* Sector offset in the cluster */
            if (csect == 0) {                                    /* On the cluster boundary? */
                if (fp->fptr == 0) {                             /* On the top of the file? */
                    clst = fp->obj.sclust;                       /* Follow cluster chain from the
                                                                    origin */
                } else {                                         /* Middle or end of the file */
#if FF_USE_FASTSEEK
                    if (fp->cltbl) {
                        clst = clmt_clust(fp, fp->fptr); /* Get cluster# from the CLMT */
                    } else
#endif
                    {
                        clst = get_fat(&fp->obj, fp->clust); /* Follow cluster chain on the FAT */
                    }
                }
                if (clst < 2)
                    ABORT(fs, FR_INT_ERR);
                if (clst == 0xFFFFFFFF)
                    ABORT(fs, FR_DISK_ERR);
                fp->clust = clst; /* Update current cluster */
            }
            sect = clst2sect(fs, fp->clust); /* Get current sector */
            if (sect == 0)
                ABORT(fs, FR_INT_ERR);
            sect += csect;
            cc = btr / SS(fs);                /* When remaining bytes >= sector size, */
            if (cc > 0) {                     /* Read maximum contiguous sectors directly */
                if (csect + cc > fs->csize) { /* Clip at cluster boundary */
                    cc = fs->csize - csect;
                }
                if (disk_read(fs->pdrv, rbuff, sect, cc) != DISK_OK)
                    ABORT(fs, FR_DISK_ERR);

                    /* Replace one of the read sectors with cached data if it
                     * contains a dirty sector */
#if !FF_FS_READONLY && FF_FS_MINIMIZE <= 2
#if FF_FS_TINY
                if (fs->wflag && fs->winsect - sect < cc) {
                    memcpy(rbuff + ((fs->winsect - sect) * SS(fs)), fs->win, SS(fs));
                }
#else
                if ((fp->flag & FA_DIRTY) && fp->sect - sect < cc) {
                    memcpy(rbuff + ((fp->sect - sect) * SS(fs)), fp->buf, SS(fs));
                }
#endif
#endif
                rcnt = SS(fs) * cc; /* Number of bytes transferred */
                continue;
            }
#if !FF_FS_TINY
            if (fp->sect != sect) { /* Load data sector if not in cache */
#if !FF_FS_READONLY
                if (fp->flag & FA_DIRTY) { /* Write-back dirty sector cache */
                    if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != DISK_OK)
                        ABORT(fs, FR_DISK_ERR);
                    fp->flag &= (uint8_t)~FA_DIRTY;
                }
#endif
                if (disk_read(fs->pdrv, fp->buf, sect, 1) != DISK_OK)
                    ABORT(fs, FR_DISK_ERR); /* Fill sector cache */
            }
#endif
            fp->sect = sect;
        }
        rcnt = SS(fs) - (unsigned)fp->fptr % SS(fs); /* Number of bytes remains in the sector */
        if (rcnt > btr)
            rcnt = btr; /* Clip it by btr if needed */
#if FF_FS_TINY
        if (move_window(fs, fp->sect) != FR_OK)
            ABORT(fs, FR_DISK_ERR);                       /* Move sector window */
        memcpy(rbuff, fs->win + fp->fptr % SS(fs), rcnt); /* Extract partial sector */
#else
        memcpy(rbuff, fp->buf + fp->fptr % SS(fs), rcnt); /* Extract partial sector */
#endif
    }

    LEAVE_FF(fs, FR_OK);
}

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Write File                                                            */
/*-----------------------------------------------------------------------*/

fs_result_t f_write(file_t *fp,          /* Open file to be written */
                const void *buff, /* Data to be written */
                unsigned btw,         /* Number of bytes to write */
                unsigned *bw          /* Number of bytes written */
)
{
    fs_result_t res;
    filesystem_t *fs;
    uint32_t clst;
    fs_lba_t sect;
    unsigned wcnt, cc, csect;
    const uint8_t *wbuff = (const uint8_t *)buff;

    *bw = 0;                       /* Clear write byte counter */
    res = validate(&fp->obj, &fs); /* Check validity of the file object */
    if (res != FR_OK || (res = (fs_result_t)fp->err) != FR_OK)
        LEAVE_FF(fs, res); /* Check validity */
    if (!(fp->flag & FA_WRITE))
        LEAVE_FF(fs, FR_DENIED); /* Check access mode */

    /* Check fptr wrap-around (file size cannot reach 4 GiB at FAT volume) */
    if ((fs->fs_type != FS_EXFAT) && (uint32_t)(fp->fptr + btw) < (uint32_t)fp->fptr) {
        btw = (unsigned)(0xFFFFFFFF - (uint32_t)fp->fptr);
    }

    for (; btw > 0; btw -= wcnt, *bw += wcnt, wbuff += wcnt, fp->fptr += wcnt,
                    fp->obj.objsize = (fp->fptr > fp->obj.objsize)
                                          ? fp->fptr
                                          : fp->obj.objsize) {   /* Repeat until all data written */
        if (fp->fptr % SS(fs) == 0) {                            /* On the sector boundary? */
            csect = (unsigned)(fp->fptr / SS(fs)) & (fs->csize - 1); /* Sector offset in the cluster */
            if (csect == 0) {                                    /* On the cluster boundary? */
                if (fp->fptr == 0) {                             /* On the top of the file? */
                    clst = fp->obj.sclust;                       /* Follow from the origin */
                    if (clst == 0) {                             /* If no cluster is allocated, */
                        clst = create_chain(&fp->obj, 0);        /* create a new cluster chain */
                    }
                } else { /* On the middle or end of the file */
#if FF_USE_FASTSEEK
                    if (fp->cltbl) {
                        clst = clmt_clust(fp, fp->fptr); /* Get cluster# from the CLMT */
                    } else
#endif
                    {
                        clst = create_chain(&fp->obj, fp->clust); /* Follow or stretch cluster chain
                                                                     on the FAT */
                    }
                }
                if (clst == 0)
                    break; /* Could not allocate a new cluster (disk full) */
                if (clst == 1)
                    ABORT(fs, FR_INT_ERR);
                if (clst == 0xFFFFFFFF)
                    ABORT(fs, FR_DISK_ERR);
                fp->clust = clst; /* Update current cluster */
                if (fp->obj.sclust == 0)
                    fp->obj.sclust = clst; /* Set start cluster if the first write */
            }
#if FF_FS_TINY
            if (fs->winsect == fp->sect && sync_window(fs) != FR_OK)
                ABORT(fs, FR_DISK_ERR); /* Write-back sector cache */
#else
            if (fp->flag & FA_DIRTY) { /* Write-back sector cache */
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != DISK_OK)
                    ABORT(fs, FR_DISK_ERR);
                fp->flag &= (uint8_t)~FA_DIRTY;
            }
#endif
            sect = clst2sect(fs, fp->clust); /* Get current sector */
            if (sect == 0)
                ABORT(fs, FR_INT_ERR);
            sect += csect;
            cc = btw / SS(fs);                /* When remaining bytes >= sector size, */
            if (cc > 0) {                     /* Write maximum contiguous sectors directly */
                if (csect + cc > fs->csize) { /* Clip at cluster boundary */
                    cc = fs->csize - csect;
                }
                if (disk_write(fs->pdrv, wbuff, sect, cc) != DISK_OK)
                    ABORT(fs, FR_DISK_ERR);
#if FF_FS_MINIMIZE <= 2
#if FF_FS_TINY
                if (fs->winsect - sect < cc) { /* Refill sector cache if it gets invalidated by
                                                  the direct write */
                    memcpy(fs->win, wbuff + ((fs->winsect - sect) * SS(fs)), SS(fs));
                    fs->wflag = 0;
                }
#else
                if (fp->sect - sect < cc) { /* Refill sector cache if it gets invalidated by
                                               the direct write */
                    memcpy(fp->buf, wbuff + ((fp->sect - sect) * SS(fs)), SS(fs));
                    fp->flag &= (uint8_t)~FA_DIRTY;
                }
#endif
#endif
                wcnt = SS(fs) * cc; /* Number of bytes transferred */
                continue;
            }
#if FF_FS_TINY
            if (fp->fptr >= fp->obj.objsize) { /* Avoid silly cache filling
                                                  on the growing edge */
                if (sync_window(fs) != FR_OK)
                    ABORT(fs, FR_DISK_ERR);
                fs->winsect = sect;
            }
#else
            if (fp->sect != sect && /* Fill sector cache with file data */
                fp->fptr < fp->obj.objsize && disk_read(fs->pdrv, fp->buf, sect, 1) != DISK_OK) {
                ABORT(fs, FR_DISK_ERR);
            }
#endif
            fp->sect = sect;
        }
        wcnt = SS(fs) - (unsigned)fp->fptr % SS(fs); /* Number of bytes remains in the sector */
        if (wcnt > btw)
            wcnt = btw; /* Clip it by btw if needed */
#if FF_FS_TINY
        if (move_window(fs, fp->sect) != FR_OK)
            ABORT(fs, FR_DISK_ERR);                       /* Move sector window */
        memcpy(fs->win + fp->fptr % SS(fs), wbuff, wcnt); /* Fit data to the sector */
        fs->wflag = 1;
#else
        memcpy(fp->buf + fp->fptr % SS(fs), wbuff, wcnt); /* Fit data to the sector */
        fp->flag |= FA_DIRTY;
#endif
    }

    fp->flag |= FA_MODIFIED; /* Set file change flag */

    LEAVE_FF(fs, FR_OK);
}

/*-----------------------------------------------------------------------*/
/* Synchronize the File                                                  */
/*-----------------------------------------------------------------------*/

fs_result_t f_sync(file_t *fp /* Open file to be synced */
)
{
    fs_result_t res;
    filesystem_t *fs;
    uint32_t tm;
    uint8_t *dir;

    res = validate(&fp->obj, &fs); /* Check validity of the file object */
    if (res == FR_OK) {
        if (fp->flag & FA_MODIFIED) { /* Is there any change to the file? */
#if !FF_FS_TINY
            if (fp->flag & FA_DIRTY) { /* Write-back cached data if needed */
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != DISK_OK)
                    LEAVE_FF(fs, FR_DISK_ERR);
                fp->flag &= (uint8_t)~FA_DIRTY;
            }
#endif
            /* Update the directory entry */
            tm = GET_FATTIME(); /* Modified time */
            if (fs->fs_type == FS_EXFAT) {
                res = fill_first_frag(&fp->obj); /* Fill first fragment on the FAT if needed */
                if (res == FR_OK) {
                    res = fill_last_frag(&fp->obj, fp->clust, 0xFFFFFFFF); /* Fill last fragment on
                                                                              the FAT if needed */
                }
                if (res == FR_OK) {
                    directory_t dj;
                    DEF_NAMBUF

                    INIT_NAMBUF(fs);
                    res = load_obj_xdir(&dj, &fp->obj); /* Load directory entry block */
                    if (res == FR_OK) {
                        fs->dirbuf[XDIR_Attr] |= AM_ARC; /* Set archive attribute to indicate that
                                                            the file has been changed */
                        fs->dirbuf[XDIR_GenFlags] =
                            fp->obj.stat | 1; /* Update file allocation information */
                        st_dword(fs->dirbuf + XDIR_FstClus,
                                 fp->obj.sclust); /* Update start cluster */
                        st_qword(fs->dirbuf + XDIR_FileSize,
                                 fp->obj.objsize); /* Update file size */
                        st_qword(fs->dirbuf + XDIR_ValidFileSize,
                                 fp->obj.objsize);               /* (FatFs does not support
                                                                    Valid File Size feature) */
                        st_dword(fs->dirbuf + XDIR_ModTime, tm); /* Update modified time */
                        fs->dirbuf[XDIR_ModTime10] = 0;
                        st_dword(fs->dirbuf + XDIR_AccTime, 0);
                        res = store_xdir(&dj); /* Restore it to the directory */
                        if (res == FR_OK) {
                            res = sync_fs(fs);
                            fp->flag &= (uint8_t)~FA_MODIFIED;
                        }
                    }
                    FREE_NAMBUF();
                }
            } else {
                res = move_window(fs, fp->dir_sect);
                if (res == FR_OK) {
                    dir = fp->dir_ptr;
                    dir[DIR_Attr] |= AM_ARC; /* Set archive attribute to indicate that the
                                                file has been changed */
                    st_clust(fp->obj.fs, dir, fp->obj.sclust);            /* Update file allocation
                                                                             information  */
                    st_dword(dir + DIR_FileSize, (uint32_t)fp->obj.objsize); /* Update file size */
                    st_dword(dir + DIR_ModTime, tm);                      /* Update modified time */
                    st_word(dir + DIR_LstAccDate, 0);
                    fs->wflag = 1;
                    res = sync_fs(fs); /* Restore it to the directory */
                    fp->flag &= (uint8_t)~FA_MODIFIED;
                }
            }
        }
    }

    LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */

/*-----------------------------------------------------------------------*/
/* Close File                                                            */
/*-----------------------------------------------------------------------*/

fs_result_t f_close(file_t *fp /* Open file to be closed */
)
{
    fs_result_t res;
    filesystem_t *fs;

#if !FF_FS_READONLY
    res = f_sync(fp); /* Flush cached data */
    if (res == FR_OK)
#endif
    {
        res = validate(&fp->obj, &fs); /* Lock volume */
        if (res == FR_OK) {
#if FF_FS_LOCK
            res = dec_share(fp->obj.lockid); /* Decrement file open counter */
            if (res == FR_OK)
                fp->obj.fs = 0; /* Invalidate file object */
#else
            fp->obj.fs = 0;                               /* Invalidate file object */
#endif
#if FF_FS_REENTRANT
            unlock_volume(fs, FR_OK); /* Unlock volume */
#endif
        }
    }
    return res;
}

#if FF_FS_RPATH >= 1
/*-----------------------------------------------------------------------*/
/* Change Current Directory or Current Drive, Get Current Directory      */
/*-----------------------------------------------------------------------*/

fs_result_t f_chdrive(const char *path /* Drive number to set */
)
{
    int vol;

    /* Get logical drive number */
    vol = get_ldnumber(&path);
    if (vol < 0)
        return FR_INVALID_DRIVE;
    CurrVol = (uint8_t)vol; /* Set it as current volume */

    return FR_OK;
}

//
// Get current drive.
//
int f_drive()
{
    return CurrVol;
}

fs_result_t f_chdir(const char *path /* Pointer to the directory path */
)
{
#if FF_STR_VOLUME_ID == 2
    unsigned i;
#endif
    fs_result_t res;
    directory_t dj;
    filesystem_t *fs;
    DEF_NAMBUF

    /* Get logical drive */
    res = mount_volume(&path, &fs, 0);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path);        /* Follow the path */
        if (res == FR_OK) {                  /* Follow completed */
            if (dj.fn[NSFLAG] & NS_NONAME) { /* Is it the start directory itself? */
                fs->cdir = dj.obj.sclust;
                if (fs->fs_type == FS_EXFAT) {
                    fs->cdc_scl = dj.obj.c_scl;
                    fs->cdc_size = dj.obj.c_size;
                    fs->cdc_ofs = dj.obj.c_ofs;
                }
            } else {
                if (dj.obj.attr & AM_DIR) { /* It is a sub-directory */
                    if (fs->fs_type == FS_EXFAT) {
                        fs->cdir = ld_dword(fs->dirbuf + XDIR_FstClus); /* Sub-directory cluster */
                        fs->cdc_scl = dj.obj.sclust;                    /* Save containing directory
                                                                           information */
                        fs->cdc_size = ((uint32_t)dj.obj.objsize & 0xFFFFFF00) | dj.obj.stat;
                        fs->cdc_ofs = dj.blk_ofs;
                    } else {
                        fs->cdir = ld_clust(fs, dj.dir); /* Sub-directory cluster */
                    }
                } else {
                    res = FR_NO_PATH; /* Reached but a file */
                }
            }
        }
        FREE_NAMBUF();
        if (res == FR_NO_FILE)
            res = FR_NO_PATH;
#if FF_STR_VOLUME_ID == 2 /* Also current drive is changed if in Unix style volume ID */
        if (res == FR_OK) {
            for (i = DISK_VOLUMES - 1; i && fs != FatFs[i]; i--)
                ; /* Set current drive */
            CurrVol = (uint8_t)i;
        }
#endif
    }

    LEAVE_FF(fs, res);
}

#if FF_FS_RPATH >= 2
fs_result_t f_getcwd(char *buff, /* Pointer to the directory path */
                 unsigned len     /* Size of buff in unit of char */
)
{
    fs_result_t res;
    directory_t dj;
    filesystem_t *fs;
    unsigned i, n;
    uint32_t ccl;
    char *tp = buff;
#if DISK_VOLUMES >= 2
    unsigned vl;
#if FF_STR_VOLUME_ID
    const char *vp;
#endif
#endif
    file_info_t fno;
    DEF_NAMBUF

    /* Get logical drive */
    buff[0] = 0;                                       /* Set null string to get current volume */
    res = mount_volume((const char **)&buff, &fs, 0); /* Get current volume */
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);

        /* Follow parent directories and create the path */
        i = len;                                       /* Bottom of buffer (directory stack base) */
        if (fs->fs_type != FS_EXFAT) { /* (Cannot do getcwd on exFAT and
                                                          returns root path) */
            dj.obj.sclust = fs->cdir;                  /* Start to follow upper directory from
                                                          current directory */
            while ((ccl = dj.obj.sclust) !=
                   0) { /* Repeat while current directory is a sub-directory */
                res = dir_sdi(&dj, 1 * SZDIRE); /* Get parent directory */
                if (res != FR_OK)
                    break;
                res = move_window(fs, dj.sect);
                if (res != FR_OK)
                    break;
                dj.obj.sclust = ld_clust(fs, dj.dir); /* Goto parent directory */
                res = dir_sdi(&dj, 0);
                if (res != FR_OK)
                    break;
                do { /* Find the entry links to the child directory */
                    res = DIR_READ_FILE(&dj);
                    if (res != FR_OK)
                        break;
                    if (ccl == ld_clust(fs, dj.dir))
                        break; /* Found the entry */
                    res = dir_next(&dj, 0);
                } while (res == FR_OK);
                if (res == FR_NO_FILE)
                    res = FR_INT_ERR; /* It cannot be 'not found'. */
                if (res != FR_OK)
                    break;
                get_fileinfo(&dj, &fno); /* Get the directory name and push
                                            it to the buffer */
                for (n = 0; fno.fname[n]; n++)
                    ;            /* Name length */
                if (i < n + 1) { /* Insufficient space to store the path name? */
                    res = FR_NOT_ENOUGH_CORE;
                    break;
                }
                while (n)
                    buff[--i] = fno.fname[--n]; /* Stack the name */
                buff[--i] = '/';
            }
        }
        if (res == FR_OK) {
            if (i == len)
                buff[--i] = '/'; /* Is it the root-directory? */
#if DISK_VOLUMES >= 2              /* Put drive prefix */
            vl = 0;
#if FF_STR_VOLUME_ID >= 1 /* String volume ID */
            for (n = 0, vp = disk_name[CurrVol]; vp[n]; n++)
                ;
            if (i >= n + 2) {
                if (FF_STR_VOLUME_ID == 2)
                    *tp++ = (char)'/';
                for (vl = 0; vl < n; *tp++ = (char)vp[vl], vl++)
                    ;
                if (FF_STR_VOLUME_ID == 1)
                    *tp++ = (char)':';
                vl++;
            }
#else /* Numeric volume ID */
            if (i >= 3) {
                *tp++ = (char)'0' + CurrVol;
                *tp++ = (char)':';
                vl = 2;
            }
#endif
            if (vl == 0)
                res = FR_NOT_ENOUGH_CORE;
#endif
            /* Add current directory path */
            if (res == FR_OK) {
                do { /* Copy stacked path string */
                    *tp++ = buff[i++];
                } while (i < len);
            }
        }
        FREE_NAMBUF();
    }

    *tp = 0;
    LEAVE_FF(fs, res);
}

#endif /* FF_FS_RPATH >= 2 */
#endif /* FF_FS_RPATH >= 1 */

#if FF_FS_MINIMIZE <= 2
/*-----------------------------------------------------------------------*/
/* Seek File Read/Write Pointer                                          */
/*-----------------------------------------------------------------------*/

fs_result_t f_lseek(file_t *fp,    /* Pointer to the file object */
                fs_size_t ofs /* File pointer from top of file */
)
{
    fs_result_t res;
    filesystem_t *fs;
    uint32_t clst, bcs;
    fs_lba_t nsect;
    fs_size_t ifptr;
#if FF_USE_FASTSEEK
    uint32_t cl, pcl, ncl, tcl, tlen, ulen;
    uint32_t *tbl;
    fs_lba_t dsc;
#endif

    res = validate(&fp->obj, &fs); /* Check validity of the file object */
    if (res == FR_OK)
        res = (fs_result_t)fp->err;
#if !FF_FS_READONLY
    if (res == FR_OK && fs->fs_type == FS_EXFAT) {
        res = fill_last_frag(&fp->obj, fp->clust,
                             0xFFFFFFFF); /* Fill last fragment on the FAT if needed */
    }
#endif
    if (res != FR_OK)
        LEAVE_FF(fs, res);

#if FF_USE_FASTSEEK
    if (fp->cltbl) {                 /* Fast seek */
        if (ofs == CREATE_LINKMAP) { /* Create CLMT */
            tbl = fp->cltbl;
            tlen = *tbl++;
            ulen = 2;            /* Given table size and required table size */
            cl = fp->obj.sclust; /* Origin of the chain */
            if (cl != 0) {
                do {
                    /* Get a fragment */
                    tcl = cl;
                    ncl = 0;
                    ulen += 2; /* Top, length and used items */
                    do {
                        pcl = cl;
                        ncl++;
                        cl = get_fat(&fp->obj, cl);
                        if (cl <= 1)
                            ABORT(fs, FR_INT_ERR);
                        if (cl == 0xFFFFFFFF)
                            ABORT(fs, FR_DISK_ERR);
                    } while (cl == pcl + 1);
                    if (ulen <= tlen) { /* Store the length and top of the
                                           fragment */
                        *tbl++ = ncl;
                        *tbl++ = tcl;
                    }
                } while (cl < fs->n_fatent); /* Repeat until end of chain */
            }
            *fp->cltbl = ulen; /* Number of items used */
            if (ulen <= tlen) {
                *tbl = 0; /* Terminate table */
            } else {
                res = FR_NOT_ENOUGH_CORE; /* Given table size is smaller than
                                             required */
            }
        } else { /* Fast seek */
            if (ofs > fp->obj.objsize)
                ofs = fp->obj.objsize; /* Clip offset at the file size */
            fp->fptr = ofs;            /* Set file pointer */
            if (ofs > 0) {
                fp->clust = clmt_clust(fp, ofs - 1);
                dsc = clst2sect(fs, fp->clust);
                if (dsc == 0)
                    ABORT(fs, FR_INT_ERR);
                dsc += (uint32_t)((ofs - 1) / SS(fs)) & (fs->csize - 1);
                if (fp->fptr % SS(fs) && dsc != fp->sect) { /* Refill sector cache if needed */
#if !FF_FS_TINY
#if !FF_FS_READONLY
                    if (fp->flag & FA_DIRTY) { /* Write-back dirty sector cache */
                        if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != DISK_OK)
                            ABORT(fs, FR_DISK_ERR);
                        fp->flag &= (uint8_t)~FA_DIRTY;
                    }
#endif
                    if (disk_read(fs->pdrv, fp->buf, dsc, 1) != DISK_OK)
                        ABORT(fs, FR_DISK_ERR); /* Load current sector */
#endif
                    fp->sect = dsc;
                }
            }
        }
    } else
#endif

    /* Normal Seek */
    {
        if (fs->fs_type != FS_EXFAT && ofs >= 0x100000000)
            ofs = 0xFFFFFFFF; /* Clip at 4 GiB - 1 if at FATxx */

        if (ofs > fp->obj.objsize &&
            (FF_FS_READONLY || !(fp->flag & FA_WRITE))) { /* In read-only mode, clip offset
                                                             with the file size */
            ofs = fp->obj.objsize;
        }
        ifptr = fp->fptr;
        fp->fptr = nsect = 0;
        if (ofs > 0) {
            bcs = (uint32_t)fs->csize * SS(fs); /* Cluster size (byte) */
            if (ifptr > 0 && (ofs - 1) / bcs >=
                                 (ifptr - 1) / bcs) { /* When seek to same or following cluster, */
                fp->fptr = (ifptr - 1) & ~(fs_size_t)(bcs - 1); /* start from the current cluster */
                ofs -= fp->fptr;
                clst = fp->clust;
            } else {                   /* When seek to back cluster, */
                clst = fp->obj.sclust; /* start from the first cluster */
#if !FF_FS_READONLY
                if (clst == 0) { /* If no cluster chain, create a new chain */
                    clst = create_chain(&fp->obj, 0);
                    if (clst == 1)
                        ABORT(fs, FR_INT_ERR);
                    if (clst == 0xFFFFFFFF)
                        ABORT(fs, FR_DISK_ERR);
                    fp->obj.sclust = clst;
                }
#endif
                fp->clust = clst;
            }
            if (clst != 0) {
                while (ofs > bcs) { /* Cluster following loop */
                    ofs -= bcs;
                    fp->fptr += bcs;
#if !FF_FS_READONLY
                    if (fp->flag & FA_WRITE) { /* Check if in write mode or not */
                        if (fp->fptr > fp->obj.objsize) { /* No FAT chain object
                                                                            needs correct objsize to
                                                                            generate FAT value */
                            fp->obj.objsize = fp->fptr;
                            fp->flag |= FA_MODIFIED;
                        }
                        clst = create_chain(&fp->obj, clst); /* Follow chain with forceed stretch */
                        if (clst == 0) { /* Clip file size in case of disk full */
                            ofs = 0;
                            break;
                        }
                    } else
#endif
                    {
                        clst = get_fat(&fp->obj, clst); /* Follow cluster chain if not
                                                           in write mode */
                    }
                    if (clst == 0xFFFFFFFF)
                        ABORT(fs, FR_DISK_ERR);
                    if (clst <= 1 || clst >= fs->n_fatent)
                        ABORT(fs, FR_INT_ERR);
                    fp->clust = clst;
                }
                fp->fptr += ofs;
                if (ofs % SS(fs)) {
                    nsect = clst2sect(fs, clst); /* Current sector */
                    if (nsect == 0)
                        ABORT(fs, FR_INT_ERR);
                    nsect += (uint32_t)(ofs / SS(fs));
                }
            }
        }
        if (!FF_FS_READONLY && fp->fptr > fp->obj.objsize) { /* Set file change flag if the file
                                                                size is extended */
            fp->obj.objsize = fp->fptr;
            fp->flag |= FA_MODIFIED;
        }
        if (fp->fptr % SS(fs) && nsect != fp->sect) { /* Fill sector cache if needed */
#if !FF_FS_TINY
#if !FF_FS_READONLY
            if (fp->flag & FA_DIRTY) { /* Write-back dirty sector cache */
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != DISK_OK)
                    ABORT(fs, FR_DISK_ERR);
                fp->flag &= (uint8_t)~FA_DIRTY;
            }
#endif
            if (disk_read(fs->pdrv, fp->buf, nsect, 1) != DISK_OK)
                ABORT(fs, FR_DISK_ERR); /* Fill sector cache */
#endif
            fp->sect = nsect;
        }
    }

    LEAVE_FF(fs, res);
}

#if FF_FS_MINIMIZE <= 1
/*-----------------------------------------------------------------------*/
/* Create a Directory Object                                             */
/*-----------------------------------------------------------------------*/

fs_result_t f_opendir(directory_t *dp,          /* Pointer to directory object to create */
                  const char *path /* Pointer to the directory path */
)
{
    fs_result_t res;
    filesystem_t *fs;
    DEF_NAMBUF

    if (!dp)
        return FR_INVALID_OBJECT;

    /* Get logical drive */
    res = mount_volume(&path, &fs, 0);
    if (res == FR_OK) {
        dp->obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(dp, path);             /* Follow the path to the directory */
        if (res == FR_OK) {                      /* Follow completed */
            if (!(dp->fn[NSFLAG] & NS_NONAME)) { /* It is not the origin directory itself */
                if (dp->obj.attr & AM_DIR) {     /* This object is a sub-directory */
                    if (fs->fs_type == FS_EXFAT) {
                        dp->obj.c_scl = dp->obj.sclust; /* Get containing directory
                                                           inforamation */
                        dp->obj.c_size = ((uint32_t)dp->obj.objsize & 0xFFFFFF00) | dp->obj.stat;
                        dp->obj.c_ofs = dp->blk_ofs;
                        init_alloc_info(fs, &dp->obj); /* Get object allocation info */
                    } else {
                        dp->obj.sclust = ld_clust(fs, dp->dir); /* Get object allocation info */
                    }
                } else { /* This object is a file */
                    res = FR_NO_PATH;
                }
            }
            if (res == FR_OK) {
                dp->obj.id = fs->id;
                res = dir_sdi(dp, 0); /* Rewind directory */
#if FF_FS_LOCK
                if (res == FR_OK) {
                    if (dp->obj.sclust != 0) {
                        dp->obj.lockid = inc_share(dp, 0); /* Lock the sub directory */
                        if (!dp->obj.lockid)
                            res = FR_TOO_MANY_OPEN_FILES;
                    } else {
                        dp->obj.lockid = 0; /* Root directory need not to be locked */
                    }
                }
#endif
            }
        }
        FREE_NAMBUF();
        if (res == FR_NO_FILE)
            res = FR_NO_PATH;
    }
    if (res != FR_OK)
        dp->obj.fs = 0; /* Invalidate the directory object if function failed */

    LEAVE_FF(fs, res);
}

/*-----------------------------------------------------------------------*/
/* Close Directory                                                       */
/*-----------------------------------------------------------------------*/

fs_result_t f_closedir(directory_t *dp /* Pointer to the directory object to be closed */
)
{
    fs_result_t res;
    filesystem_t *fs;

    res = validate(&dp->obj, &fs); /* Check validity of the file object */
    if (res == FR_OK) {
#if FF_FS_LOCK
        if (dp->obj.lockid)
            res = dec_share(dp->obj.lockid); /* Decrement sub-directory open counter */
        if (res == FR_OK)
            dp->obj.fs = 0; /* Invalidate directory object */
#else
        dp->obj.fs = 0; /* Invalidate directory object */
#endif
#if FF_FS_REENTRANT
        unlock_volume(fs, FR_OK); /* Unlock volume */
#endif
    }
    return res;
}

/*-----------------------------------------------------------------------*/
/* Read Directory Entries in Sequence                                    */
/*-----------------------------------------------------------------------*/

fs_result_t f_readdir(directory_t *dp,     /* Pointer to the open directory object */
                  file_info_t *fno /* Pointer to file information to return */
)
{
    fs_result_t res;
    filesystem_t *fs;
    DEF_NAMBUF

    res = validate(&dp->obj, &fs); /* Check validity of the directory object */
    if (res == FR_OK) {
        if (!fno) {
            res = dir_sdi(dp, 0); /* Rewind the directory object */
        } else {
            INIT_NAMBUF(fs);
            res = DIR_READ_FILE(dp); /* Read an item */
            if (res == FR_NO_FILE)
                res = FR_OK;           /* Ignore end of directory */
            if (res == FR_OK) {        /* A valid entry is found */
                get_fileinfo(dp, fno); /* Get the object information */
                res = dir_next(dp, 0); /* Increment index for next */
                if (res == FR_NO_FILE)
                    res = FR_OK; /* Ignore end of directory now */
            }
            FREE_NAMBUF();
        }
    }
    LEAVE_FF(fs, res);
}

#if FF_USE_FIND
/*-----------------------------------------------------------------------*/
/* Find Next File                                                        */
/*-----------------------------------------------------------------------*/

fs_result_t f_findnext(directory_t *dp,     /* Pointer to the open directory object */
                   file_info_t *fno /* Pointer to the file information structure */
)
{
    fs_result_t res;

    for (;;) {
        res = f_readdir(dp, fno); /* Get a directory item */
        if (res != FR_OK || !fno || !fno->fname[0])
            break; /* Terminate if any error or end of directory */
        if (pattern_match(dp->pat, fno->fname, 0, FIND_RECURS))
            break; /* Test for the file name */
#if FF_USE_FIND == 2
        if (pattern_match(dp->pat, fno->altname, 0, FIND_RECURS))
            break; /* Test for alternative name if exist */
#endif
    }
    return res;
}

/*-----------------------------------------------------------------------*/
/* Find First File                                                       */
/*-----------------------------------------------------------------------*/

fs_result_t f_findfirst(directory_t *dp,             /* Pointer to the blank directory object */
                    file_info_t *fno,        /* Pointer to the file information structure */
                    const char *path,   /* Pointer to the directory to open */
                    const char *pattern /* Pointer to the matching pattern */
)
{
    fs_result_t res;

    dp->pat = pattern;         /* Save pointer to pattern string */
    res = f_opendir(dp, path); /* Open the target directory */
    if (res == FR_OK) {
        res = f_findnext(dp, fno); /* Find the first item */
    }
    return res;
}

#endif /* FF_USE_FIND */

#if FF_FS_MINIMIZE == 0
/*-----------------------------------------------------------------------*/
/* Get File Status                                                       */
/*-----------------------------------------------------------------------*/

fs_result_t f_stat(const char *path, /* Pointer to the file path */
               file_info_t *fno       /* Pointer to file information to return */
)
{
    fs_result_t res;
    directory_t dj;
    DEF_NAMBUF

    /* Get logical drive */
    res = mount_volume(&path, &dj.obj.fs, 0);
    if (res == FR_OK) {
        INIT_NAMBUF(dj.obj.fs);
        res = follow_path(&dj, path);        /* Follow the file path */
        if (res == FR_OK) {                  /* Follow completed */
            if (dj.fn[NSFLAG] & NS_NONAME) { /* It is origin directory */
                res = FR_INVALID_NAME;
            } else { /* Found an object */
                if (fno)
                    get_fileinfo(&dj, fno);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(dj.obj.fs, res);
}

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Get Number of Free Clusters                                           */
/*-----------------------------------------------------------------------*/

fs_result_t f_getfree(const char *path, /* Logical drive number */
                  uint32_t *nclst,      /* Pointer to a variable to return number of
                                        free clusters */
                  filesystem_t **fatfs      /* Pointer to return pointer to corresponding
                                        filesystem object */
)
{
    fs_result_t res;
    filesystem_t *fs;
    uint32_t nfree, clst, stat;
    fs_lba_t sect;
    unsigned i;
    obj_id_t obj;

    /* Get logical drive */
    res = mount_volume(&path, &fs, 0);
    if (res == FR_OK) {
        *fatfs = fs; /* Return ptr to the fs object */
        /* If free_clst is valid, return it without full FAT scan */
        if (fs->free_clst <= fs->n_fatent - 2) {
            *nclst = fs->free_clst;
        } else {
            /* Scan FAT to obtain number of free clusters */
            nfree = 0;
            if (fs->fs_type == FS_FAT12) { /* FAT12: Scan bit field FAT entries */
                clst = 2;
                obj.fs = fs;
                do {
                    stat = get_fat(&obj, clst);
                    if (stat == 0xFFFFFFFF) {
                        res = FR_DISK_ERR;
                        break;
                    }
                    if (stat == 1) {
                        res = FR_INT_ERR;
                        break;
                    }
                    if (stat == 0)
                        nfree++;
                } while (++clst < fs->n_fatent);
            } else {
                if (fs->fs_type == FS_EXFAT) { /* exFAT: Scan allocation bitmap */
                    uint8_t bm;
                    unsigned b;

                    clst = fs->n_fatent - 2; /* Number of clusters */
                    sect = fs->bitbase;      /* Bitmap sector */
                    i = 0;                   /* Offset in the sector */
                    do {                     /* Counts numbuer of bits with zero in the bitmap */
                        if (i == 0) {        /* New sector? */
                            res = move_window(fs, sect++);
                            if (res != FR_OK)
                                break;
                        }
                        for (b = 8, bm = ~fs->win[i]; b && clst; b--, clst--) {
                            nfree += bm & 1;
                            bm >>= 1;
                        }
                        i = (i + 1) % SS(fs);
                    } while (clst);
                } else {
                    /* FAT16/32: Scan uint16_t/uint32_t FAT entries */
                    clst = fs->n_fatent; /* Number of entries */
                    sect = fs->fatbase;  /* Top of the FAT */
                    i = 0;               /* Offset in the sector */
                    do {                 /* Counts numbuer of entries with zero in the FAT */
                        if (i == 0) {    /* New sector? */
                            res = move_window(fs, sect++);
                            if (res != FR_OK)
                                break;
                        }
                        if (fs->fs_type == FS_FAT16) {
                            if (ld_word(fs->win + i) == 0)
                                nfree++;
                            i += 2;
                        } else {
                            if ((ld_dword(fs->win + i) & 0x0FFFFFFF) == 0)
                                nfree++;
                            i += 4;
                        }
                        i %= SS(fs);
                    } while (--clst);
                }
            }
            if (res == FR_OK) {        /* Update parameters if succeeded */
                *nclst = nfree;        /* Return the free clusters */
                fs->free_clst = nfree; /* Now free_clst is valid */
                fs->fsi_flag |= 1;     /* FAT32: FSInfo is to be updated */
            }
        }
    }

    LEAVE_FF(fs, res);
}

/*-----------------------------------------------------------------------*/
/* Truncate File                                                         */
/*-----------------------------------------------------------------------*/

fs_result_t f_truncate(file_t *fp /* Pointer to the file object */
)
{
    fs_result_t res;
    filesystem_t *fs;
    uint32_t ncl;

    res = validate(&fp->obj, &fs); /* Check validity of the file object */
    if (res != FR_OK || (res = (fs_result_t)fp->err) != FR_OK)
        LEAVE_FF(fs, res);
    if (!(fp->flag & FA_WRITE))
        LEAVE_FF(fs, FR_DENIED); /* Check access mode */

    if (fp->fptr < fp->obj.objsize) { /* Process when fptr is not on the eof */
        if (fp->fptr == 0) {          /* When set file size to zero, remove entire
                                         cluster chain */
            res = remove_chain(&fp->obj, fp->obj.sclust, 0);
            fp->obj.sclust = 0;
        } else { /* When truncate a part of the file, remove remaining
                    clusters */
            ncl = get_fat(&fp->obj, fp->clust);
            res = FR_OK;
            if (ncl == 0xFFFFFFFF)
                res = FR_DISK_ERR;
            if (ncl == 1)
                res = FR_INT_ERR;
            if (res == FR_OK && ncl < fs->n_fatent) {
                res = remove_chain(&fp->obj, ncl, fp->clust);
            }
        }
        fp->obj.objsize = fp->fptr; /* Set file size to current read/write point */
        fp->flag |= FA_MODIFIED;
#if !FF_FS_TINY
        if (res == FR_OK && (fp->flag & FA_DIRTY)) {
            if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != DISK_OK) {
                res = FR_DISK_ERR;
            } else {
                fp->flag &= (uint8_t)~FA_DIRTY;
            }
        }
#endif
        if (res != FR_OK)
            ABORT(fs, res);
    }

    LEAVE_FF(fs, res);
}

/*-----------------------------------------------------------------------*/
/* Delete a File/Directory                                               */
/*-----------------------------------------------------------------------*/

fs_result_t f_unlink(const char *path /* Pointer to the file or directory path */
)
{
    fs_result_t res;
    filesystem_t *fs;
    directory_t dj, sdj;
    uint32_t dclst = 0;
    obj_id_t obj;
    DEF_NAMBUF

    /* Get logical drive */
    res = mount_volume(&path, &fs, FA_WRITE);
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path); /* Follow the file path */
        if (FF_FS_RPATH && res == FR_OK && (dj.fn[NSFLAG] & NS_DOT)) {
            res = FR_INVALID_NAME; /* Cannot remove dot entry */
        }
#if FF_FS_LOCK
        if (res == FR_OK)
            res = chk_share(&dj, 2); /* Check if it is an open object */
#endif
        if (res == FR_OK) { /* The object is accessible */
            if (dj.fn[NSFLAG] & NS_NONAME) {
                res = FR_INVALID_NAME; /* Cannot remove the origin directory */
            } else {
                if (dj.obj.attr & AM_RDO) {
                    res = FR_DENIED; /* Cannot remove R/O object */
                }
            }
            if (res == FR_OK) {
                obj.fs = fs;
                if (fs->fs_type == FS_EXFAT) {
                    init_alloc_info(fs, &obj);
                    dclst = obj.sclust;
                } else {
                    dclst = ld_clust(fs, dj.dir);
                }
                if (dj.obj.attr & AM_DIR) { /* Is it a sub-directory? */
#if FF_FS_RPATH != 0
                    if (dclst == fs->cdir) { /* Is it the current directory? */
                        res = FR_DENIED;
                    } else
#endif
                    {
                        sdj.obj.fs = fs; /* Open the sub-directory */
                        sdj.obj.sclust = dclst;
                        if (fs->fs_type == FS_EXFAT) {
                            sdj.obj.objsize = obj.objsize;
                            sdj.obj.stat = obj.stat;
                        }
                        res = dir_sdi(&sdj, 0);
                        if (res == FR_OK) {
                            res = DIR_READ_FILE(&sdj); /* Test if the directory is empty */
                            if (res == FR_OK)
                                res = FR_DENIED; /* Not empty? */
                            if (res == FR_NO_FILE)
                                res = FR_OK; /* Empty? */
                        }
                    }
                }
            }
            if (res == FR_OK) {
                res = dir_remove(&dj);            /* Remove the directory entry */
                if (res == FR_OK && dclst != 0) { /* Remove the cluster chain if exist */
                    res = remove_chain(&obj, dclst, 0);
                }
                if (res == FR_OK)
                    res = sync_fs(fs);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

/*-----------------------------------------------------------------------*/
/* Create a Directory                                                    */
/*-----------------------------------------------------------------------*/

fs_result_t f_mkdir(const char *path /* Pointer to the directory path */
)
{
    fs_result_t res;
    filesystem_t *fs;
    directory_t dj;
    obj_id_t sobj;
    uint32_t dcl, pcl, tm;
    DEF_NAMBUF

    res = mount_volume(&path, &fs, FA_WRITE); /* Get logical drive */
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path); /* Follow the file path */
        if (res == FR_OK)
            res = FR_EXIST;                                                 /* Name collision? */
        if (FF_FS_RPATH && res == FR_NO_FILE && (dj.fn[NSFLAG] & NS_DOT)) { /* Invalid name? */
            res = FR_INVALID_NAME;
        }
        if (res == FR_NO_FILE) {          /* It is clear to create a new directory */
            sobj.fs = fs;                 /* New object id to create a new chain */
            dcl = create_chain(&sobj, 0); /* Allocate a cluster for the new directory */
            res = FR_OK;
            if (dcl == 0)
                res = FR_DENIED; /* No space to allocate a new cluster? */
            if (dcl == 1)
                res = FR_INT_ERR; /* Any insanity? */
            if (dcl == 0xFFFFFFFF)
                res = FR_DISK_ERR; /* Disk error? */
            tm = GET_FATTIME();
            if (res == FR_OK) {
                res = dir_clear(fs, dcl); /* Clean up the new table */
                if (res == FR_OK) {
                    if (fs->fs_type != FS_EXFAT) {           /* Create dot entries (FAT only) */
                        memset(fs->win + DIR_Name, ' ', 11); /* Create "." entry */
                        fs->win[DIR_Name] = '.';
                        fs->win[DIR_Attr] = AM_DIR;
                        st_dword(fs->win + DIR_ModTime, tm);
                        st_clust(fs, fs->win, dcl);
                        memcpy(fs->win + SZDIRE, fs->win, SZDIRE); /* Create ".." entry */
                        fs->win[SZDIRE + 1] = '.';
                        pcl = dj.obj.sclust;
                        st_clust(fs, fs->win + SZDIRE, pcl);
                        fs->wflag = 1;
                    }
                    res = dir_register(&dj); /* Register the object to the
                                                parent directoy */
                }
            }
            if (res == FR_OK) {
                if (fs->fs_type == FS_EXFAT) {                /* Initialize directory entry block */
                    st_dword(fs->dirbuf + XDIR_ModTime, tm);  /* Created time */
                    st_dword(fs->dirbuf + XDIR_FstClus, dcl); /* Table start cluster */
                    st_dword(fs->dirbuf + XDIR_FileSize,
                             (uint32_t)fs->csize * SS(fs)); /* Directory size needs to be valid */
                    st_dword(fs->dirbuf + XDIR_ValidFileSize, (uint32_t)fs->csize * SS(fs));
                    fs->dirbuf[XDIR_GenFlags] = 3;  /* Initialize the object flag */
                    fs->dirbuf[XDIR_Attr] = AM_DIR; /* Attribute */
                    res = store_xdir(&dj);
                } else {
                    st_dword(dj.dir + DIR_ModTime, tm); /* Created time */
                    st_clust(fs, dj.dir, dcl);          /* Table start cluster */
                    dj.dir[DIR_Attr] = AM_DIR;          /* Attribute */
                    fs->wflag = 1;
                }
                if (res == FR_OK) {
                    res = sync_fs(fs);
                }
            } else {
                remove_chain(&sobj, dcl, 0); /* Could not register, remove
                                                the allocated cluster */
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

/*-----------------------------------------------------------------------*/
/* Rename a File/Directory                                               */
/*-----------------------------------------------------------------------*/

fs_result_t f_rename(const char *path_old, /* Pointer to the object name to be renamed */
                 const char *path_new  /* Pointer to the new name */
)
{
    fs_result_t res;
    filesystem_t *fs;
    directory_t djo, djn;
    uint8_t buf[SZDIRE * 2], *dir;
    fs_lba_t sect;
    DEF_NAMBUF

    get_ldnumber(&path_new);                      /* Snip the drive number of new name off */
    res = mount_volume(&path_old, &fs, FA_WRITE); /* Get logical drive of the old object */
    if (res == FR_OK) {
        djo.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&djo, path_old); /* Check old object */
        if (res == FR_OK && (djo.fn[NSFLAG] & (NS_DOT | NS_NONAME)))
            res = FR_INVALID_NAME; /* Check validity of name */
#if FF_FS_LOCK
        if (res == FR_OK) {
            res = chk_share(&djo, 2);
        }
#endif
        if (res == FR_OK) { /* Object to be renamed is found */
            if (fs->fs_type == FS_EXFAT) { /* At exFAT volume */
                uint8_t nf, nn;
                uint16_t nh;

                memcpy(buf, fs->dirbuf, SZDIRE * 2); /* Save 85+C0 entry of old object */
                memcpy(&djn, &djo, sizeof djo);
                res = follow_path(&djn, path_new); /* Make sure if new object
                                                      name is not in use */
                if (res == FR_OK) {                /* Is new name already in use by any
                                                      other object? */
                    res = (djn.obj.sclust == djo.obj.sclust && djn.dptr == djo.dptr) ? FR_NO_FILE
                                                                                     : FR_EXIST;
                }
                if (res == FR_NO_FILE) {      /* It is a valid path and no name
                                                 collision */
                    res = dir_register(&djn); /* Register the new entry */
                    if (res == FR_OK) {
                        nf = fs->dirbuf[XDIR_NumSec];
                        nn = fs->dirbuf[XDIR_NumName];
                        nh = ld_word(fs->dirbuf + XDIR_NameHash);
                        memcpy(fs->dirbuf, buf, SZDIRE * 2); /* Restore 85+C0 entry */
                        fs->dirbuf[XDIR_NumSec] = nf;
                        fs->dirbuf[XDIR_NumName] = nn;
                        st_word(fs->dirbuf + XDIR_NameHash, nh);
                        if (!(fs->dirbuf[XDIR_Attr] & AM_DIR))
                            fs->dirbuf[XDIR_Attr] |= AM_ARC; /* Set archive attribute if it is a
                                                                file */
                        /* Start of critical section where an interruption
                         * can cause a cross-link */
                        res = store_xdir(&djn);
                    }
                }
            } else {
                /* At FAT/FAT32 volume */
                memcpy(buf, djo.dir, SZDIRE);      /* Save directory entry of the object */
                memcpy(&djn, &djo, sizeof(directory_t));   /* Duplicate the directory object */
                res = follow_path(&djn, path_new); /* Make sure if new object
                                                      name is not in use */
                if (res == FR_OK) {                /* Is new name already in use by any
                                                      other object? */
                    res = (djn.obj.sclust == djo.obj.sclust && djn.dptr == djo.dptr) ? FR_NO_FILE
                                                                                     : FR_EXIST;
                }
                if (res == FR_NO_FILE) {      /* It is a valid path and no name
                                                 collision */
                    res = dir_register(&djn); /* Register the new entry */
                    if (res == FR_OK) {
                        dir = djn.dir; /* Copy directory entry of the object
                                          except name */
                        memcpy(dir + 13, buf + 13, SZDIRE - 13);
                        dir[DIR_Attr] = buf[DIR_Attr];
                        if (!(dir[DIR_Attr] & AM_DIR))
                            dir[DIR_Attr] |= AM_ARC; /* Set archive attribute
                                                        if it is a file */
                        fs->wflag = 1;
                        if ((dir[DIR_Attr] & AM_DIR) &&
                            djo.obj.sclust != djn.obj.sclust) { /* Update .. entry in the
                                                                   sub-directory if needed */
                            sect = clst2sect(fs, ld_clust(fs, dir));
                            if (sect == 0) {
                                res = FR_INT_ERR;
                            } else {
                                /* Start of critical section where an
                                 * interruption can cause a cross-link */
                                res = move_window(fs, sect);
                                dir = fs->win + SZDIRE * 1; /* Ptr to .. entry */
                                if (res == FR_OK && dir[1] == '.') {
                                    st_clust(fs, dir, djn.obj.sclust);
                                    fs->wflag = 1;
                                }
                            }
                        }
                    }
                }
            }
            if (res == FR_OK) {
                res = dir_remove(&djo); /* Remove old entry */
                if (res == FR_OK) {
                    res = sync_fs(fs);
                }
            }
            /* End of the critical section */
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */
#endif /* FF_FS_MINIMIZE == 0 */
#endif /* FF_FS_MINIMIZE <= 1 */
#endif /* FF_FS_MINIMIZE <= 2 */

#if FF_USE_CHMOD && !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Change Attribute                                                      */
/*-----------------------------------------------------------------------*/

fs_result_t f_chmod(const char *path, /* Pointer to the file path */
                uint8_t attr,         /* Attribute bits */
                uint8_t mask          /* Attribute mask to change */
)
{
    fs_result_t res;
    filesystem_t *fs;
    directory_t dj;
    DEF_NAMBUF

    res = mount_volume(&path, &fs, FA_WRITE); /* Get logical drive */
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path); /* Follow the file path */
        if (res == FR_OK && (dj.fn[NSFLAG] & (NS_DOT | NS_NONAME)))
            res = FR_INVALID_NAME; /* Check object validity */
        if (res == FR_OK) {
            mask &= AM_RDO | AM_HID | AM_SYS | AM_ARC; /* Valid attribute mask */
            if (fs->fs_type == FS_EXFAT) {
                fs->dirbuf[XDIR_Attr] = (attr & mask) | (fs->dirbuf[XDIR_Attr] &
                                                         (uint8_t)~mask); /* Apply attribute change */
                res = store_xdir(&dj);
            } else {
                dj.dir[DIR_Attr] =
                    (attr & mask) | (dj.dir[DIR_Attr] & (uint8_t)~mask); /* Apply attribute change */
                fs->wflag = 1;
            }
            if (res == FR_OK) {
                res = sync_fs(fs);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

/*-----------------------------------------------------------------------*/
/* Change Timestamp                                                      */
/*-----------------------------------------------------------------------*/

fs_result_t f_utime(const char *path, /* Pointer to the file/directory name */
                const file_info_t *fno /* Pointer to the timestamp to be set */
)
{
    fs_result_t res;
    filesystem_t *fs;
    directory_t dj;
    DEF_NAMBUF

    res = mount_volume(&path, &fs, FA_WRITE); /* Get logical drive */
    if (res == FR_OK) {
        dj.obj.fs = fs;
        INIT_NAMBUF(fs);
        res = follow_path(&dj, path); /* Follow the file path */
        if (res == FR_OK && (dj.fn[NSFLAG] & (NS_DOT | NS_NONAME)))
            res = FR_INVALID_NAME; /* Check object validity */
        if (res == FR_OK) {
            if (fs->fs_type == FS_EXFAT) {
                st_dword(fs->dirbuf + XDIR_ModTime, (uint32_t)fno->fdate << 16 | fno->ftime);
                res = store_xdir(&dj);
            } else {
                st_dword(dj.dir + DIR_ModTime, (uint32_t)fno->fdate << 16 | fno->ftime);
                fs->wflag = 1;
            }
            if (res == FR_OK) {
                res = sync_fs(fs);
            }
        }
        FREE_NAMBUF();
    }

    LEAVE_FF(fs, res);
}

#endif /* FF_USE_CHMOD && !FF_FS_READONLY */

#if FF_USE_LABEL
/*-----------------------------------------------------------------------*/
/* Get Volume Label                                                      */
/*-----------------------------------------------------------------------*/

fs_result_t f_getlabel(const char *path, /* Logical drive number */
                   char *label,      /* Buffer to store the volume label */
                   uint32_t *vsn)        /* Variable to store the volume serial number */
{
    fs_result_t res;
    filesystem_t *fs;
    directory_t dj;
    unsigned si, di;
    uint16_t wc;

    /* Get logical drive */
    res = mount_volume(&path, &fs, 0);

    /* Get volume label */
    if (res == FR_OK && label) {
        dj.obj.fs = fs;
        dj.obj.sclust = 0; /* Open root directory */
        res = dir_sdi(&dj, 0);
        if (res == FR_OK) {
            res = DIR_READ_LABEL(&dj); /* Find a volume label entry */
            if (res == FR_OK) {
                if (fs->fs_type == FS_EXFAT) {
                    uint16_t hs;
                    unsigned nw;

                    for (si = di = hs = 0; si < dj.dir[XDIR_NumLabel];
                         si++) { /* Extract volume label from 83 entry */
                        wc = ld_word(dj.dir + XDIR_Label + si * 2);
                        if (hs == 0 && IsSurrogate(wc)) { /* Is the code a surrogate? */
                            hs = wc;
                            continue;
                        }
                        nw = put_utf((uint32_t)hs << 16 | wc, &label[di],
                                     4); /* Store it in API encoding */
                        if (nw == 0) {   /* Encode error? */
                            di = 0;
                            break;
                        }
                        di += nw;
                        hs = 0;
                    }
                    if (hs != 0)
                        di = 0; /* Broken surrogate pair? */
                    label[di] = 0;
                } else {
                    si = di = 0; /* Extract volume label from AM_VOL entry */
                    while (si < 11) {
                        wc = dj.dir[si++];

                        /* Unicode output */
                        if (dbc_1st((uint8_t)wc) && si < 11)
                            wc = wc << 8 | dj.dir[si++]; /* Is it a DBC? */
                        wc = ff_oem2uni(wc, CODEPAGE);   /* Convert it into Unicode */
                        if (wc == 0) {                   /* Invalid char in current code page? */
                            di = 0;
                            break;
                        }
                        di += put_utf(wc, &label[di], 4); /* Store it in Unicode */
                    }
                    do { /* Truncate trailing spaces */
                        label[di] = 0;
                        if (di == 0)
                            break;
                    } while (label[--di] == ' ');
                }
            }
        }
        if (res == FR_NO_FILE) { /* No label entry and return nul string */
            label[0] = 0;
            res = FR_OK;
        }
    }

    /* Get volume serial number */
    if (res == FR_OK && vsn) {
        res = move_window(fs, fs->volbase);
        if (res == FR_OK) {
            switch (fs->fs_type) {
            case FS_EXFAT:
                di = BPB_VolIDEx;
                break;

            case FS_FAT32:
                di = BS_VolID32;
                break;

            default:
                di = BS_VolID;
            }
            *vsn = ld_dword(fs->win + di);
        }
    }

    LEAVE_FF(fs, res);
}

#if !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Set Volume Label                                                      */
/*-----------------------------------------------------------------------*/

fs_result_t f_setlabel(const char *label) // Volume label to set with heading logical drive number
{
    fs_result_t res;
    filesystem_t *fs;
    directory_t dj;
    uint8_t dirvn[22];
    unsigned di;
    uint16_t wc;
    static const char badchr[18] =
        "+.,;=[]"
        "/*:<>|\\\"\?\x7F"; /* [0..16] for FAT, [7..16] for exFAT */
    uint32_t dc;

    /* Get logical drive */
    res = mount_volume(&label, &fs, FA_WRITE);
    if (res != FR_OK)
        LEAVE_FF(fs, res);

    if (fs->fs_type == FS_EXFAT) { /* On the exFAT volume */
        memset(dirvn, 0, 22);
        di = 0;
        while ((unsigned)*label >= ' ') { /* Create volume label */
            dc = tchar2uni(&label);   /* Get a Unicode character */
            if (dc >= 0x10000) {
                if (dc == 0xFFFFFFFF || di >= 10) { /* Wrong surrogate or buffer overflow */
                    dc = 0;
                } else {
                    st_word(dirvn + di * 2, (uint16_t)(dc >> 16));
                    di++;
                }
            }
            if (dc == 0 || strchr(&badchr[7], (int)dc) ||
                di >= 11) { /* Check validity of the volume label */
                LEAVE_FF(fs, FR_INVALID_NAME);
            }
            st_word(dirvn + di * 2, (uint16_t)dc);
            di++;
        }
    } else {
        /* On the FAT/FAT32 volume */
        memset(dirvn, ' ', 11);
        di = 0;
        while ((unsigned)*label >= ' ') { /* Create volume label */
            dc = tchar2uni(&label);
            wc = (dc < 0x10000) ? ff_uni2oem(ff_wtoupper(dc), CODEPAGE) : 0;

            if (wc == 0 || strchr(&badchr[0], (int)wc) ||
                di >= (unsigned)((wc >= 0x100) ? 10 : 11)) { /* Reject invalid characters for
                                                            volume label */
                LEAVE_FF(fs, FR_INVALID_NAME);
            }
            if (wc >= 0x100)
                dirvn[di++] = (uint8_t)(wc >> 8);
            dirvn[di++] = (uint8_t)wc;
        }
        if (dirvn[0] == DDEM)
            LEAVE_FF(fs, FR_INVALID_NAME); /* Reject illegal name (heading DDEM) */
        while (di && dirvn[di - 1] == ' ')
            di--; /* Snip trailing spaces */
    }

    /* Set volume label */
    dj.obj.fs = fs;
    dj.obj.sclust = 0; /* Open root directory */
    res = dir_sdi(&dj, 0);
    if (res == FR_OK) {
        res = DIR_READ_LABEL(&dj); /* Get volume label entry */
        if (res == FR_OK) {
            if (fs->fs_type == FS_EXFAT) {
                dj.dir[XDIR_NumLabel] = (uint8_t)di; /* Change the volume label */
                memcpy(dj.dir + XDIR_Label, dirvn, 22);
            } else {
                if (di != 0) {
                    memcpy(dj.dir, dirvn, 11); /* Change the volume label */
                } else {
                    dj.dir[DIR_Name] = DDEM; /* Remove the volume label */
                }
            }
            fs->wflag = 1;
            res = sync_fs(fs);
        } else { /* No volume label entry or an error */
            if (res == FR_NO_FILE) {
                res = FR_OK;
                if (di != 0) {               /* Create a volume label entry */
                    res = dir_alloc(&dj, 1); /* Allocate an entry */
                    if (res == FR_OK) {
                        memset(dj.dir, 0, SZDIRE); /* Clean the entry */
                        if (fs->fs_type == FS_EXFAT) {
                            dj.dir[XDIR_Type] = ET_VLABEL; /* Create volume label entry */
                            dj.dir[XDIR_NumLabel] = (uint8_t)di;
                            memcpy(dj.dir + XDIR_Label, dirvn, 22);
                        } else {
                            dj.dir[DIR_Attr] = AM_VOL; /* Create volume label entry */
                            memcpy(dj.dir, dirvn, 11);
                        }
                        fs->wflag = 1;
                        res = sync_fs(fs);
                    }
                }
            }
        }
    }

    LEAVE_FF(fs, res);
}

#endif /* !FF_FS_READONLY */
#endif /* FF_USE_LABEL */

#if FF_USE_EXPAND && !FF_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Allocate a Contiguous Blocks to the File                              */
/*-----------------------------------------------------------------------*/

fs_result_t f_expand(file_t *fp,     /* Pointer to the file object */
                 fs_size_t fsz, /* File size to be expanded to */
                 uint8_t opt)    /* Operation mode 0:Find and prepare or 1:Find and allocate */
{
    fs_result_t res;
    filesystem_t *fs;
    uint32_t n, clst, stcl, scl, ncl, tcl, lclst;

    res = validate(&fp->obj, &fs); /* Check validity of the file object */
    if (res != FR_OK || (res = (fs_result_t)fp->err) != FR_OK)
        LEAVE_FF(fs, res);
    if (fsz == 0 || fp->obj.objsize != 0 || !(fp->flag & FA_WRITE))
        LEAVE_FF(fs, FR_DENIED);
    if (fs->fs_type != FS_EXFAT && fsz >= 0x100000000)
        LEAVE_FF(fs, FR_DENIED); /* Check if in size limit */

    n = (uint32_t)fs->csize * SS(fs);                      /* Cluster size */
    tcl = (uint32_t)(fsz / n) + ((fsz & (n - 1)) ? 1 : 0); /* Number of clusters required */
    stcl = fs->last_clst;
    lclst = 0;
    if (stcl < 2 || stcl >= fs->n_fatent)
        stcl = 2;

    if (fs->fs_type == FS_EXFAT) {
        scl = find_bitmap(fs, stcl, tcl); /* Find a contiguous cluster block */
        if (scl == 0)
            res = FR_DENIED; /* No contiguous cluster block was found */
        if (scl == 0xFFFFFFFF)
            res = FR_DISK_ERR;
        if (res == FR_OK) {                           /* A contiguous free area is found */
            if (opt) {                                /* Allocate it now */
                res = change_bitmap(fs, scl, tcl, 1); /* Mark the cluster block 'in use' */
                lclst = scl + tcl - 1;
            } else { /* Set it as suggested point for next allocation */
                lclst = scl - 1;
            }
        }
    } else {
        scl = clst = stcl;
        ncl = 0;
        for (;;) { /* Find a contiguous cluster block */
            n = get_fat(&fp->obj, clst);
            if (++clst >= fs->n_fatent)
                clst = 2;
            if (n == 1) {
                res = FR_INT_ERR;
                break;
            }
            if (n == 0xFFFFFFFF) {
                res = FR_DISK_ERR;
                break;
            }
            if (n == 0) { /* Is it a free cluster? */
                if (++ncl == tcl)
                    break; /* Break if a contiguous cluster block is found */
            } else {
                scl = clst;
                ncl = 0; /* Not a free cluster */
            }
            if (clst == stcl) { /* No contiguous cluster? */
                res = FR_DENIED;
                break;
            }
        }
        if (res == FR_OK) {                                 /* A contiguous free area is found */
            if (opt) {                                      /* Allocate it now */
                for (clst = scl, n = tcl; n; clst++, n--) { /* Create a cluster chain on the FAT */
                    res = put_fat(fs, clst, (n == 1) ? 0xFFFFFFFF : clst + 1);
                    if (res != FR_OK)
                        break;
                    lclst = clst;
                }
            } else { /* Set it as suggested point for next allocation */
                lclst = scl - 1;
            }
        }
    }

    if (res == FR_OK) {
        fs->last_clst = lclst;    /* Set suggested start cluster to start next */
        if (opt) {                /* Is it allocated now? */
            fp->obj.sclust = scl; /* Update object allocation information */
            fp->obj.objsize = fsz;
            fp->obj.stat = 2; /* Set status 'contiguous chain' */
            fp->flag |= FA_MODIFIED;
            if (fs->free_clst <= fs->n_fatent - 2) { /* Update FSINFO */
                fs->free_clst -= tcl;
                fs->fsi_flag |= 1;
            }
        }
    }

    LEAVE_FF(fs, res);
}

#endif /* FF_USE_EXPAND && !FF_FS_READONLY */

#if FF_USE_FORWARD
/*-----------------------------------------------------------------------*/
/* Forward Data to the Stream Directly                                   */
/*-----------------------------------------------------------------------*/

fs_result_t f_forward(file_t *fp,                          /* Pointer to the file object */
                  unsigned (*func)(const uint8_t *, unsigned), /* Pointer to the streaming function */
                  unsigned btf,                         /* Number of bytes to forward */
                  unsigned *bf)                         /* Pointer to number of bytes forwarded */
{
    fs_result_t res;
    filesystem_t *fs;
    uint32_t clst;
    fs_lba_t sect;
    fs_size_t remain;
    unsigned rcnt, csect;
    uint8_t *dbuf;

    *bf = 0;                       /* Clear transfer byte counter */
    res = validate(&fp->obj, &fs); /* Check validity of the file object */
    if (res != FR_OK || (res = (fs_result_t)fp->err) != FR_OK)
        LEAVE_FF(fs, res);
    if (!(fp->flag & FA_READ))
        LEAVE_FF(fs, FR_DENIED); /* Check access mode */

    remain = fp->obj.objsize - fp->fptr;
    if (btf > remain)
        btf = (unsigned)remain; /* Truncate btf by remaining bytes */

    for (; btf > 0 && (*func)(0, 0);
         fp->fptr += rcnt, *bf += rcnt,
         btf -= rcnt) { /* Repeat until all data transferred or stream goes busy */
        csect = (unsigned)(fp->fptr / SS(fs) & (fs->csize - 1)); /* Sector offset in the cluster */
        if (fp->fptr % SS(fs) == 0) {                        /* On the sector boundary? */
            if (csect == 0) {                                /* On the cluster boundary? */
                clst = (fp->fptr == 0) ?                     /* On the top of the file? */
                           fp->obj.sclust
                                       : get_fat(&fp->obj, fp->clust);
                if (clst <= 1)
                    ABORT(fs, FR_INT_ERR);
                if (clst == 0xFFFFFFFF)
                    ABORT(fs, FR_DISK_ERR);
                fp->clust = clst; /* Update current cluster */
            }
        }
        sect = clst2sect(fs, fp->clust); /* Get current data sector */
        if (sect == 0)
            ABORT(fs, FR_INT_ERR);
        sect += csect;
#if FF_FS_TINY
        if (move_window(fs, sect) != FR_OK)
            ABORT(fs, FR_DISK_ERR); /* Move sector window to the file data */
        dbuf = fs->win;
#else
        if (fp->sect != sect) {        /* Fill sector cache with file data */
#if !FF_FS_READONLY
            if (fp->flag & FA_DIRTY) { /* Write-back dirty sector cache */
                if (disk_write(fs->pdrv, fp->buf, fp->sect, 1) != DISK_OK)
                    ABORT(fs, FR_DISK_ERR);
                fp->flag &= (uint8_t)~FA_DIRTY;
            }
#endif
            if (disk_read(fs->pdrv, fp->buf, sect, 1) != DISK_OK)
                ABORT(fs, FR_DISK_ERR);
        }
        dbuf = fp->buf;
#endif
        fp->sect = sect;
        rcnt = SS(fs) - (unsigned)fp->fptr % SS(fs); /* Number of bytes remains in the sector */
        if (rcnt > btf)
            rcnt = btf;                                         /* Clip it by btr if needed */
        rcnt = (*func)(dbuf + ((unsigned)fp->fptr % SS(fs)), rcnt); /* Forward the file data */
        if (rcnt == 0)
            ABORT(fs, FR_INT_ERR);
    }

    LEAVE_FF(fs, FR_OK);
}
#endif /* FF_USE_FORWARD */

#if !FF_FS_READONLY && FF_USE_MKFS
/*-----------------------------------------------------------------------*/
/* Create FAT/exFAT volume (with sub-functions)                          */
/*-----------------------------------------------------------------------*/

#define N_SEC_TRACK 63     /* Sectors per track for determination of drive CHS */
#define GPT_ALIGN 0x100000 /* Alignment of partitions in GPT [byte] (>=128KB) */
#define GPT_ITEMS 128      /* Number of GPT table size (>=128, sector aligned) */

/* Create partitions on the physical drive in format of MBR or GPT */

static fs_result_t create_partition(uint8_t drv,           /* Physical drive number */
                                const fs_lba_t plst[], /* Partition list */
                                uint8_t sys,  /* System ID for each partition (for only MBR) */
                                uint8_t *buf) /* Working buffer for a sector */
{
    unsigned i, cy;
    fs_lba_t sz_drv;
    uint32_t sz_drv32, nxt_alloc32, sz_part32;
    uint8_t *pte;
    uint8_t hd, n_hd, sc, n_sc;

    /* Get physical drive size */
    if (disk_ioctl(drv, GET_SECTOR_COUNT, &sz_drv) != DISK_OK)
        return FR_DISK_ERR;

    /* Create partitions in MBR format */
    sz_drv32 = (uint32_t)sz_drv;
    n_sc = N_SEC_TRACK; /* Determine drive CHS without any consideration
                           of the drive geometry */
    for (n_hd = 8; n_hd != 0 && sz_drv32 / n_hd / n_sc > 1024; n_hd *= 2)
        ;
    if (n_hd == 0)
        n_hd = 255; /* Number of heads needs to be <256 */

    memset(buf, 0, FF_MAX_SS); /* Clear MBR */
    pte = buf + MBR_Table;     /* Partition table in the MBR */
    for (i = 0, nxt_alloc32 = n_sc; i < 4 && nxt_alloc32 != 0 && nxt_alloc32 < sz_drv32;
         i++, nxt_alloc32 += sz_part32) {
        sz_part32 = (uint32_t)plst[i]; /* Get partition size */
        if (sz_part32 <= 100)
            sz_part32 = (sz_part32 == 100)
                            ? sz_drv32
                            : sz_drv32 / 100 * sz_part32; /* Size in percentage? */
        if (nxt_alloc32 + sz_part32 > sz_drv32 || nxt_alloc32 + sz_part32 < nxt_alloc32)
            sz_part32 = sz_drv32 - nxt_alloc32; /* Clip at drive size */
        if (sz_part32 == 0)
            break; /* End of table or no sector to allocate? */

        st_dword(pte + PTE_StLba, nxt_alloc32); /* Start LBA */
        st_dword(pte + PTE_SizLba, sz_part32);  /* Number of sectors */
        pte[PTE_System] = sys;                  /* System type */

        cy = (unsigned)(nxt_alloc32 / n_sc / n_hd); /* Start cylinder */
        hd = (uint8_t)(nxt_alloc32 / n_sc % n_hd); /* Start head */
        sc = (uint8_t)(nxt_alloc32 % n_sc + 1);    /* Start sector */
        pte[PTE_StHead] = hd;
        pte[PTE_StSec] = (uint8_t)((cy >> 2 & 0xC0) | sc);
        pte[PTE_StCyl] = (uint8_t)cy;

        cy = (unsigned)((nxt_alloc32 + sz_part32 - 1) / n_sc / n_hd); /* End cylinder */
        hd = (uint8_t)((nxt_alloc32 + sz_part32 - 1) / n_sc % n_hd); /* End head */
        sc = (uint8_t)((nxt_alloc32 + sz_part32 - 1) % n_sc + 1);    /* End sector */
        pte[PTE_EdHead] = hd;
        pte[PTE_EdSec] = (uint8_t)((cy >> 2 & 0xC0) | sc);
        pte[PTE_EdCyl] = (uint8_t)cy;

        pte += SZ_PTE; /* Next entry */
    }

    st_word(buf + BS_55AA, 0xAA55); /* MBR signature */
    if (disk_write(drv, buf, 0, 1) != DISK_OK)
        return FR_DISK_ERR; /* Write it to the MBR */

    return FR_OK;
}

fs_result_t f_mkfs(const char *path, // Logical drive number
                   uint8_t fmt,      // Format option (FM_FAT, FM_FAT32, FM_EXFAT and FM_SFD)
                   void *work,       // Pointer to working buffer (null: use len bytes of heap memory)
                   unsigned len)     // Size of working buffer [byte]
{
    static const uint16_t cst[] = {
        1, 4, 16, 64, 256, 512, 0
    }; /* Cluster size boundary for FAT volume (4Ks unit) */
    static const uint16_t cst32[] = {
        1, 2, 4, 8, 16, 32, 0
    }; /* Cluster size boundary for FAT32 volume (128Ks unit) */
    static const mkfs_parm_t defopt = { 0, 0, 0, 0 }; /* Default parameter */
    uint8_t fsopt, fsty, sys, pdrv;
    uint8_t *buf;
    uint16_t ss; /* Sector size */
    uint32_t sz_buf, sz_blk, n_clst, pau, nsect, n, vsn;
    fs_lba_t sz_vol, b_vol, b_fat, b_data; /* Size of volume, Base LBA of volume, fat, data */
    fs_lba_t sect, lba[2];
    uint32_t sz_rsv, sz_fat, sz_dir, sz_au; /* Size of reserved, fat, dir, data, cluster */
    unsigned n_fat, n_root, i;               /* Index, Number of FATs and Number of roor dir entries */
    int vol;
    media_status_t ds;
    fs_result_t res;

    /* Check mounted drive and clear work area */
    vol = get_ldnumber(&path); /* Get target logical drive */
    if (vol < 0)
        return FR_INVALID_DRIVE;
    if (FatFs[vol])
        FatFs[vol]->fs_type = 0; /* Clear the fs object if mounted */
    pdrv = LD2PD(vol);           /* Hosting physical drive */

    /* Initialize the hosting physical drive */
    ds = disk_initialize(pdrv);
    if (ds & MEDIA_NOINIT)
        return FR_NOT_READY;
    if (ds & MEDIA_PROTECT)
        return FR_WRITE_PROTECTED;

    /* Get physical drive parameters (sz_drv, sz_blk and ss) */
    const mkfs_parm_t *opt = &defopt; /* Use default parameter */
    sz_blk = opt->align;
    if (sz_blk == 0)
        disk_ioctl(pdrv, GET_BLOCK_SIZE, &sz_blk); /* Block size from the paramter or lower layer */
    if (sz_blk == 0 || sz_blk > 0x8000 || (sz_blk & (sz_blk - 1)))
        sz_blk = 1; /* Use default if the block size is invalid */
#if FF_MAX_SS != FF_MIN_SS
    if (disk_ioctl(pdrv, GET_SECTOR_SIZE, &ss) != DISK_OK)
        return FR_DISK_ERR;
    if (ss > FF_MAX_SS || ss < FF_MIN_SS || (ss & (ss - 1)))
        return FR_DISK_ERR;
#else
    ss = FF_MAX_SS;
#endif

    /* Options for FAT sub-type and FAT parameters */
    fsopt = fmt & (FM_ANY | FM_SFD);
    n_fat = (opt->n_fat >= 1 && opt->n_fat <= 2) ? opt->n_fat : 1;
    n_root = (opt->n_root >= 1 && opt->n_root <= 32768 && (opt->n_root % (ss / SZDIRE)) == 0)
                 ? opt->n_root
                 : 512;
    sz_au =
        (opt->au_size <= 0x1000000 && (opt->au_size & (opt->au_size - 1)) == 0) ? opt->au_size : 0;
    sz_au /= ss; /* Byte --> Sector */

    /* Get working buffer */
    sz_buf = len / ss; /* Size of working buffer [sector] */
    if (sz_buf == 0)
        return FR_NOT_ENOUGH_CORE;
    buf = (uint8_t *)work; /* Working buffer */
#if FF_USE_LFN == 3
    if (!buf)
        buf = ff_memalloc(sz_buf * ss); /* Use heap memory for working buffer */
#endif
    if (!buf)
        return FR_NOT_ENOUGH_CORE;

    /* Determine where the volume to be located (b_vol, sz_vol) */
    b_vol = sz_vol = 0;
    if (disk_ioctl(pdrv, GET_SECTOR_COUNT, &sz_vol) != DISK_OK)
        LEAVE_MKFS(FR_DISK_ERR);
    if (!(fsopt & FM_SFD)) { /* To be partitioned? */
                             /* Create a single-partition on the drive in this function */
        /* Partitioning is in MBR */
        if (sz_vol > N_SEC_TRACK) {
            b_vol = N_SEC_TRACK;
            sz_vol -= b_vol; /* Estimated partition offset and size */
        }
    }
    if (sz_vol < 128)
        LEAVE_MKFS(FR_MKFS_ABORTED); /* Check if volume size is >=128s */

    /* Now start to create an FAT volume at b_vol and sz_vol */

    do {                                         /* Pre-determine the FAT type */
        if (fsopt & FM_EXFAT) { /* exFAT possible? */
            if ((fsopt & FM_ANY) == FM_EXFAT || sz_vol >= 0x4000000 ||
                sz_au > 128) { /* exFAT only, vol >= 64MS or sz_au > 128S ? */
                fsty = FS_EXFAT;
                break;
            }
        }
        if (sz_au > 128)
            sz_au = 128;             /* Invalid AU for FAT/FAT32? */
        if (fsopt & FM_FAT32) {      /* FAT32 possible? */
            if (!(fsopt & FM_FAT)) { /* no-FAT? */
                fsty = FS_FAT32;
                break;
            }
        }
        if (!(fsopt & FM_FAT))
            LEAVE_MKFS(FR_INVALID_PARAMETER); /* no-FAT? */
        fsty = FS_FAT16;
    } while (0);

    vsn = (uint32_t)sz_vol + GET_FATTIME(); /* VSN generated from current time
                                            and partitiion size */

    if (fsty == FS_EXFAT) { /* Create an exFAT volume */
        uint32_t szb_bit, szb_case, sum, nbit, clu, clen[3];
        uint16_t ch, si;
        unsigned j, st;

        if (sz_vol < 2*128)
            LEAVE_MKFS(FR_MKFS_ABORTED); /* Too small volume for exFAT? */
#if FF_USE_TRIM
        lba[0] = b_vol;
        lba[1] = b_vol + sz_vol - 1; /* Inform storage device that the volume area may be erased */
        disk_ioctl(pdrv, CTRL_TRIM, lba);
#endif
        /* Determine FAT location, data location and number of clusters */
        if (sz_au == 0) { /* AU auto-selection */
            sz_au = 8;
            if (sz_vol >= 0x80000)
                sz_au = 64; /* >= 512Ks */
            if (sz_vol >= 0x4000000)
                sz_au = 256; /* >= 64Ms */
        }
        b_fat = b_vol + 32;                                       /* FAT start at offset 32 */
        sz_fat = (uint32_t)((sz_vol / sz_au + 2) * 4 + ss - 1) / ss; /* Number of FAT sectors */
        b_data = (b_fat + sz_fat + sz_blk - 1) &
                 ~((fs_lba_t)sz_blk - 1); /* Align data area to the erase block boundary */
        if (b_data - b_vol >= sz_vol / 2)
            LEAVE_MKFS(FR_MKFS_ABORTED);                       /* Too small volume? */
        n_clst = (uint32_t)((sz_vol - (b_data - b_vol)) / sz_au); /* Number of clusters */
        if (n_clst < 16)
            LEAVE_MKFS(FR_MKFS_ABORTED); /* Too few clusters? */
        if (n_clst > MAX_EXFAT)
            LEAVE_MKFS(FR_MKFS_ABORTED); /* Too many clusters? */

        szb_bit = (n_clst + 7) / 8; /* Size of allocation bitmap */
        clen[0] =
            (szb_bit + sz_au * ss - 1) / (sz_au * ss); /* Number of allocation bitmap clusters */

        /* Create a compressed up-case table */
        sect = b_data + sz_au * clen[0]; /* Table start sector */
        sum = 0;                         /* Table checksum to be stored in the 82 entry */
        st = 0;
        si = 0;
        i = 0;
        j = 0;
        szb_case = 0;
        do {
            switch (st) {
            case 0:
                ch = (uint16_t)ff_wtoupper(si); /* Get an up-case char */
                if (ch != si) {
                    si++;
                    break; /* Store the up-case char if exist */
                }
                for (j = 1; (uint16_t)(si + j) && (uint16_t)(si + j) == ff_wtoupper((uint16_t)(si + j)); j++)
                    ; /* Get run length of no-case block */
                if (j >= 128) {
                    ch = 0xFFFF;
                    st = 2;
                    break; /* Compress the no-case block if run is >= 128
                              chars */
                }
                st = 1; /* Do not compress short run */
                        /* FALLTHROUGH */
            case 1:
                ch = si++; /* Fill the short run */
                if (--j == 0)
                    st = 0;
                break;

            default:
                ch = (uint16_t)j;
                si += (uint16_t)j; /* Number of chars to skip */
                st = 0;
            }
            sum = xsum32(buf[i + 0] = (uint8_t)ch, sum); /* Put it into the write buffer */
            sum = xsum32(buf[i + 1] = (uint8_t)(ch >> 8), sum);
            i += 2;
            szb_case += 2;
            if (si == 0 || i == sz_buf * ss) { /* Write buffered data when buffer full
                                                  or end of process */
                n = (i + ss - 1) / ss;
                if (disk_write(pdrv, buf, sect, n) != DISK_OK)
                    LEAVE_MKFS(FR_DISK_ERR);
                sect += n;
                i = 0;
            }
        } while (si);
        clen[1] = (szb_case + sz_au * ss - 1) / (sz_au * ss); /* Number of up-case table clusters */
        clen[2] = 1;                                          /* Number of root dir clusters */

        /* Initialize the allocation bitmap */
        sect = b_data;
        nsect = (szb_bit + ss - 1) / ss;    /* Start of bitmap and number of bitmap sectors */
        nbit = clen[0] + clen[1] + clen[2]; /* Number of clusters in-use by system (bitmap,
                                               up-case and root-dir) */
        do {
            memset(buf, 0, sz_buf * ss); /* Initialize bitmap buffer */
            for (i = 0; nbit != 0 && i / 8 < sz_buf * ss; buf[i / 8] |= 1 << (i % 8), i++, nbit--)
                ;                                  /* Mark used clusters */
            n = (nsect > sz_buf) ? sz_buf : nsect; /* Write the buffered data */
            if (disk_write(pdrv, buf, sect, n) != DISK_OK)
                LEAVE_MKFS(FR_DISK_ERR);
            sect += n;
            nsect -= n;
        } while (nsect);

        /* Initialize the FAT */
        sect = b_fat;
        nsect = sz_fat; /* Start of FAT and number of FAT sectors */
        j = nbit = clu = 0;
        do {
            memset(buf, 0, sz_buf * ss);
            i = 0;          /* Clear work area and reset write offset */
            if (clu == 0) { /* Initialize FAT [0] and FAT[1] */
                st_dword(buf + i, 0xFFFFFFF8);
                i += 4;
                clu++;
                st_dword(buf + i, 0xFFFFFFFF);
                i += 4;
                clu++;
            }
            do { /* Create chains of bitmap, up-case and root dir */
                while (nbit != 0 && i < sz_buf * ss) { /* Create a chain */
                    st_dword(buf + i, (nbit > 1) ? clu + 1 : 0xFFFFFFFF);
                    i += 4;
                    clu++;
                    nbit--;
                }
                if (nbit == 0 && j < 3)
                    nbit = clen[j++]; /* Get next chain length */
            } while (nbit != 0 && i < sz_buf * ss);
            n = (nsect > sz_buf) ? sz_buf : nsect; /* Write the buffered data */
            if (disk_write(pdrv, buf, sect, n) != DISK_OK)
                LEAVE_MKFS(FR_DISK_ERR);
            sect += n;
            nsect -= n;
        } while (nsect);

        /* Initialize the root directory */
        memset(buf, 0, sz_buf * ss);
        buf[SZDIRE * 0 + 0] = ET_VLABEL;              /* Volume label entry (no label) */
        buf[SZDIRE * 1 + 0] = ET_BITMAP;              /* Bitmap entry */
        st_dword(buf + SZDIRE * 1 + 20, 2);           /*  cluster */
        st_dword(buf + SZDIRE * 1 + 24, szb_bit);     /*  size */
        buf[SZDIRE * 2 + 0] = ET_UPCASE;              /* Up-case table entry */
        st_dword(buf + SZDIRE * 2 + 4, sum);          /*  sum */
        st_dword(buf + SZDIRE * 2 + 20, 2 + clen[0]); /*  cluster */
        st_dword(buf + SZDIRE * 2 + 24, szb_case);    /*  size */
        sect = b_data + sz_au * (clen[0] + clen[1]);
        nsect = sz_au; /* Start of the root directory and number of sectors */
        do {           /* Fill root directory sectors */
            n = (nsect > sz_buf) ? sz_buf : nsect;
            if (disk_write(pdrv, buf, sect, n) != DISK_OK)
                LEAVE_MKFS(FR_DISK_ERR);
            memset(buf, 0, ss); /* Rest of entries are filled with zero */
            sect += n;
            nsect -= n;
        } while (nsect);

        /* Create two set of the exFAT VBR blocks */
        sect = b_vol;
        for (n = 0; n < 2; n++) {
            /* Main record (+0) */
            memset(buf, 0, ss);
            memcpy(buf + BS_JmpBoot,
                   "\xEB\x76\x90"
                   "EXFAT   ",
                   11);                           /* Boot jump code (x86), OEM name */
            st_qword(buf + BPB_VolOfsEx, b_vol);  /* Volume offset in the physical drive [sector] */
            st_qword(buf + BPB_TotSecEx, sz_vol); /* Volume size [sector] */
            st_dword(buf + BPB_FatOfsEx, (uint32_t)(b_fat - b_vol));   /* FAT offset [sector] */
            st_dword(buf + BPB_FatSzEx, sz_fat);                    /* FAT size [sector] */
            st_dword(buf + BPB_DataOfsEx, (uint32_t)(b_data - b_vol)); /* Data offset [sector] */
            st_dword(buf + BPB_NumClusEx, n_clst);                  /* Number of clusters */
            st_dword(buf + BPB_RootClusEx, 2 + clen[0] + clen[1]);  /* Root dir cluster # */
            st_dword(buf + BPB_VolIDEx, vsn);                       /* VSN */
            st_word(buf + BPB_FSVerEx, 0x100);                      /* Filesystem version (1.00) */
            for (buf[BPB_BytsPerSecEx] = 0, i = ss; i >>= 1; buf[BPB_BytsPerSecEx]++)
                ; /* Log2 of sector size [byte] */
            for (buf[BPB_SecPerClusEx] = 0, i = sz_au; i >>= 1; buf[BPB_SecPerClusEx]++)
                ;                                 /* Log2 of cluster size [sector] */
            buf[BPB_NumFATsEx] = 1;               /* Number of FATs */
            buf[BPB_DrvNumEx] = 0x80;             /* Drive number (for int13) */
            st_word(buf + BS_BootCodeEx, 0xFEEB); /* Boot code (x86) */
            st_word(buf + BS_55AA, 0xAA55);       /* Signature (placed here
                                                     regardless of sector size) */
            for (i = sum = 0; i < ss; i++) {      /* VBR checksum */
                if (i != BPB_VolFlagEx && i != BPB_VolFlagEx + 1 && i != BPB_PercInUseEx)
                    sum = xsum32(buf[i], sum);
            }
            if (disk_write(pdrv, buf, sect++, 1) != DISK_OK)
                LEAVE_MKFS(FR_DISK_ERR);
            /* Extended bootstrap record (+1..+8) */
            memset(buf, 0, ss);
            st_word(buf + ss - 2, 0xAA55); /* Signature (placed at end of sector) */
            for (j = 1; j < 9; j++) {
                for (i = 0; i < ss; sum = xsum32(buf[i++], sum))
                    ; /* VBR checksum */
                if (disk_write(pdrv, buf, sect++, 1) != DISK_OK)
                    LEAVE_MKFS(FR_DISK_ERR);
            }
            /* OEM/Reserved record (+9..+10) */
            memset(buf, 0, ss);
            for (; j < 11; j++) {
                for (i = 0; i < ss; sum = xsum32(buf[i++], sum))
                    ; /* VBR checksum */
                if (disk_write(pdrv, buf, sect++, 1) != DISK_OK)
                    LEAVE_MKFS(FR_DISK_ERR);
            }
            /* Sum record (+11) */
            for (i = 0; i < ss; i += 4)
                st_dword(buf + i, sum); /* Fill with checksum value */
            if (disk_write(pdrv, buf, sect++, 1) != DISK_OK)
                LEAVE_MKFS(FR_DISK_ERR);
        }

    } else {
        /* Create an FAT/FAT32 volume */
        do {
            pau = sz_au;
            /* Pre-determine number of clusters and FAT sub-type */
            if (fsty == FS_FAT32) {              /* FAT32 volume */
                if (pau == 0) {                  /* AU auto-selection */
                    n = (uint32_t)sz_vol / 0x20000; /* Volume size in unit of 128KS */
                    for (i = 0, pau = 1; cst32[i] && cst32[i] <= n; i++, pau <<= 1)
                        ; /* Get from table */
                }
                n_clst = (uint32_t)sz_vol / pau;            /* Number of clusters */
                sz_fat = (n_clst * 4 + 8 + ss - 1) / ss; /* FAT size [sector] */
                sz_rsv = 32;                             /* Number of reserved sectors */
                sz_dir = 0;                              /* No static directory */
                if (n_clst <= MAX_FAT16 || n_clst > MAX_FAT32)
                    LEAVE_MKFS(FR_MKFS_ABORTED);
            } else {                            /* FAT volume */
                if (pau == 0) {                 /* au auto-selection */
                    n = (uint32_t)sz_vol / 0x1000; /* Volume size in unit of 4KS */
                    for (i = 0, pau = 1; cst[i] && cst[i] <= n; i++, pau <<= 1)
                        ; /* Get from table */
                }
                n_clst = (uint32_t)sz_vol / pau;
                if (n_clst > MAX_FAT12) {
                    n = n_clst * 2 + 4; /* FAT size [byte] */
                } else {
                    fsty = FS_FAT12;
                    n = (n_clst * 3 + 1) / 2 + 3; /* FAT size [byte] */
                }
                sz_fat = (n + ss - 1) / ss;           /* FAT size [sector] */
                sz_rsv = 1;                           /* Number of reserved sectors */
                sz_dir = (uint32_t)n_root * SZDIRE / ss; /* Root dir size [sector] */
            }
            b_fat = b_vol + sz_rsv;                   /* FAT base */
            b_data = b_fat + sz_fat * n_fat + sz_dir; /* Data base */

            /* Align data area to erase block boundary (for flash memory
             * media) */
            n = (uint32_t)(((b_data + sz_blk - 1) & ~(sz_blk - 1)) -
                        b_data);    /* Sectors to next nearest from current data
                                       base */
            if (fsty == FS_FAT32) { /* FAT32: Move FAT */
                sz_rsv += n;
                b_fat += n;
            } else {             /* FAT: Expand FAT */
                if (n % n_fat) { /* Adjust fractional error if needed */
                    n--;
                    sz_rsv++;
                    b_fat++;
                }
                sz_fat += n / n_fat;
            }

            /* Determine number of clusters and final check of validity of
             * the FAT sub-type */
            if (sz_vol < b_data + pau * 16 - b_vol)
                LEAVE_MKFS(FR_MKFS_ABORTED); /* Too small volume? */
            n_clst = ((uint32_t)sz_vol - sz_rsv - sz_fat * n_fat - sz_dir) / pau;
            if (fsty == FS_FAT32) {
                if (n_clst <= MAX_FAT16) { /* Too few clusters for FAT32? */
                    if (sz_au == 0 && (sz_au = pau / 2) != 0)
                        continue; /* Adjust cluster size and retry */
                    LEAVE_MKFS(FR_MKFS_ABORTED);
                }
            }
            if (fsty == FS_FAT16) {
                if (n_clst > MAX_FAT16) { /* Too many clusters for FAT16 */
                    if (sz_au == 0 && (pau * 2) <= 64) {
                        sz_au = pau * 2;
                        continue; /* Adjust cluster size and retry */
                    }
                    if ((fsopt & FM_FAT32)) {
                        fsty = FS_FAT32;
                        continue; /* Switch type to FAT32 and retry */
                    }
                    if (sz_au == 0 && (sz_au = pau * 2) <= 128)
                        continue; /* Adjust cluster size and retry */
                    LEAVE_MKFS(FR_MKFS_ABORTED);
                }
                if (n_clst <= MAX_FAT12) { /* Too few clusters for FAT16 */
                    if (sz_au == 0 && (sz_au = pau * 2) <= 128)
                        continue; /* Adjust cluster size and retry */
                    LEAVE_MKFS(FR_MKFS_ABORTED);
                }
            }
            if (fsty == FS_FAT12 && n_clst > MAX_FAT12)
                LEAVE_MKFS(FR_MKFS_ABORTED); /* Too many clusters for FAT12 */

            /* Ok, it is the valid cluster configuration */
            break;
        } while (1);

#if FF_USE_TRIM
        lba[0] = b_vol;
        lba[1] = b_vol + sz_vol - 1; /* Inform storage device that the volume area may be erased */
        disk_ioctl(pdrv, CTRL_TRIM, lba);
#endif
        /* Create FAT VBR */
        memset(buf, 0, ss);
        memcpy(buf + BS_JmpBoot,
               "\xEB\xFE\x90"
               "MSDOS5.0",
               11);                                  /* Boot jump code (x86), OEM name */
        st_word(buf + BPB_BytsPerSec, ss);           /* Sector size [byte] */
        buf[BPB_SecPerClus] = (uint8_t)pau;             /* Cluster size [sector] */
        st_word(buf + BPB_RsvdSecCnt, (uint16_t)sz_rsv); /* Size of reserved area */
        buf[BPB_NumFATs] = (uint8_t)n_fat;              /* Number of FATs */
        st_word(buf + BPB_RootEntCnt,
                (uint16_t)((fsty == FS_FAT32) ? 0 : n_root)); /* Number of root directory entries */
        if (sz_vol < 0x10000) {
            st_word(buf + BPB_TotSec16, (uint16_t)sz_vol); /* Volume size in 16-bit LBA */
        } else {
            st_dword(buf + BPB_TotSec32, (uint32_t)sz_vol); /* Volume size in 32-bit LBA */
        }
        buf[BPB_Media] = 0xF8;            /* Media descriptor byte */
        st_word(buf + BPB_SecPerTrk, 63); /* Number of sectors per track (for int13) */
        st_word(buf + BPB_NumHeads, 255); /* Number of heads (for int13) */
        st_dword(buf + BPB_HiddSec,
                 (uint32_t)b_vol); /* Volume offset in the physical drive [sector] */
        if (fsty == FS_FAT32) {
            st_dword(buf + BS_VolID32, vsn);     /* VSN */
            st_dword(buf + BPB_FATSz32, sz_fat); /* FAT size [sector] */
            st_dword(buf + BPB_RootClus32, 2);   /* Root directory cluster # (2) */
            st_word(buf + BPB_FSInfo32, 1);      /* Offset of FSINFO sector (VBR + 1) */
            st_word(buf + BPB_BkBootSec32, 6);   /* Offset of backup VBR (VBR + 6) */
            buf[BS_DrvNum32] = 0x80;             /* Drive number (for int13) */
            buf[BS_BootSig32] = 0x29;            /* Extended boot signature */
            memcpy(buf + BS_VolLab32,
                   "NO NAME    "
                   "FAT32   ",
                   19); /* Volume label, FAT signature */
        } else {
            st_dword(buf + BS_VolID, vsn);            /* VSN */
            st_word(buf + BPB_FATSz16, (uint16_t)sz_fat); /* FAT size [sector] */
            buf[BS_DrvNum] = 0x80;                    /* Drive number (for int13) */
            buf[BS_BootSig] = 0x29;                   /* Extended boot signature */
            memcpy(buf + BS_VolLab,
                   "NO NAME    "
                   "FAT     ",
                   19); /* Volume label, FAT signature */
        }
        st_word(buf + BS_55AA, 0xAA55); /* Signature (offset is fixed here
                                           regardless of sector size) */
        if (disk_write(pdrv, buf, b_vol, 1) != DISK_OK)
            LEAVE_MKFS(FR_DISK_ERR); /* Write it to the VBR sector */

        /* Create FSINFO record if needed */
        if (fsty == FS_FAT32) {
            disk_write(pdrv, buf, b_vol + 6, 1); /* Write backup VBR (VBR + 6) */
            memset(buf, 0, ss);
            st_dword(buf + FSI_LeadSig, 0x41615252);
            st_dword(buf + FSI_StrucSig, 0x61417272);
            st_dword(buf + FSI_Free_Count, n_clst - 1); /* Number of free clusters */
            st_dword(buf + FSI_Nxt_Free, 2);            /* Last allocated cluster# */
            st_word(buf + BS_55AA, 0xAA55);
            disk_write(pdrv, buf, b_vol + 7, 1); /* Write backup FSINFO (VBR + 7) */
            disk_write(pdrv, buf, b_vol + 1, 1); /* Write original FSINFO (VBR + 1) */
        }

        /* Initialize FAT area */
        memset(buf, 0, sz_buf * ss);
        sect = b_fat;                 /* FAT start sector */
        for (i = 0; i < n_fat; i++) { /* Initialize FATs each */
            if (fsty == FS_FAT32) {
                st_dword(buf + 0, 0xFFFFFFF8); /* FAT[0] */
                st_dword(buf + 4, 0xFFFFFFFF); /* FAT[1] */
                st_dword(buf + 8, 0x0FFFFFFF); /* FAT[2] (root directory) */
            } else {
                st_dword(buf + 0,
                         (fsty == FS_FAT12) ? 0xFFFFF8 : 0xFFFFFFF8); /* FAT[0] and FAT[1] */
            }
            nsect = sz_fat; /* Number of FAT sectors */
            do {            /* Fill FAT sectors */
                n = (nsect > sz_buf) ? sz_buf : nsect;
                if (disk_write(pdrv, buf, sect, (unsigned)n) != DISK_OK)
                    LEAVE_MKFS(FR_DISK_ERR);
                memset(buf, 0, ss); /* Rest of FAT all are cleared */
                sect += n;
                nsect -= n;
            } while (nsect);
        }

        /* Initialize root directory (fill with zero) */
        nsect = (fsty == FS_FAT32) ? pau : sz_dir; /* Number of root directory sectors */
        do {
            n = (nsect > sz_buf) ? sz_buf : nsect;
            if (disk_write(pdrv, buf, sect, (unsigned)n) != DISK_OK)
                LEAVE_MKFS(FR_DISK_ERR);
            sect += n;
            nsect -= n;
        } while (nsect);
    }

    /* A FAT volume has been created here */

    /* Determine system ID in the MBR partition table */
    if (fsty == FS_EXFAT) {
        sys = 0x07; /* exFAT */
    } else if (fsty == FS_FAT32) {
        sys = 0x0C; /* FAT32X */
    } else if (sz_vol >= 0x10000) {
        sys = 0x06; /* FAT12/16 (large) */
    } else if (fsty == FS_FAT16) {
        sys = 0x04; /* FAT16 */
    } else {
        sys = 0x01; /* FAT12 */
    }

    /* Update partition information */
    if (!(fsopt & FM_SFD)) { /* Create partition table if not in SFD format */
        lba[0] = sz_vol;
        lba[1] = 0;
        res = create_partition(pdrv, lba, sys, buf);
        if (res != FR_OK)
            LEAVE_MKFS(res);
    }

    if (disk_ioctl(pdrv, CTRL_SYNC, 0) != DISK_OK)
        LEAVE_MKFS(FR_DISK_ERR);

    LEAVE_MKFS(FR_OK);
}

#endif /* !FF_FS_READONLY && FF_USE_MKFS */

#if FF_USE_STRFUNC

/*-----------------------------------------------------------------------*/
/* Get a String from the File                                            */
/*-----------------------------------------------------------------------*/

char *f_gets(char *buff, /* Pointer to the buffer to store read string */
              int len,     /* Size of string buffer (items) */
              file_t *fp)     /* Pointer to the file object */
{
    int nc = 0;
    char *p = buff;
    uint8_t s[4];
    unsigned rc;
    uint32_t dc;
    unsigned ct;

    /* With code conversion (Unicode API) */
    /* Make a room for the character and terminator  */
    len -= 4;

    while (nc < len) {
        /* Read a character in UTF-8 */
        f_read(fp, s, 1, &rc); /* Get a code unit */
        if (rc != 1)
            break; /* EOF? */
        dc = s[0];
        if (dc >= 0x80) { /* Multi-byte sequence? */
            ct = 0;
            if ((dc & 0xE0) == 0xC0) { /* 2-byte sequence? */
                dc &= 0x1F;
                ct = 1;
            }
            if ((dc & 0xF0) == 0xE0) { /* 3-byte sequence? */
                dc &= 0x0F;
                ct = 2;
            }
            if ((dc & 0xF8) == 0xF0) { /* 4-byte sequence? */
                dc &= 0x07;
                ct = 3;
            }
            if (ct == 0)
                continue;
            f_read(fp, s, ct, &rc); /* Get trailing bytes */
            if (rc != ct)
                break;
            rc = 0;
            do { /* Merge the byte sequence */
                if ((s[rc] & 0xC0) != 0x80)
                    break;
                dc = dc << 6 | (s[rc] & 0x3F);
            } while (++rc < ct);
            if (rc != ct || dc < 0x80 || IsSurrogate(dc) || dc >= 0x110000)
                continue; /* Wrong encoding? */
        }
        /* A code point is avaialble in dc to be output */

        if (FF_USE_STRFUNC == 2 && dc == '\r')
            continue;                               /* Strip \r off if needed */

        /* Output it in UTF-8 encoding */
        if (dc < 0x80) { /* Single byte? */
            *p++ = (char)dc;
            nc++;
            if (dc == '\n')
                break;           /* End of line? */
        } else if (dc < 0x800) { /* 2-byte sequence? */
            *p++ = (char)(0xC0 | (dc >> 6 & 0x1F));
            *p++ = (char)(0x80 | (dc >> 0 & 0x3F));
            nc += 2;
        } else if (dc < 0x10000) { /* 3-byte sequence? */
            *p++ = (char)(0xE0 | (dc >> 12 & 0x0F));
            *p++ = (char)(0x80 | (dc >> 6 & 0x3F));
            *p++ = (char)(0x80 | (dc >> 0 & 0x3F));
            nc += 3;
        } else { /* 4-byte sequence */
            *p++ = (char)(0xF0 | (dc >> 18 & 0x07));
            *p++ = (char)(0x80 | (dc >> 12 & 0x3F));
            *p++ = (char)(0x80 | (dc >> 6 & 0x3F));
            *p++ = (char)(0x80 | (dc >> 0 & 0x3F));
            nc += 4;
        }
    }

    *p = 0;               /* Terminate the string */
    return nc ? buff : 0; /* When no data read due to EOF or error, return
                             with error. */
}

#if !FF_FS_READONLY
#include <stdarg.h>
#define SZ_PUTC_BUF 64
#define SZ_NUM_BUF 32

/*-----------------------------------------------------------------------*/
/* Put a Character to the File (with sub-functions)                      */
/*-----------------------------------------------------------------------*/

/* Output buffer and work area */

typedef struct {
    file_t *fp;       /* Ptr to the writing file */
    int idx, nchr; /* Write index of buf[] (-1:error), number of encoding
                      units written */
    uint8_t bs[4];
    unsigned wi, ct;
    uint8_t buf[SZ_PUTC_BUF]; /* Write buffer */
} putbuff;

/* Buffered file write with code conversion */

static void putc_bfd(putbuff *pb, char c)
{
    unsigned n;
    int i, nc;
    uint16_t hs, wc;
    uint32_t dc;
    const char *tp;

    if (FF_USE_STRFUNC == 2 && c == '\n') { /* LF -> CRLF conversion */
        putc_bfd(pb, '\r');
    }

    i = pb->idx; /* Write index of pb->buf[] */
    if (i < 0)
        return;    /* In write error? */
    nc = pb->nchr; /* Write unit counter */

    /* UTF-8 input */
    for (;;) {
        if (pb->ct == 0) {                /* Out of multi-byte sequence? */
            pb->bs[pb->wi = 0] = (uint8_t)c; /* Save 1st byte */
            if ((uint8_t)c < 0x80)
                break; /* Single byte code? */
            if (((uint8_t)c & 0xE0) == 0xC0)
                pb->ct = 1; /* 2-byte sequence? */
            if (((uint8_t)c & 0xF0) == 0xE0)
                pb->ct = 2; /* 3-byte sequence? */
            if (((uint8_t)c & 0xF8) == 0xF0)
                pb->ct = 3;                 /* 4-byte sequence? */
            return;                         /* Wrong leading byte (discard it) */
        } else {                            /* In the multi-byte sequence */
            if (((uint8_t)c & 0xC0) != 0x80) { /* Broken sequence? */
                pb->ct = 0;
                continue; /* Discard the sequense */
            }
            pb->bs[++pb->wi] = (uint8_t)c; /* Save the trailing byte */
            if (--pb->ct == 0)
                break; /* End of the sequence? */
            return;
        }
    }
    tp = (const char *)pb->bs;
    dc = tchar2uni(&tp); /* UTF-8 ==> UTF-16 */
    if (dc == 0xFFFFFFFF)
        return; /* Wrong code? */
    hs = (uint16_t)(dc >> 16);
    wc = (uint16_t)dc;
    /* A code point in UTF-16 is available in hs and wc */

    /* Write a code point in UTF-8 */
    if (hs != 0) { /* 4-byte sequence? */
        nc += 3;
        hs = (hs & 0x3FF) + 0x40;
        pb->buf[i++] = (uint8_t)(0xF0 | hs >> 8);
        pb->buf[i++] = (uint8_t)(0x80 | (hs >> 2 & 0x3F));
        pb->buf[i++] = (uint8_t)(0x80 | (hs & 3) << 4 | (wc >> 6 & 0x0F));
        pb->buf[i++] = (uint8_t)(0x80 | (wc & 0x3F));
    } else {
        if (wc < 0x80) { /* Single byte? */
            pb->buf[i++] = (uint8_t)wc;
        } else {
            if (wc < 0x800) { /* 2-byte sequence? */
                nc += 1;
                pb->buf[i++] = (uint8_t)(0xC0 | wc >> 6);
            } else { /* 3-byte sequence */
                nc += 2;
                pb->buf[i++] = (uint8_t)(0xE0 | wc >> 12);
                pb->buf[i++] = (uint8_t)(0x80 | (wc >> 6 & 0x3F));
            }
            pb->buf[i++] = (uint8_t)(0x80 | (wc & 0x3F));
        }
    }

    if (i >= (int)(sizeof pb->buf) - 4) { /* Write buffered characters to the file */
        f_write(pb->fp, pb->buf, (unsigned)i, &n);
        i = (n == (unsigned)i) ? 0 : -1;
    }
    pb->idx = i;
    pb->nchr = nc + 1;
}

/* Flush remaining characters in the buffer */

static int putc_flush(putbuff *pb)
{
    unsigned nw;

    if (pb->idx >= 0 /* Flush buffered characters to the file */
        && f_write(pb->fp, pb->buf, (unsigned)pb->idx, &nw) == FR_OK && (unsigned)pb->idx == nw)
        return pb->nchr;
    return -1;
}

/* Initialize write buffer */

static void putc_init(putbuff *pb, file_t *fp)
{
    memset(pb, 0, sizeof(putbuff));
    pb->fp = fp;
}

int f_putc(char c, /* A character to be output */
           file_t *fp) /* Pointer to the file object */
{
    putbuff pb;

    putc_init(&pb, fp);
    putc_bfd(&pb, c); /* Put the character */
    return putc_flush(&pb);
}

/*-----------------------------------------------------------------------*/
/* Put a String to the File                                              */
/*-----------------------------------------------------------------------*/

int f_puts(const char *str, /* Pointer to the string to be output */
           file_t *fp)          /* Pointer to the file object */
{
    putbuff pb;

    putc_init(&pb, fp);
    while (*str)
        putc_bfd(&pb, *str++); /* Put the string */
    return putc_flush(&pb);
}

/*-----------------------------------------------------------------------*/
/* Put a Formatted String to the File (with sub-functions)               */
/*-----------------------------------------------------------------------*/
#if FF_PRINT_FLOAT
#include <math.h>

static int ilog10(double n) /* Calculate log10(n) in integer output */
{
    int rv = 0;

    while (n >= 10) { /* Decimate digit in right shift */
        if (n >= 100000) {
            n /= 100000;
            rv += 5;
        } else {
            n /= 10;
            rv++;
        }
    }
    while (n < 1) { /* Decimate digit in left shift */
        if (n < 0.00001) {
            n *= 100000;
            rv -= 5;
        } else {
            n *= 10;
            rv--;
        }
    }
    return rv;
}

static double i10x(int n) /* Calculate 10^n in integer input */
{
    double rv = 1;

    while (n > 0) { /* Left shift */
        if (n >= 5) {
            rv *= 100000;
            n -= 5;
        } else {
            rv *= 10;
            n--;
        }
    }
    while (n < 0) { /* Right shift */
        if (n <= -5) {
            rv /= 100000;
            n += 5;
        } else {
            rv /= 10;
            n++;
        }
    }
    return rv;
}

static void ftoa(char *buf,  /* Buffer to output the floating point string */
                 double val, /* Value to output */
                 int prec,   /* Number of fractional digits */
                 char fmt)  /* Notation */
{
    int d;
    int e = 0, m = 0;
    char sign = 0;
    double w;
    const char *er = 0;
    const char ds = FF_PRINT_FLOAT == 2 ? ',' : '.';

    if (isnan(val)) { /* Not a number? */
        er = "NaN";
    } else {
        if (prec < 0)
            prec = 6;  /* Default precision? (6 fractional digits) */
        if (val < 0) { /* Negative? */
            val = 0 - val;
            sign = '-';
        } else {
            sign = '+';
        }
        if (isinf(val)) { /* Infinite? */
            er = "INF";
        } else {
            if (fmt == 'f') {              /* Decimal notation? */
                val += i10x(0 - prec) / 2; /* Round (nearest) */
                m = ilog10(val);
                if (m < 0)
                    m = 0;
                if (m + prec + 3 >= SZ_NUM_BUF)
                    er = "OV";                           /* Buffer overflow? */
            } else {                                     /* E notation */
                if (val != 0) {                          /* Not a true zero? */
                    val += i10x(ilog10(val) - prec) / 2; /* Round (nearest) */
                    e = ilog10(val);
                    if (e > 99 || prec + 7 >= SZ_NUM_BUF) { /* Buffer overflow or E > +99? */
                        er = "OV";
                    } else {
                        if (e < -99)
                            e = -99;
                        val /= i10x(e); /* Normalize */
                    }
                }
            }
        }
        if (!er) { /* Not error condition */
            if (sign == '-')
                *buf++ = sign; /* Add a - if negative value */
            do {               /* Put decimal number */
                if (m == -1)
                    *buf++ = ds; /* Insert a decimal separator when get into
                                    fractional part */
                w = i10x(m);     /* Snip the highest digit d */
                d = (int)(val / w);
                val -= d * w;
                *buf++ = (char)('0' + d); /* Put the digit */
            } while (--m >= -prec);       /* Output all digits specified by prec */
            if (fmt != 'f') {             /* Put exponent if needed */
                *buf++ = (char)fmt;
                if (e < 0) {
                    e = 0 - e;
                    *buf++ = '-';
                } else {
                    *buf++ = '+';
                }
                *buf++ = (char)('0' + e / 10);
                *buf++ = (char)('0' + e % 10);
            }
        }
    }
    if (er) { /* Error condition */
        if (sign)
            *buf++ = sign; /* Add sign if needed */
        do {               /* Put error symbol */
            *buf++ = *er++;
        } while (*er);
    }
    *buf = 0; /* Term */
}
#endif /* FF_PRINT_FLOAT */

int f_printf(file_t *fp,          /* Pointer to the file object */
             const char *fmt, /* Pointer to the format string */
             ...)              /* Optional arguments... */
{
    va_list arp;
    putbuff pb;
    unsigned i, j, w, f, r;
    int prec;
#if FF_PRINT_LLI
    uint64_t v;
#else
    uint32_t v;
#endif
    char *tp;
    char tc, pad;
    char nul = 0;
    char d, str[SZ_NUM_BUF];

    putc_init(&pb, fp);

    va_start(arp, fmt);

    for (;;) {
        tc = *fmt++;
        if (tc == 0)
            break;       /* End of format string */
        if (tc != '%') { /* Not an escape character (pass-through) */
            putc_bfd(&pb, tc);
            continue;
        }
        f = w = 0;
        pad = ' ';
        prec = -1; /* Initialize parms */
        tc = *fmt++;
        if (tc == '0') { /* Flag: '0' padded */
            pad = '0';
            tc = *fmt++;
        } else if (tc == '-') { /* Flag: Left aligned */
            f = 2;
            tc = *fmt++;
        }
        if (tc == '*') { /* Minimum width from an argument */
            w = va_arg(arp, int);
            tc = *fmt++;
        } else {
            while (IsDigit(tc)) { /* Minimum width */
                w = w * 10 + tc - '0';
                tc = *fmt++;
            }
        }
        if (tc == '.') { /* Precision */
            tc = *fmt++;
            if (tc == '*') { /* Precision from an argument */
                prec = va_arg(arp, int);
                tc = *fmt++;
            } else {
                prec = 0;
                while (IsDigit(tc)) { /* Precision */
                    prec = prec * 10 + tc - '0';
                    tc = *fmt++;
                }
            }
        }
        if (tc == 'l') { /* Size: long int */
            f |= 4;
            tc = *fmt++;
#if FF_PRINT_LLI
            if (tc == 'l') { /* Size: long long int */
                f |= 8;
                tc = *fmt++;
            }
#endif
        }
        if (tc == 0)
            break;    /* End of format string */
        switch (tc) { /* Atgument type is... */
        case 'b':     /* Unsigned binary */
            r = 2;
            break;

        case 'o': /* Unsigned octal */
            r = 8;
            break;

        case 'd': /* Signed decimal */
        case 'u': /* Unsigned decimal */
            r = 10;
            break;

        case 'x': /* Unsigned hexadecimal (lower case) */
        case 'X': /* Unsigned hexadecimal (upper case) */
            r = 16;
            break;

        case 'c': /* Character */
            putc_bfd(&pb, (char)va_arg(arp, int));
            continue;

        case 's':                      /* String */
            tp = va_arg(arp, char *); /* Get a pointer argument */
            if (!tp)
                tp = &nul; /* Null ptr generates a null string */
            for (j = 0; tp[j]; j++)
                ; /* j = tcslen(tp) */
            if (prec >= 0 && j > (unsigned)prec)
                j = prec; /* Limited length of string body */
            for (; !(f & 2) && j < w; j++)
                putc_bfd(&pb, pad); /* Left pads */
            while (*tp && prec--)
                putc_bfd(&pb, *tp++); /* Body */
            while (j++ < w)
                putc_bfd(&pb, ' '); /* Right pads */
            continue;
#if FF_PRINT_FLOAT
        case 'f':                                     /* Floating point (decimal) */
        case 'e':                                     /* Floating point (e) */
        case 'E':                                     /* Floating point (E) */
            ftoa(str, va_arg(arp, double), prec, tc); /* Make a floating point string */
            for (j = strlen(str); !(f & 2) && j < w; j++)
                putc_bfd(&pb, pad); /* Left pads */
            for (i = 0; str[i]; putc_bfd(&pb, str[i++]))
                ; /* Body */
            while (j++ < w)
                putc_bfd(&pb, ' '); /* Right pads */
            continue;
#endif
        default: /* Unknown type (pass-through) */
            putc_bfd(&pb, tc);
            continue;
        }

        /* Get an integer argument and put it in numeral */
#if FF_PRINT_LLI
        if (f & 8) { /* long long argument? */
            v = (uint64_t)va_arg(arp, long long);
        } else if (f & 4) { /* long argument? */
            v = (tc == 'd') ? (uint64_t)(long long)va_arg(arp, long)
                            : (uint64_t)va_arg(arp, unsigned long);
        } else { /* int/short/char argument */
            v = (tc == 'd') ? (uint64_t)(long long)va_arg(arp, int) : (uint64_t)va_arg(arp, unsigned int);
        }
        if (tc == 'd' && (v & 0x8000000000000000)) { /* Negative value? */
            v = 0 - v;
            f |= 1;
        }
#else
        if (f & 4) { /* long argument? */
            v = (uint32_t)va_arg(arp, long);
        } else { /* int/short/char argument */
            v = (tc == 'd') ? (uint32_t)(long)va_arg(arp, int) : (uint32_t)va_arg(arp, unsigned int);
        }
        if (tc == 'd' && (v & 0x80000000)) { /* Negative value? */
            v = 0 - v;
            f |= 1;
        }
#endif
        i = 0;
        do { /* Make an integer number string */
            d = (char)(v % r);
            v /= r;
            if (d > 9)
                d += (tc == 'x') ? 0x27 : 0x07;
            str[i++] = d + '0';
        } while (v && i < SZ_NUM_BUF);
        if (f & 1)
            str[i++] = '-'; /* Sign */
        /* Write it */
        for (j = i; !(f & 2) && j < w; j++) { /* Left pads */
            putc_bfd(&pb, pad);
        }
        do { /* Body */
            putc_bfd(&pb, (char)str[--i]);
        } while (i);
        while (j++ < w) { /* Right pads */
            putc_bfd(&pb, ' ');
        }
    }

    va_end(arp);

    return putc_flush(&pb);
}

#endif /* !FF_FS_READONLY */
#endif /* FF_USE_STRFUNC */

//
// Get size of the file_t structure.
//
unsigned f_sizeof_file_t()
{
    return sizeof(file_t);
}

//
// Get size of the directory_t structure.
//
unsigned f_sizeof_directory_t()
{
    return sizeof(directory_t);
}

//
// Get size of the filesystem_t structure.
//
unsigned f_sizeof_filesystem_t()
{
    return sizeof(filesystem_t);
}

//
// Get the file size in bytes.
//
fs_size_t f_size(file_t *fp)
{
    return fp->obj.objsize;
}

//
// Get the current file position.
//
fs_size_t f_tell(file_t *fp)
{
    return fp->fptr;
}

//
// Return nonzero if the end-of-file indicator is set.
//
int f_eof(file_t *fp)
{
    return (fp->fptr == fp->obj.objsize);
}
//
// Return nonzero if the error indicator is set.
//
int f_error(file_t *fp)
{
    return fp->err;
}
