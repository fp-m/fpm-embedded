//
// Memory allocation routines.
//
// Free memory is stored as a list sorted by address in ascending order.
// The first segment that fits in size is selected.
// This memory allocator minimizes memory waste, which can make things
// run a little slower, but it also increases the chances of things running smoothly.
//
#include <rpm/api.h>
#include <rpm/internal.h>

//
// Debug configuration.
//
#define MEM_DEBUG 1

#ifdef NDEBUG // Disable memory debugging on NDEBUG
#undef MEM_DEBUG
#endif

#ifndef MEM_DEBUG // By default memory debugging is disabled
#define MEM_DEBUG 0
#endif

typedef struct {
    // Total amount of free memory.
    size_t free_size;

    // A linked list of memory gaps, sorted by address in ascending order.
    void *free_list;
} heap_pool_t;

static heap_pool_t heap_pool;

//
// Memory alignment.
// Align data on pointer-sized boundaries.
//
#define SIZEOF_POINTER sizeof(void *)
#define MEM_ALIGN(x) (((x) + SIZEOF_POINTER - 1) & -SIZEOF_POINTER)

//
// Every memory block has a header.
//
typedef struct {
    // Block size including the header.
    size_t size;

#if MEM_DEBUG
    unsigned magic;                  // For data curruption test
#define MEMORY_BUSY_MAGIC 0x42555359 // Memory block in use
#define MEMORY_GAP_MAGIC  0x47415021 // Free memory block
#endif
} heap_header_t;

//
// In memory gaps (free blocks), the space just after the header
// is used as a pointer to the next gap (linked free list).
//
#define NEXT(h) (*(heap_header_t **)((h) + 1))

//
// Allocate a block of memory.
// Fill it with zeroes.
//
void *rpm_alloc(size_t nbytes)
{
    void *p = rpm_alloc_dirty(nbytes);
    if (p && nbytes > 0) {
        memset(p, 0, nbytes);
    }
    return p;
}

//
// Allocate a block of memory.
// The memory may contain garbage.
//
void *rpm_alloc_dirty(size_t nbytes)
{
    heap_header_t *h, **hprev, *newh;

    // All allocations need to be several bytes larger than the
    // amount requested by our caller.  They also need to be large enough
    // that they can contain a "heap_header_t" and any magic values used in
    // debugging (for when the block gets freed and becomes an isolated
    // gap).
    if (nbytes < SIZEOF_POINTER)
        nbytes = SIZEOF_POINTER;
    nbytes = MEM_ALIGN(nbytes + sizeof(heap_header_t));

    // Scan the list of all available memory gaps and find the first
    // one that meets our requirement.
    h = (heap_header_t *)heap_pool.free_list;
    hprev = (heap_header_t **)(void *)&heap_pool.free_list;
    while (h) {
#if MEM_DEBUG
        if (h->magic != MEMORY_GAP_MAGIC) {
            rpm_printf("mem: bad gap magic at 0x%x\n", h);
            rpm_printf("     size=%d\n", h->size);
            rpm_reboot();
        }
#endif
        if (h->size >= nbytes)
            break;

        hprev = &NEXT(h);
        h = NEXT(h);
    }

    // Did we find any space available?
    if (!h) {
        // rpm_printf ("rpm_alloc failed, size=%d bytes\n", nbytes);
        return 0;
    }

    // Remove a chunk of space and, if we can, release any of what's left
    // as a new gap.  If we can't release any then allocate more than was
    // requested and remove this gap from the gap list.
    if (h->size >= nbytes + sizeof(heap_header_t) + 2 * SIZEOF_POINTER) {
        newh = (heap_header_t *)((size_t)h + nbytes);
        newh->size = h->size - nbytes;
        h->size = nbytes;
        NEXT(newh) = NEXT(h);
        *hprev = newh;
#if MEM_DEBUG
        newh->magic = MEMORY_GAP_MAGIC;
#endif
    } else {
        *hprev = NEXT(h);
    }

#if MEM_DEBUG
    h->magic = MEMORY_BUSY_MAGIC;
#endif
    heap_pool.free_size -= h->size;
    // rpm_printf ("mem %d bytes returned 0x%x\n", h->size, h+1);
    return h + 1;
}

