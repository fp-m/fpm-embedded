/*----------------------------------------------------------------------/
/ Test if the file is contiguous                                        /
/----------------------------------------------------------------------*/

fs_result_t test_contiguous_file(file_t *fp,   /* [IN]  Open file object to be checked */
                             int *cont) /* [OUT] 1:Contiguous, 0:Fragmented or zero-length */
{
    uint32_t clst, clsz, step;
    fs_size_t fsz;
    fs_result_t fr;

    *cont = 0;
    fr = f_rewind(fp); /* Validates and prepares the file */
    if (fr != FR_OK)
        return fr;

#if FF_MAX_SS == FF_MIN_SS
    clsz = (uint32_t)fp->obj.fs->csize * FF_MAX_SS; /* Cluster size */
#else
    clsz = (uint32_t)fp->obj.fs->csize * fp->obj.fs->ssize;
#endif
    fsz = f_size(fp);
    if (fsz > 0) {
        clst = fp->obj.sclust - 1; /* A cluster leading the first cluster for first test */
        while (fsz) {
            step = (fsz >= clsz) ? clsz : (uint32_t)fsz;
            fr = f_lseek(fp, f_tell(fp) + step); /* Advances file pointer a cluster */
            if (fr != FR_OK)
                return fr;
            if (clst + 1 != fp->clust)
                break; /* Is not the cluster next to previous one? */
            clst = fp->clust;
            fsz -= step; /* Get current cluster for next test */
        }
        if (fsz == 0)
            *cont = 1; /* All done without fail? */
    }

    return FR_OK;
}
