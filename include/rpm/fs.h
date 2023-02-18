//
// Filesystem Application Interface.
//

// Type for file size in bytes.
//typedef uint64_t fs_size_t;

//
// File access functions.
//
FRESULT f_open(FIL *fp, const char *path, uint8_t mode);          /* Open or create a file */
FRESULT f_close(FIL *fp);                                       /* Close an open file object */
FRESULT f_read(FIL *fp, void *buff, unsigned btr, unsigned *br);        /* Read data from the file */
FRESULT f_write(FIL *fp, const void *buff, unsigned btw, unsigned *bw); /* Write data to the file */
FRESULT f_lseek(FIL *fp, fs_size_t ofs);         /* Move file pointer of the file object */
FRESULT f_truncate(FIL *fp);                   /* Truncate the file */
FRESULT f_sync(FIL *fp);                       /* Flush cached data of the writing file */
FRESULT f_forward(FIL *fp, unsigned (*func)(const uint8_t *, unsigned), unsigned btf,
                  unsigned *bf);                      /* Forward data to the stream */
FRESULT f_expand(FIL *fp, fs_size_t fsz, uint8_t opt); /* Allocate a contiguous block to the file */
char *f_gets(char *buff, int len, FIL *fp); /* Get a string from the file */
int f_putc(char c, FIL *fp);                 /* Put a character to the file */
int f_puts(const char *str, FIL *cp);        /* Put a string to the file */
int f_printf(FIL *fp, const char *str, ...); /* Put a formatted string to the file */
#define f_tell(fp) ((fp)->fptr)
#define f_eof(fp) ((int)((fp)->fptr == (fp)->obj.objsize))
#define f_size(fp) ((fp)->obj.objsize)
#define f_error(fp) ((fp)->err)
#define f_rewind(fp) f_lseek((fp), 0)

//
// Directory access functions.
//
FRESULT f_opendir(DIR *dp, const char *path); /* Open a directory */
FRESULT f_closedir(DIR *dp);                   /* Close an open directory */
FRESULT f_readdir(DIR *dp, FILINFO *fno);      /* Read a directory item */
FRESULT f_findfirst(DIR *dp, FILINFO *fno, const char *path,
                    const char *pattern); /* Find first file */
FRESULT f_findnext(DIR *dp, FILINFO *fno); /* Find next file */
#define f_rewinddir(dp) f_readdir((dp), 0)

//
// File and directory management functions.
//
FRESULT f_stat(const char *path, FILINFO *fno);          /* Get file status */
FRESULT f_unlink(const char *path);       /* Delete an existing file or directory */
FRESULT f_rename(const char *path_old,
                 const char *path_new);                  /* Rename/Move a file or directory */
FRESULT f_chmod(const char *path, uint8_t attr, uint8_t mask); /* Change attribute of a file/dir */
FRESULT f_utime(const char *path, const FILINFO *fno);   /* Change timestamp of a file/dir */
FRESULT f_mkdir(const char *path);        /* Create a sub directory */
FRESULT f_chdir(const char *path);                       /* Change current directory */
FRESULT f_chdrive(const char *path);                     /* Change current drive */
FRESULT f_getcwd(char *buff, unsigned len);                  /* Get current directory */
#define f_rmdir(path) f_unlink(path)

//
// Volume management and system configuration functions.
//
FRESULT f_mount(FATFS *fs, const char *path, uint8_t opt); /* Mount/Unmount a logical drive */
FRESULT f_mkfs(const char *path, const MKFS_PARM *opt, void *work,
               unsigned len); /* Create a FAT volume */
FRESULT f_getfree(const char *path, uint32_t *nclst,
                  FATFS **fatfs); /* Get number of free clusters on the drive */
FRESULT f_getlabel(const char *path, char *label, uint32_t *vsn); /* Get volume label */
FRESULT f_setlabel(const char *label);                          /* Set volume label */
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
