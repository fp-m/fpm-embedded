//
// Dynamic Loader Interface.
//
#ifndef FPM_LOADER_H
#define FPM_LOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

//
// Context of the current program being running.
//
struct _fpm_context_t;
typedef struct _fpm_context_t fpm_context_t;

//
// Definition of exported procedure for dynamic linking.
//
typedef struct {
    const char *name;       // Name of procedure
    void *address;          // Address of procedure
} fpm_binding_t;

//
// Load dynamic binary.
// Return true on success.
//
bool fpm_load(fpm_context_t *ctx, const char *filename);

//
// Unmap ELF binary from memory.
//
void fpm_unload(fpm_context_t *ctx);

//
// Internal platform-dependent helper routines.
//
bool fpm_load_arch(fpm_context_t *ctx, const char *filename);
void fpm_unload_arch(fpm_context_t *ctx);

//
// Get names of linked procedures.
//
void fpm_get_symbols(fpm_context_t *ctx, const char *symbols[]);

//
// Invoke entry address of the ELF binary with argc, argv arguments.
// Bind dynamic symbols of the binary according to the given linkmap.
// Assume the entry has signature:
//
//      int main(int argc, char *argv[])
//
// Return the exit code.
//
bool fpm_invoke(fpm_context_t *ctx, fpm_binding_t linkmap[], int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif // FPM_LOADER_H
