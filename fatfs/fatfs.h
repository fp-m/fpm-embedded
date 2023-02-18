/*----------------------------------------------------------------------------/
/  FatFs - Generic FAT Filesystem module  R0.15                               /
/-----------------------------------------------------------------------------/
/
/ Copyright (C) 2022, ChaN, all right reserved.
/
/ FatFs module is an open source software. Redistribution and use of FatFs in
/ source and binary forms, with or without modification, are permitted provided
/ that the following condition is met:

/ 1. Redistributions of source code must retain the above copyright notice,
/    this condition and the following disclaimer.
/
/ This software is provided by the copyright holder and contributors "AS IS"
/ and any warranties related to this software are DISCLAIMED.
/ The copyright owner or contributors be NOT LIABLE for any damages caused
/ by use of this software.
/
/----------------------------------------------------------------------------*/
#ifndef RPM_FATFS_H
#define RPM_FATFS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ffconf.h" /* FatFs configuration options */
#include <stdint.h>

/* Type of file size and LBA variables */

// TODO: move to rpm/api.h
typedef uint64_t fs_size_t; // Type for file size in bytes.
typedef uint32_t fs_lba_t;  // only LBA-32 is supported

/* Filesystem object structure (FATFS) */

typedef struct {
    uint8_t fs_type;   /* Filesystem type (0:not mounted) */
    uint8_t pdrv;      /* Volume hosting physical drive */
    uint8_t ldrv;      /* Logical drive number (used only when FF_FS_REENTRANT) */
    uint8_t n_fats;    /* Number of FATs (1 or 2) */
    uint8_t wflag;     /* win[] status (b0:dirty) */
    uint8_t fsi_flag;  /* FSINFO status (b7:disabled, b0:dirty) */
    uint16_t id;        /* Volume mount ID */
    uint16_t n_rootdir; /* Number of root directory entries (FAT12/16) */
    uint16_t csize;     /* Cluster size [sectors] */
#if FF_MAX_SS != FF_MIN_SS
    uint16_t ssize; /* Sector size (512, 1024, 2048 or 4096) */
#endif
    uint16_t *lfnbuf; /* LFN working buffer */
    uint8_t *dirbuf; /* Directory entry block scratchpad buffer for exFAT */
#if !FF_FS_READONLY
    uint32_t last_clst; /* Last allocated cluster */
    uint32_t free_clst; /* Number of free clusters */
#endif
#if FF_FS_RPATH
    uint32_t cdir; /* Current directory start cluster (0:root) */
    uint32_t cdc_scl;  /* Containing directory start cluster (invalid when cdir is 0) */
    uint32_t cdc_size; /* b31-b8:Size of containing directory, b7-b0: Chain status */
    uint32_t cdc_ofs;  /* Offset in the containing directory (invalid when cdir is 0) */
#endif
    uint32_t n_fatent; /* Number of FAT entries (number of clusters + 2) */
    uint32_t fsize;    /* Number of sectors per FAT */
    fs_lba_t volbase;  /* Volume base sector */
    fs_lba_t fatbase;  /* FAT base sector */
    fs_lba_t dirbase;  /* Root directory base sector (FAT12/16) or cluster (FAT32/exFAT) */
    fs_lba_t database; /* Data base sector */
    fs_lba_t bitbase; /* Allocation bitmap base sector */
    fs_lba_t winsect;       /* Current sector appearing in the win[] */
    uint8_t win[FF_MAX_SS]; /* Disk access window for Directory, FAT (and file data at tiny cfg) */
} FATFS;

/* Object ID and allocation information (FFOBJID) */

typedef struct {
    FATFS *fs; /* Pointer to the hosting volume of this object */
    uint16_t id;   /* Hosting volume's mount ID */
    uint8_t attr; /* Object attribute */
    uint8_t stat; /* Object chain status (b1-0: =0:not contiguous, =2:contiguous, =3:fragmented in this
                  session, b2:sub-directory stretched) */
    uint32_t sclust;    /* Object data start cluster (0:no cluster or root directory) */
    fs_size_t objsize; /* Object size (valid when sclust != 0) */
    uint32_t n_cont; /* Size of first fragment - 1 (valid when stat == 3) */
    uint32_t n_frag; /* Size of last fragment needs to be written to FAT (valid when not zero) */
    uint32_t c_scl;  /* Containing directory start cluster (valid when sclust != 0) */
    uint32_t c_size; /* b31-b8:Size of containing directory, b7-b0: Chain status (valid when c_scl !=
                     0) */
    uint32_t c_ofs;  /* Offset in the containing directory (valid when file object and sclust != 0) */
#if FF_FS_LOCK
    unsigned lockid; /* File lock ID origin from 1 (index of file semaphore table Files[]) */
#endif
} FFOBJID;

/* File object structure (file_t) */

struct _file_t {
    FFOBJID obj;  /* Object identifier (must be the 1st member to detect invalid object pointer) */
    uint8_t flag;    /* File status flags */
    uint8_t err;     /* Abort flag (error code) */
    fs_size_t fptr; /* File read/write pointer (Zeroed on file open) */
    uint32_t clust;  /* Current cluster of fpter (invalid when fptr is 0) */
    fs_lba_t sect;   /* Sector number appearing in buf[] (0:invalid) */
#if !FF_FS_READONLY
    fs_lba_t dir_sect; /* Sector number containing the directory entry (not used at exFAT) */
    uint8_t *dir_ptr;  /* Pointer to the directory entry in the win[] (not used at exFAT) */
#endif
#if FF_USE_FASTSEEK
    uint32_t *cltbl; /* Pointer to the cluster link map table (nulled on open, set by application) */
#endif
#if !FF_FS_TINY
    uint8_t buf[FF_MAX_SS]; /* File private data read/write window */
#endif
};

