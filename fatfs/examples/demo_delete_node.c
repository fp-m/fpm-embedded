/*------------------------------------------------------------/
/ Delete a sub-directory even if it contains any file
/-------------------------------------------------------------/
/ The delete_node() function is for R0.12+.
/ It works regardless of FF_FS_RPATH.
*/

fs_result_t delete_node(char *path,  /* Path name buffer with the sub-directory to delete */
                    unsigned sz_buff, /* Size of path name buffer (items) */
                    file_info_t *fno) /* Name read buffer */
{
    unsigned i, j;
    fs_result_t fr;
    directory_t *dir = alloca(f_sizeof_directory_t());

    fr = f_opendir(dir, path); /* Open the sub-directory to make it empty */
    if (fr != FR_OK)
        return fr;

    for (i = 0; path[i]; i++)
        ; /* Get current path length */
    path[i++] = _T('/');

    for (;;) {
        fr = f_readdir(dir, fno); /* Get a directory item */
        if (fr != FR_OK || !fno->fname[0])
            break; /* End of directory? */
        j = 0;
        do {                        /* Make a path name */
            if (i + j >= sz_buff) { /* Buffer over flow? */
                fr = 100;
                break; /* Fails with 100 when buffer overflow */
            }
            path[i + j] = fno->fname[j];
        } while (fno->fname[j++]);
        if (fno->fattrib & AM_DIR) { /* Item is a sub-directory */
            fr = delete_node(path, sz_buff, fno);
        } else { /* Item is a file */
            fr = f_unlink(path);
        }
        if (fr != FR_OK)
            break;
    }

    path[--i] = 0; /* Restore the path name */
    f_closedir(dir);

    if (fr == FR_OK)
        fr = f_unlink(path); /* Delete the empty sub-directory */
    return fr;
}

int main(void) /* How to use */
{
    fs_result_t fr;
    filesystem_t *fs = alloca(f_sizeof_filesystem_t());
    char buff[256];
    file_info_t fno;

    f_mount(fs, _T("5:"), 0);

    /* Directory to be deleted */
    _tcscpy(buff, _T("5:dir"));

    /* Delete the directory */
    fr = delete_node(buff, sizeof buff / sizeof buff[0], &fno);

    /* Check the result */
    if (fr) {
        _tprintf(_T("Failed to delete the directory. (%u)\n"), fr);
        return fr;
    } else {
        _tprintf(_T("The directory and the contents have successfully been deleted.\n"), buff);
        return 0;
    }
}
