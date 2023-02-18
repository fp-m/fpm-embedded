//
// Filesystem Application Interface.
//

// Type for file size in bytes.
//typedef uint64_t fs_size_t;

// Type for logical block address.
// Disk sizes up to 2 Tbytes are supported (LBA-32).
//typedef uint32_t fs_lba_t;

//
// File function return codes.
//
#if 0
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
#endif

//
// File access functions.
//
typedef struct _file_t file_t; // Opaque pointer
unsigned f_sizeof_file_t();

fs_result_t f_open(file_t *fp, const char *path, uint8_t mode);          /* Open or create a file */
fs_result_t f_close(file_t *fp);                                       /* Close an open file object */
fs_result_t f_read(file_t *fp, void *buff, unsigned btr, unsigned *br);        /* Read data from the file */
fs_result_t f_write(file_t *fp, const void *buff, unsigned btw, unsigned *bw); /* Write data to the file */
fs_result_t f_lseek(file_t *fp, fs_size_t ofs);         /* Move file pointer of the file object */
fs_result_t f_truncate(file_t *fp);                   /* Truncate the file */
fs_result_t f_sync(file_t *fp);                       /* Flush cached data of the writing file */
fs_result_t f_forward(file_t *fp, unsigned (*func)(const uint8_t *, unsigned), unsigned btf,
                  unsigned *bf);                      /* Forward data to the stream */
fs_result_t f_expand(file_t *fp, fs_size_t fsz, uint8_t opt); /* Allocate a contiguous block to the file */

char *f_gets(char *buff, int len, file_t *fp); /* Get a string from the file */
int f_putc(char c, file_t *fp);                 /* Put a character to the file */
int f_puts(const char *str, file_t *cp);        /* Put a string to the file */
int f_printf(file_t *fp, const char *str, ...); /* Put a formatted string to the file */

#define f_tell(fp) ((fp)->fptr)
#define f_eof(fp) ((int)((fp)->fptr == (fp)->obj.objsize))
#define f_size(fp) ((fp)->obj.objsize)
#define f_error(fp) ((fp)->err)
#define f_rewind(fp) f_lseek((fp), 0)

//
// Directory access functions.
//
fs_result_t f_opendir(DIR *dp, const char *path); /* Open a directory */
fs_result_t f_closedir(DIR *dp);                   /* Close an open directory */
fs_result_t f_readdir(DIR *dp, FILINFO *fno);      /* Read a directory item */
fs_result_t f_findfirst(DIR *dp, FILINFO *fno, const char *path,
                    const char *pattern); /* Find first file */
fs_result_t f_findnext(DIR *dp, FILINFO *fno); /* Find next file */
#define f_rewinddir(dp) f_readdir((dp), 0)

//
// File and directory management functions.
//
fs_result_t f_stat(const char *path, FILINFO *fno);          /* Get file status */
fs_result_t f_unlink(const char *path);       /* Delete an existing file or directory */
fs_result_t f_rename(const char *path_old,
                 const char *path_new);                  /* Rename/Move a file or directory */
fs_result_t f_chmod(const char *path, uint8_t attr, uint8_t mask); /* Change attribute of a file/dir */
fs_result_t f_utime(const char *path, const FILINFO *fno);   /* Change timestamp of a file/dir */
fs_result_t f_mkdir(const char *path);        /* Create a sub directory */
fs_result_t f_chdir(const char *path);                       /* Change current directory */
fs_result_t f_chdrive(const char *path);                     /* Change current drive */
fs_result_t f_getcwd(char *buff, unsigned len);                  /* Get current directory */
#define f_rmdir(path) f_unlink(path)

//
// Volume management functions.
//
fs_result_t f_mount(FATFS *fs, const char *path, uint8_t opt); /* Mount/Unmount a logical drive */
fs_result_t f_mkfs(const char *path, const MKFS_PARM *opt, void *work,
               unsigned len); /* Create a FAT volume */
fs_result_t f_getfree(const char *path, uint32_t *nclst,
                  FATFS **fatfs); /* Get number of free clusters on the drive */
fs_result_t f_getlabel(const char *path, char *label, uint32_t *vsn); /* Get volume label */
fs_result_t f_setlabel(const char *label);                          /* Set volume label */
#define f_unmount(path) f_mount(0, path, 0)

//
// File access modes.
// Use as 3rd argument of f_open().
//
enum {
    FA_READ = 0x01,
    FA_WRITE = 0x02,
    FA_OPEN_EXISTING = 0x00,
    FA_CREATE_NEW = 0x04,
    FA_CREATE_ALWAYS = 0x08,
    FA_OPEN_ALWAYS = 0x10,
    FA_OPEN_APPEND = 0x30,
};
