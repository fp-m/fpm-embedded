//
// Memory allocation routines.
//
// Free memory is stored as a list sorted by address in ascending order.
// The first segment that fits in size is selected.
// This memory allocator minimizes memory waste, which can make things
// run a little slower, but it also increases the chances of things running smoothly.
//
#include <rpm/api.h>

typedef struct {
    size_t free_size; // The amount of free memory.

    void *free_list;  // Linked list of memory holes, ordered lowest-addressed block first.
} mem_pool_t;

static mem_pool_t mem_pool;

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
    size_t size;                  // Block size including the header
#if MEM_DEBUG
    unsigned short magic;         // For data curruption test
#define MEMORY_HOLE_MAGIC 0x4d48  // Free memory block (hole)
#define MEMORY_BLOCK_MAGIC 0x4d42 // Memory block in use
#endif
} mheader_t;

//
// In memory holes (free blocks), the space just after the header
// is used as a pointer to the next hole (linked free list).
//
#define NEXT(h) (*(mheader_t **)((h) + 1))

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
    mheader_t *h, **hprev, *newh;

    // All allocations need to be several bytes larger than the
    // amount requested by our caller.  They also need to be large enough
    // that they can contain a "mheader_t" and any magic values used in
    // debugging (for when the block gets freed and becomes an isolated
    // hole).
    if (nbytes < SIZEOF_POINTER)
        nbytes = SIZEOF_POINTER;
    nbytes = MEM_ALIGN(nbytes + sizeof(mheader_t));

    // Scan the list of all available memory holes and find the first
    // one that meets our requirement.
    h = (mheader_t *)mem_pool.free_list;
    hprev = (mheader_t **)(void *)&mem_pool.free_list;
    while (h) {
#if MEM_DEBUG
        if (h->magic != MEMORY_HOLE_MAGIC) {
            debug_printf("mem: bad hole magic at 0x%x\n", h);
            debug_printf("     size=%d\n", h->size);
            uos_halt(1);
        }
#endif
        if (h->size >= nbytes)
            break;

        hprev = &NEXT(h);
        h = NEXT(h);
    }

    // Did we find any space available?
    if (!h) {
        // debug_printf ("rpm_alloc failed, size=%d bytes\n", nbytes);
        return 0;
    }

    // Remove a chunk of space and, if we can, release any of what's left
    // as a new hole.  If we can't release any then allocate more than was
    // requested and remove this hole from the hole list.
    if (h->size >= nbytes + sizeof(mheader_t) + 2 * SIZEOF_POINTER) {
        newh = (mheader_t *)((size_t)h + nbytes);
        newh->size = h->size - nbytes;
        h->size = nbytes;
        NEXT(newh) = NEXT(h);
        *hprev = newh;
#if MEM_DEBUG
        newh->magic = MEMORY_HOLE_MAGIC;
#endif
    } else {
        *hprev = NEXT(h);
    }

#if MEM_DEBUG
    h->magic = MEMORY_BLOCK_MAGIC;
#endif
    mem_pool.free_size -= h->size;
    // debug_printf ("mem %d bytes returned 0x%x\n", h->size, h+1);
    return h + 1;
}

//
// Add new hole to the free list.
//
static void mem_make_hole(mheader_t *newh)
{
    mem_pool.free_size += newh->size;
#if MEM_DEBUG
    newh->magic = MEMORY_HOLE_MAGIC;
#endif

    //
    // Walk through the hole list and see if this newly freed block can
    // be merged with another block to form a larger space.  Whatever
    // happens, we still ensure that the list is ordered lowest-addressed
    // hole first through to highest-addressed-hole last.
    //
    mheader_t *h = (mheader_t *)mem_pool.free_list;
    mheader_t **hprev = (mheader_t **)(void *)&mem_pool.free_list;
    for (;;) {
        if (!h) {
            // At the end of free list
            *hprev = newh;
            NEXT(newh) = 0;
            break;
        }

        if ((size_t)h > (size_t)newh) {
            // Insert the new hole before the old one
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
            // Append the new hole at the end of the old one
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
    mheader_t *h = (mheader_t *)block - 1;
#if MEM_DEBUG
    if (h->magic != MEMORY_BLOCK_MAGIC) {
        debug_printf("free: bad block magic\n");
        uos_halt(1);
    }
#endif

    // Convert our block into a hole.
    mem_make_hole(h);
}

void *rpm_realloc(void *old_block, size_t bytes)
{
    if (!old_block)
        return 0;

    // Make the header pointer.
    mheader_t *h = (mheader_t *)old_block - 1;
#if MEM_DEBUG
    if (h->magic != MEMORY_BLOCK_MAGIC) {
        debug_printf("realloc: bad block magic\n");
        uos_halt(1);
    }
#endif
    size_t old_size = h->size - sizeof(mheader_t);
    if (old_size >= bytes)
        return old_block;

    void *block = rpm_alloc(bytes);
    if (!block) {
        mem_make_hole(h);
        return 0;
    }
    memcpy(block, old_block, old_size);
    mem_make_hole(h);
    return block;
}

void mem_truncate(void *block, size_t nbytes)
{
    if (!block)
        return;

    // Add the size of header.
    if (nbytes < SIZEOF_POINTER)
        nbytes = SIZEOF_POINTER;
    nbytes = MEM_ALIGN(nbytes + sizeof(mheader_t));

    // Make the header pointer.
    mheader_t *h = (mheader_t *)block - 1;
#if MEM_DEBUG
    if (h->magic != MEMORY_BLOCK_MAGIC) {
        debug_printf("truncate: bad block magic\n");
        uos_halt(1);
    }
#endif
    // Is there enough space to split?
    if (h->size >= nbytes + sizeof(mheader_t) + 2 * SIZEOF_POINTER) {
        // Split into two blocks.
        mheader_t *newh = (mheader_t *)((size_t)h + nbytes);
        newh->size = h->size - nbytes;

        h->size = nbytes;
        mem_make_hole(newh);
    }
}

//
// Return the amount of heap space that's still available.
//
size_t mem_available()
{
    return mem_pool.free_size;
}

//
// Return the size of the given block.
//
size_t mem_size(void *block)
{
    if (!block)
        return 0;

    // Make the header pointer.
    mheader_t *h = (mheader_t *)block - 1;
#if MEM_DEBUG
    if (h->magic != MEMORY_BLOCK_MAGIC) {
        debug_printf("size: bad block magic\n");
        uos_halt(1);
    }
#endif
    return h->size - sizeof(mheader_t);
}

#if MEM_DEBUG
void mem_print_free_list()
{
    debug_printf("free list:");
    for (mheader_t *h = mem_pool.free_list; h; h = NEXT(h)) {
        debug_printf(" %p-%p", h, (char *)h + h->size - 1);
    }
    debug_printf("\n");
}
#endif

//
// Initialize the memory for dynamic allocation.
//
void mem_init(size_t start, size_t stop)
{
    mheader_t *h = (mheader_t *)start;

    // debug_printf ("mem_init start=0x%x, size %d bytes\n", start, size);
    assert(stop > start);
    h->size = stop - start;
#if MEM_DEBUG
    h->magic = MEMORY_HOLE_MAGIC;
#endif
    NEXT(h) = mem_pool.free_list;
    mem_pool.free_list = h;
    mem_pool.free_size += h->size;
}
