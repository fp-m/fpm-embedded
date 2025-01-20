//
// Memory allocation routines.
//
// Free memory is stored as a list sorted by address in ascending order.
// The first segment that fits in size is selected.
// This memory allocator minimizes memory waste, which can make things
// run a little slower, but it also increases the chances of things running smoothly.
//
#include <fpm/api.h>
#include <fpm/context.h>
#include <fpm/internal.h>

//
// Debug configuration.
//
// #define MEM_DEBUG 1

#ifdef NDEBUG // Disable memory debugging on NDEBUG
#undef MEM_DEBUG
#endif

#ifndef MEM_DEBUG // By default memory debugging is disabled
#define MEM_DEBUG 0
#endif

//
// Every memory block has a header.
//
typedef struct {
    // Block size including the header.
    size_t size;

#if MEM_DEBUG
    unsigned magic; // For data curruption test
#endif
} heap_header_t;

//
// In free blocks, the space just after the header
// is used as a pointer to the next free block (linked free list).
//
#define NEXT(h) (*(heap_header_t **)((h) + 1))

//
// Magic values for debug.
//
enum {
    HEAP_BUSY_MAGIC = 0x42555359, // Memory block in use
    HEAP_GAP_MAGIC = 0x47415021,  // Free memory block
};

//
// Align allocations on this granularity.
//
static unsigned const SIZEOF_POINTER = sizeof(void *);

//
// Descriptor of current program being running.
//
volatile fpm_context_t *fpm_context;

//
// Memory alignment.
// Align data on pointer-sized boundaries.
//
static inline size_t size_align(size_t nbytes)
{
    return (nbytes + SIZEOF_POINTER - 1) & -SIZEOF_POINTER;
}

//
// Allocate memory of given size.
// Fill it with zeroes.
//
void *fpm_alloc(size_t nbytes)
{
    void *p = fpm_alloc_dirty(nbytes);
    if (p && nbytes > 0) {
        memset(p, 0, nbytes);
    }
    return p;
}

//
// Allocate memory of given size.
// The memory may contain garbage.
//
void *fpm_alloc_dirty(size_t nbytes)
{
    heap_header_t *h, **hprev, *newh;

    // All allocations need to be several bytes larger than the
    // amount requested by our caller.  They also need to be large enough
    // that they can contain a "heap_header_t" and any magic values used in
    // debugging (for when the block gets freed and becomes an isolated
    // free block).
    if (nbytes < SIZEOF_POINTER) {
        nbytes = SIZEOF_POINTER;
    }
    nbytes = size_align(nbytes + sizeof(heap_header_t));

    // Scan the list of all available free blocks and find the first
    // one that meets our requirement.
    h = (heap_header_t *)fpm_context->free_list;
    hprev = (heap_header_t **)(void *)&fpm_context->free_list;
    while (h) {
#if MEM_DEBUG
        if (h->magic != HEAP_GAP_MAGIC) {
            fpm_printf("fpm_alloc: bad block magic at %p, size=%zu\n", h, h->size);
            fpm_reboot();
        }
#endif
        if (h->size >= nbytes) {
            break;
        }

        hprev = &NEXT(h);
        h = NEXT(h);
    }

    // Did we find any space available?
    if (!h) {
        // fpm_printf ("fpm_alloc failed, size=%zu bytes\n", nbytes);
        return 0;
    }

    // Remove a chunk of space and, if we can, release any of what's left
    // as a new free block.  If we can't release any then allocate more than was
    // requested and remove this block from the free list.
    if (h->size >= nbytes + sizeof(heap_header_t) + 2 * SIZEOF_POINTER) {
        newh = (heap_header_t *)((size_t)h + nbytes);
        newh->size = h->size - nbytes;
        h->size = nbytes;
        NEXT(newh) = NEXT(h);
        *hprev = newh;
#if MEM_DEBUG
        newh->magic = HEAP_GAP_MAGIC;
#endif
    } else {
        *hprev = NEXT(h);
    }

#if MEM_DEBUG
    h->magic = HEAP_BUSY_MAGIC;
#endif
    fpm_context->free_size -= h->size;
    // fpm_printf("fpm_alloc_dirty: return %p, size %zu bytes\n", h+1, h->size);
    return h + 1;
}

//
// Add new block to the free list.
//
static void make_free_block(heap_header_t *newh)
{
    fpm_context->free_size += newh->size;
#if MEM_DEBUG
    newh->magic = HEAP_GAP_MAGIC;
#endif

    //
    // Walk through the free list and see if this newly freed block can
    // be merged with another block to form a larger space.  Whatever
    // happens, we still ensure that the list is ordered lowest-addressed
    // block first through to highest-addressed-block last.
    //
    heap_header_t *h = (heap_header_t *)fpm_context->free_list;
    heap_header_t **hprev = (heap_header_t **)(void *)&fpm_context->free_list;
    for (;;) {
        if (!h) {
            // At the end of free list
            *hprev = newh;
            NEXT(newh) = 0;
            break;
        }

        if ((size_t)h > (size_t)newh) {
            // Insert the new block before the old one
            *hprev = newh;
            if (((size_t)newh + newh->size) == (size_t)h) {
                newh->size += h->size;
                NEXT(newh) = NEXT(h);
            } else {
                NEXT(newh) = h;
            }
            break;
        }

        if (((size_t)h + h->size) == (size_t)newh) {
            // Append the new block at the end of the old one
            h->size += newh->size;
            if (((size_t)h + h->size) == (size_t)NEXT(h)) {
                h->size += NEXT(h)->size;
                NEXT(h) = NEXT(NEXT(h));
            }
            break;
        }

        hprev = &NEXT(h);
        h = NEXT(h);
    }
}