/* Directory object structure (DIR) */

typedef struct {
    FFOBJID obj; /* Object identifier */
    uint32_t dptr;  /* Current read/write offset */
    uint32_t clust; /* Current cluster */
    fs_lba_t sect;  /* Current sector (0:Read operation has terminated) */
    uint8_t *dir;   /* Pointer to the directory item in the win[] */
    uint8_t fn[12]; /* SFN (in/out) {body[8],ext[3],status[1]} */
    uint32_t blk_ofs; /* Offset of current entry block being processed (0xFFFFFFFF:Invalid) */
#if FF_USE_FIND
    const char *pat; /* Pointer to the name matching pattern */
#endif
} DIR;

/* File information structure (FILINFO) */

typedef struct {
    fs_size_t fsize; /* File size */
    uint16_t fdate;    /* Modified date */
    uint16_t ftime;    /* Modified time */
    uint8_t fattrib;  /* File attribute */
    char altname[FF_SFN_BUF + 1]; /* Alternative file name */
    char fname[FF_LFN_BUF + 1];   /* Primary file name */
} FILINFO;

/* Format parameter structure (MKFS_PARM) */

typedef struct {
    uint8_t fmt;      /* Format option (FM_FAT, FM_FAT32, FM_EXFAT and FM_SFD) */
    uint8_t n_fat;    /* Number of FATs */
    unsigned align;    /* Data area alignment (sector) */
    unsigned n_root;   /* Number of root directory entries */
    uint32_t au_size; /* Cluster size (byte) */
} MKFS_PARM;

/* File function return code (fs_result_t) */

// TODO: move to rpm/api.h
typedef enum {
    FR_OK = 0,                   // Succeeded
    FR_DISK_ERR = 1,             // A hard error occurred in the low level disk I/O layer
    FR_INT_ERR = 2,              // Assertion failed
    FR_NOT_READY = 3,            // The physical drive cannot work
    FR_NO_FILE = 4,              // Could not find the file
    FR_NO_PATH = 5,              // Could not find the path
    FR_INVALID_NAME = 6,         // The path name format is invalid
    FR_DENIED = 7,               // Access denied due to prohibited access or directory full
    FR_EXIST = 8,                // Access denied due to prohibited access
    FR_INVALID_OBJECT = 9,       // The file/directory object is invalid
    FR_WRITE_PROTECTED = 10,     // The physical drive is write protected
    FR_INVALID_DRIVE = 11,       // The logical drive number is invalid
    FR_NOT_ENABLED = 12,         // The volume has no work area
    FR_NO_FILESYSTEM = 13,       // There is no valid FAT volume
    FR_MKFS_ABORTED = 14,        // The f_mkfs() aborted due to any problem
    FR_TIMEOUT = 15,             // Could not get a grant to access the volume within defined period
    FR_LOCKED = 16,              // The operation is rejected according to the file sharing policy
    FR_NOT_ENOUGH_CORE = 17,     // LFN working buffer could not be allocated
    FR_TOO_MANY_OPEN_FILES = 18, // Too many open files
    FR_INVALID_PARAMETER = 19,   // Given parameter is invalid
} fs_result_t;

/*--------------------------------------------------------------*/
/* Additional Functions                                         */
/*--------------------------------------------------------------*/

/* RTC function (provided by user) */
#if !FF_FS_READONLY && !FF_FS_NORTC
uint32_t get_fattime(void); /* Get current time */
#endif

/* LFN support functions (defined in ffunicode.c) */

uint16_t ff_oem2uni(uint16_t oem, uint16_t cp); /* OEM code to Unicode conversion */
uint16_t ff_uni2oem(uint32_t uni, uint16_t cp); /* Unicode to OEM code conversion */
uint32_t ff_wtoupper(uint32_t uni);         /* Unicode upper-case conversion */

/* O/S dependent functions (samples available in ffsystem.c) */

#if FF_USE_LFN == 3            /* Dynamic memory allocation */
void *ff_memalloc(unsigned msize); /* Allocate memory block */
void ff_memfree(void *mblock); /* Free memory block */
#endif
#if FF_FS_REENTRANT            /* Sync functions */
int ff_mutex_create(int vol);  /* Create a sync object */
void ff_mutex_delete(int vol); /* Delete a sync object */
int ff_mutex_take(int vol);    /* Lock sync object */
void ff_mutex_give(int vol);   /* Unlock sync object */
#endif

/* Definitions of volume management */

#if FF_STR_VOLUME_ID
#ifndef FF_VOLUME_STRS
extern const char *VolumeStr[FF_VOLUMES]; /* User defied volume ID */
#endif
#endif

/*--------------------------------------------------------------*/
/* Flags and Offset Address                                     */
/*--------------------------------------------------------------*/

/* Fast seek controls (2nd argument of f_lseek) */
#define CREATE_LINKMAP ((fs_size_t)0 - 1)

/* Format options (2nd argument of f_mkfs) */
#define FM_FAT 0x01
#define FM_FAT32 0x02
#define FM_EXFAT 0x04
#define FM_ANY 0x07
#define FM_SFD 0x08

/* Filesystem type (FATFS.fs_type) */
#define FS_FAT12 1
#define FS_FAT16 2
#define FS_FAT32 3
#define FS_EXFAT 4

/* File attribute bits for directory entry (FILINFO.fattrib) */
#define AM_RDO 0x01 /* Read only */
#define AM_HID 0x02 /* Hidden */
#define AM_SYS 0x04 /* System */
#define AM_DIR 0x10 /* Directory */
#define AM_ARC 0x20 /* Archive */

#ifdef __cplusplus
}
#endif

#endif // RPM_FATFS_H
