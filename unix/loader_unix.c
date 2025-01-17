#include <fpm/api.h>
#include <fpm/loader.h>
#include <fpm/context.h>
#include <fpm/fs.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

//
// Load dynamic binary.
// Return true on success.
//
bool fpm_load_arch(fpm_context_t *ctx, const char *filename)
{
    // Unix: open the shared library file in read-only mode.
    ctx->fd = open(filename, O_RDONLY);
    if (ctx->fd < 0) {
        fpm_printf("%s: Cannot open\r\n", filename);
        return false;
    }

    // Get the size of the file
    struct stat sb;
    if (fstat(ctx->fd, &sb) < 0) {
        fpm_printf("%s: Cannot fstat\r\n", filename);
err:    close(ctx->fd);
        return false;
    }
    ctx->file_size = sb.st_size;

    // Map the file into memory
    ctx->base = mmap(NULL, ctx->file_size, PROT_READ | PROT_EXEC, MAP_SHARED, ctx->fd, 0);
    if (ctx->base == MAP_FAILED) {
        fpm_printf("%s: Cannot mmap\r\n", filename);
        goto err;
    }
    return true;
}

//
// Unmap ELF binary from memory.
//
void fpm_unload_arch(fpm_context_t *ctx)
{
    // Unmap the file from memory.
    if (ctx->base != NULL) {
         munmap(ctx->base, ctx->file_size);
         ctx->base = NULL;
    }

    // Close file.
    // Note: descriptor cannot be zero.
    if (ctx->fd > 0) {
        close(ctx->fd);
        ctx->fd = 0;
    }
}