//
// Release previously allocated block of memory.
//
void fpm_free(void *block)
{
    if (!block) {
        return;
    }

    // Make the header pointer.
    heap_header_t *h = (heap_header_t *)block - 1;
#if MEM_DEBUG
    if (h->magic != HEAP_BUSY_MAGIC) {
        fpm_printf("fpm_free: bad block magic 0x%x\n", h->magic);
        fpm_reboot();
    }
#endif

    // Convert our block into a free one.
    make_free_block(h);
}

//
// Change size of previosly allocated block of memory.
// Return new pointer.
//
void *fpm_realloc(void *old_block, size_t bytes)
{
    if (!old_block) {
        return 0;
    }

    // Make the header pointer.
    heap_header_t *h = (heap_header_t *)old_block - 1;
#if MEM_DEBUG
    if (h->magic != HEAP_BUSY_MAGIC) {
        fpm_printf("fpm_realloc: bad block magic 0x%x\n", h->magic);
        fpm_reboot();
    }
#endif
    size_t old_size = h->size - sizeof(heap_header_t);
    if (old_size >= bytes) {
        return old_block;
    }

    void *block = fpm_alloc(bytes);
    if (!block) {
        make_free_block(h);
        return 0;
    }
    memcpy(block, old_block, old_size);
    make_free_block(h);
    return block;
}

//
// Reduce memory block to a given size.
// The pointer is not changed.
//
void fpm_truncate(void *block, size_t nbytes)
{
    if (!block) {
        return;
    }

    // Add the size of header.
    if (nbytes < SIZEOF_POINTER) {
        nbytes = SIZEOF_POINTER;
    }
    nbytes = size_align(nbytes + sizeof(heap_header_t));

    // Make the header pointer.
    heap_header_t *h = (heap_header_t *)block - 1;
#if MEM_DEBUG
    if (h->magic != HEAP_BUSY_MAGIC) {
        fpm_printf("fpm_truncate: bad block magic 0x%x\n", h->magic);
        fpm_reboot();
    }
#endif
    // Is there enough space to split?
    if (h->size >= nbytes + sizeof(heap_header_t) + 2 * SIZEOF_POINTER) {
        // Split into two blocks.
        heap_header_t *newh = (heap_header_t *)((size_t)h + nbytes);
        newh->size = h->size - nbytes;

        h->size = nbytes;
        make_free_block(newh);
    }
}

//
// Return the amount of heap space that's still available.
//
size_t fpm_heap_available()
{
    return fpm_context->free_size;
}

//
// Return size of the previosly allocated block of memory.
//
size_t fpm_sizeof(void *block)
{
    if (!block) {
        return 0;
    }

    // Make the header pointer.
    heap_header_t *h = (heap_header_t *)block - 1;
#if MEM_DEBUG
    if (h->magic != HEAP_BUSY_MAGIC) {
        fpm_printf("fpm_sizeof: bad block magic 0x%x\n", h->magic);
        fpm_reboot();
    }
#endif
    return h->size - sizeof(heap_header_t);
}

//
// Print list of free blocks in the heap, for debug.
//
#if MEM_DEBUG
void fpm_heap_print_free_list()
{
    fpm_printf("free list:");
    for (heap_header_t *h = fpm_context->free_list; h; h = NEXT(h)) {
        fpm_printf(" %p-%p", h, (char *)h + h->size - 1);
    }
    fpm_printf("\n");
}
#endif

static void fpm_heap_setup(size_t start, size_t nbytes)
{
    fpm_context->heap_start = start;
    fpm_context->heap_size  = nbytes;

    heap_header_t *h = (heap_header_t *)start;
    h->size = nbytes;
#if MEM_DEBUG
    h->magic = HEAP_GAP_MAGIC;
#endif
    NEXT(h) = fpm_context->free_list;
    fpm_context->free_list = h;
    fpm_context->free_size = h->size;
}

//
// Initialize the heap for dynamic allocation.
//
void fpm_heap_init(fpm_context_t *ctx, size_t start, size_t nbytes)
{
    // fpm_printf("fpm_heap_init: start=0x%zx, size %zu bytes\n", start, nbytes);

    // Link this context into the chain.
    memset(ctx, 0, sizeof(*ctx));
    ctx->parent = fpm_context;
    fpm_context = ctx;

    fpm_heap_setup(start, nbytes);
}

//
// Push context into a chain.
// Return false when cannot allocate a nested heap.
//
bool fpm_context_push(fpm_context_t *ctx)
{
    // Allocate new heap.
    // Scan the list of all available free blocks and find the largest one.
    heap_header_t *max_block = NULL;
    heap_header_t *h = (heap_header_t *)fpm_context->free_list;
    while (h) {
#if MEM_DEBUG
        if (h->magic != HEAP_GAP_MAGIC) {
            fpm_printf("fpm_context_push: bad block magic at %p, size=%zu\n", h, h->size);
            fpm_reboot();
        }
#endif
        if (max_block == NULL || h->size > max_block->size) {
            // This block is bigger.
            max_block = h;
        }
        h = NEXT(h);
    }

    // Did we find any space available?
    if (max_block == NULL || max_block->size < 1024) {
        return false;
    }

    // Switch to new context.
    ctx->parent = fpm_context;
    fpm_context = ctx;

    // Skip header and the 'next' pointer.
    const size_t skip = sizeof(heap_header_t) + sizeof(void*);
    fpm_heap_setup((size_t)max_block + skip, max_block->size - skip);
    return true;
}

//
// Pop context from chain.
//
void fpm_context_pop()
{
    fpm_context = fpm_context->parent;
}
