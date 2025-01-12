#include <fpm/api.h>
#include <fpm/loader.h>
#include <fpm/fs.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

//
// Load dynamic binary.
// Return true on success.
//
bool fpm_load_arch(fpm_executable_t *dynobj, const char *filename)
{
    // Unix: open the shared library file in read-only mode.
    dynobj->fd = open(filename, O_RDONLY);
    if (dynobj->fd < 0) {
        fpm_printf("%s: Cannot open\r\n", filename);
        return false;
    }

    // Get the size of the file
    struct stat sb;
    if (fstat(dynobj->fd, &sb) < 0) {
        fpm_printf("%s: Cannot fstat\r\n", filename);
err:    close(dynobj->fd);
        return false;
    }
    dynobj->file_size = sb.st_size;

    // Map the file into memory
    dynobj->base = mmap(NULL, dynobj->file_size, PROT_READ | PROT_EXEC, MAP_SHARED, dynobj->fd, 0);
    if (dynobj->base == MAP_FAILED) {
        fpm_printf("%s: Cannot mmap\r\n", filename);
        goto err;
    }
    return true;
}

//
// Unmap ELF binary from memory.
//
void fpm_unload_arch(fpm_executable_t *dynobj)
{
    // Unmap the file from memory.
    if (dynobj->base != NULL) {
         munmap(dynobj->base, dynobj->file_size);
         dynobj->base = NULL;
    }

    // Close file.
    // Note: descriptor cannot be zero.
    if (dynobj->fd > 0) {
        close(dynobj->fd);
        dynobj->fd = 0;
    }
}
