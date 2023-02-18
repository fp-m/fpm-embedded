//
// Filesystem Application Interface.
//

//
// File access functions.
//
FRESULT f_open(FIL *fp, const TCHAR *path, BYTE mode);          /* Open or create a file */
FRESULT f_close(FIL *fp);                                       /* Close an open file object */
FRESULT f_read(FIL *fp, void *buff, UINT btr, UINT *br);        /* Read data from the file */
FRESULT f_write(FIL *fp, const void *buff, UINT btw, UINT *bw); /* Write data to the file */
FRESULT f_lseek(FIL *fp, FSIZE_t ofs);         /* Move file pointer of the file object */
FRESULT f_truncate(FIL *fp);                   /* Truncate the file */
FRESULT f_sync(FIL *fp);                       /* Flush cached data of the writing file */
FRESULT f_forward(FIL *fp, UINT (*func)(const BYTE *, UINT), UINT btf,
                  UINT *bf);                      /* Forward data to the stream */
FRESULT f_expand(FIL *fp, FSIZE_t fsz, BYTE opt); /* Allocate a contiguous block to the file */
TCHAR *f_gets(TCHAR *buff, int len, FIL *fp); /* Get a string from the file */
int f_putc(TCHAR c, FIL *fp);                 /* Put a character to the file */
int f_puts(const TCHAR *str, FIL *cp);        /* Put a string to the file */
int f_printf(FIL *fp, const TCHAR *str, ...); /* Put a formatted string to the file */
#define f_tell(fp) ((fp)->fptr)
#define f_eof(fp) ((int)((fp)->fptr == (fp)->obj.objsize))
#define f_size(fp) ((fp)->obj.objsize)
#define f_error(fp) ((fp)->err)
#define f_rewind(fp) f_lseek((fp), 0)

//
// Directory access functions.
//
FRESULT f_opendir(DIR *dp, const TCHAR *path); /* Open a directory */
FRESULT f_closedir(DIR *dp);                   /* Close an open directory */
FRESULT f_readdir(DIR *dp, FILINFO *fno);      /* Read a directory item */
FRESULT f_findfirst(DIR *dp, FILINFO *fno, const TCHAR *path,
                    const TCHAR *pattern); /* Find first file */
FRESULT f_findnext(DIR *dp, FILINFO *fno); /* Find next file */
#define f_rewinddir(dp) f_readdir((dp), 0)

//
// File and directory management functions.
//
FRESULT f_stat(const TCHAR *path, FILINFO *fno);          /* Get file status */
FRESULT f_unlink(const TCHAR *path);       /* Delete an existing file or directory */
FRESULT f_rename(const TCHAR *path_old,
                 const TCHAR *path_new);                  /* Rename/Move a file or directory */
FRESULT f_chmod(const TCHAR *path, BYTE attr, BYTE mask); /* Change attribute of a file/dir */
FRESULT f_utime(const TCHAR *path, const FILINFO *fno);   /* Change timestamp of a file/dir */
FRESULT f_mkdir(const TCHAR *path);        /* Create a sub directory */
FRESULT f_chdir(const TCHAR *path);                       /* Change current directory */
FRESULT f_chdrive(const TCHAR *path);                     /* Change current drive */
FRESULT f_getcwd(TCHAR *buff, UINT len);                  /* Get current directory */
#define f_rmdir(path) f_unlink(path)

//
// Volume management and system configuration functions.
//
FRESULT f_mount(FATFS *fs, const TCHAR *path, BYTE opt); /* Mount/Unmount a logical drive */
FRESULT f_mkfs(const TCHAR *path, const MKFS_PARM *opt, void *work,
               UINT len); /* Create a FAT volume */
FRESULT f_getfree(const TCHAR *path, DWORD *nclst,
                  FATFS **fatfs); /* Get number of free clusters on the drive */
FRESULT f_getlabel(const TCHAR *path, TCHAR *label, DWORD *vsn); /* Get volume label */
FRESULT f_setlabel(const TCHAR *label);                          /* Set volume label */
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
