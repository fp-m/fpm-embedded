/*------------------------------------------------------------/
/ Open or create a file in append mode
/ (This function was sperseded by FA_OPEN_APPEND flag at FatFs R0.12a)
/------------------------------------------------------------*/

fs_result_t open_append(file_t *fp,          /* [OUT] File object to create */
                    const char *path) /* [IN]  File name to be opened */
{
    fs_result_t fr;

    /* Opens an existing file. If not exist, creates a new file. */
    fr = f_open(fp, path, FA_WRITE | FA_OPEN_ALWAYS);
    if (fr == FR_OK) {
        /* Seek to end of the file to append data */
        fr = f_lseek(fp, f_size(fp));
        if (fr != FR_OK)
            f_close(fp);
    }
    return fr;
}

int main(void)
{
    fs_result_t fr;
    filesystem_t *fs = alloca(f_sizeof_filesystem_t());
    file_t *fil = alloca(f_sizeof_file_t());

    /* Open or create a log file and ready to append */
    f_mount(fs, "", 0);
    fr = open_append(fil, "logfile.txt");
    if (fr != FR_OK)
        return 1;

    /* Append a line */
    f_printf(fil, "%02u/%02u/%u, %2u:%02u\n", Mday, Mon, Year, Hour, Min);

    /* Close the file */
    f_close(fil);

    return 0;
}
