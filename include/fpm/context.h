//
// Program context.
//
#ifndef FPM_CONTEXT_H
#define FPM_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

//
// Context of the current program being running.
//
typedef struct _fpm_context_t {
    // Pointer to a parent program, or NULL.
    struct _fpm_context_t *parent;

    // Heap info for memory allocation.
    size_t heap_start;  // Heap starts at this address
    size_t heap_size;   // Size of the heap in bytes
    size_t free_size;   // Total amount of free memory
    void *free_list;    // Linked list of memory gaps, sorted by address in ascending order

    // Info about current program being executed.
    void *base;              // File is mapped at this address
    unsigned num_links;      // Number of linked procedures
    const void *rel_section; // Header of .rela.plt section
    int exit_code;           // Return value of invoked object

    // For Unix only.
    int fd;           // File descriptor
    size_t file_size; // Size of file in bytes
} fpm_context_t;

#ifdef __cplusplus
}
#endif

#endif // FPM_CONTEXT_H