//
// Add new gap to the free list.
//
static void make_gap(heap_header_t *newh)
{
    heap_pool.free_size += newh->size;
#if MEM_DEBUG
    newh->magic = MEMORY_GAP_MAGIC;
#endif

    //
    // Walk through the gap list and see if this newly freed block can
    // be merged with another block to form a larger space.  Whatever
    // happens, we still ensure that the list is ordered lowest-addressed
    // gap first through to highest-addressed-gap last.
    //
    heap_header_t *h = (heap_header_t *)heap_pool.free_list;
    heap_header_t **hprev = (heap_header_t **)(void *)&heap_pool.free_list;
    for (;;) {
        if (!h) {
            // At the end of free list
            *hprev = newh;
            NEXT(newh) = 0;
            break;
        }

        if ((size_t)h > (size_t)newh) {
            // Insert the new gap before the old one
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
            // Append the new gap at the end of the old one
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
// Release a block of memory.
//
void rpm_free(void *block)
{
    if (!block)
        return;

    // Make the header pointer.
    heap_header_t *h = (heap_header_t *)block - 1;
#if MEM_DEBUG
    if (h->magic != MEMORY_BUSY_MAGIC) {
        rpm_printf("free: bad block magic 0x%x\n", h->magic);
        rpm_reboot();
    }
#endif

    // Convert our block into a gap.
    make_gap(h);
}

void *rpm_realloc(void *old_block, size_t bytes)
{
    if (!old_block)
        return 0;

    // Make the header pointer.
    heap_header_t *h = (heap_header_t *)old_block - 1;
#if MEM_DEBUG
    if (h->magic != MEMORY_BUSY_MAGIC) {
        rpm_printf("realloc: bad block magic 0x%x\n", h->magic);
        rpm_reboot();
    }
#endif
    size_t old_size = h->size - sizeof(heap_header_t);
    if (old_size >= bytes)
        return old_block;

    void *block = rpm_alloc(bytes);
    if (!block) {
        make_gap(h);
        return 0;
    }
    memcpy(block, old_block, old_size);
    make_gap(h);
    return block;
}

void rpm_alloc_truncate(void *block, size_t nbytes)
{
    if (!block)
        return;

    // Add the size of header.
    if (nbytes < SIZEOF_POINTER)
        nbytes = SIZEOF_POINTER;
    nbytes = MEM_ALIGN(nbytes + sizeof(heap_header_t));

    // Make the header pointer.
    heap_header_t *h = (heap_header_t *)block - 1;
#if MEM_DEBUG
    if (h->magic != MEMORY_BUSY_MAGIC) {
        rpm_printf("truncate: bad block magic 0x%x\n", h->magic);
        rpm_reboot();
    }
#endif
    // Is there enough space to split?
    if (h->size >= nbytes + sizeof(heap_header_t) + 2 * SIZEOF_POINTER) {
        // Split into two blocks.
        heap_header_t *newh = (heap_header_t *)((size_t)h + nbytes);
        newh->size = h->size - nbytes;

        h->size = nbytes;
        make_gap(newh);
    }
}

//
// Return the amount of heap space that's still available.
//
size_t rpm_heap_available()
{
    return heap_pool.free_size;
}

//
// Return the size of the given block.
//
size_t rpm_alloc_size(void *block)
{
    if (!block)
        return 0;

    // Make the header pointer.
    heap_header_t *h = (heap_header_t *)block - 1;
#if MEM_DEBUG
    if (h->magic != MEMORY_BUSY_MAGIC) {
        rpm_printf("size: bad block magic 0x%x\n", h->magic);
        rpm_reboot();
    }
#endif
    return h->size - sizeof(heap_header_t);
}

#if MEM_DEBUG
void rpm_heap_print_free_list()
{
    rpm_printf("free list:");
    for (heap_header_t *h = heap_pool.free_list; h; h = NEXT(h)) {
        rpm_printf(" %p-%p", h, (char *)h + h->size - 1);
    }
    rpm_printf("\n");
}
#endif

//
// Initialize the memory for dynamic allocation.
//
void rpm_heap_init(size_t start, size_t nbytes)
{
    // rpm_printf ("heap_init start=0x%x, size %d bytes\n", start, nbytes);

    heap_header_t *h = (heap_header_t *)start;
    h->size = nbytes;
#if MEM_DEBUG
    h->magic = MEMORY_GAP_MAGIC;
#endif
    NEXT(h) = heap_pool.free_list;
    heap_pool.free_list = h;
    heap_pool.free_size += h->size;
}
