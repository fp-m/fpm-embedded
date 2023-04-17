//
// Test rpm_get_dotw() - Day Of The Week function.
//
#include <gtest/gtest.h>
#include <rpm/api.h>

//
// Allocate about this amount of data in total.
//
static const unsigned TOTAL_BYTES_THRESHOLD = 1 * 1024 * 1024;

//
// Max size of one allocation.
//
static const unsigned MAX_ALLOC_BYTES = 4 * 1024;

//
// Keep up to N allocations at a time.
//
static const unsigned MAX_ALLOCATIONS = TOTAL_BYTES_THRESHOLD / MAX_ALLOC_BYTES * 2;

//
// Force pages to be populated.
//
static const bool OPTION_TOUCH_DATA = true;

//
// Zero free'd memory.
//
static const bool OPTION_ZERO_ON_FREE = true;

//
// Verify contents of allocated blocks.
//
static const bool OPTION_VERIFY = true;

typedef struct {
    uintptr_t *addr; // Address of allocation
    unsigned len;    // Allocation length
} alloc_info_t;

//
// Convert value to string as hex number.
//
template <typename T>
static std::string to_hex_string(T val)
{
    std::stringstream buf;
    buf << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << val;
    return buf.str();
}

//
// Allocate memory with unpredictable contents.
//
void *my_malloc(unsigned nbytes)
{
    std::cout << "--- malloc " << nbytes << " bytes";
    return malloc(nbytes);
}

//
// Allocate memory zeroed out.
//
void *my_calloc(unsigned count, unsigned size)
{
    std::cout << "--- calloc " << (count * size) << " bytes";
    return calloc(count, size);
}

//
// Free memory, optionally zero it out.
//
static void my_free(void *ptr, unsigned nbytes)
{
    if (nbytes > 0 && OPTION_ZERO_ON_FREE) {
        memset(ptr, 0, nbytes);
    }
    std::cout << "--- free " << nbytes << " bytes" << std::endl;
    free(ptr);
}

//
// Touch data in the allocated buffer.
//
static void page_touch(void *buffer, const unsigned nbytes)
{
    auto ptr = (uint8_t *)buffer;
    const auto end = ptr + nbytes;
    static const unsigned PAGE_SIZE = 1 * 1024;

    while (ptr < end) {
        *ptr = 0xa5;
        ptr += PAGE_SIZE;
    }
}

//
// Stress malloc by performing a mix of allocations and frees.
//
static void test_loop(unsigned max_count)
{
    alloc_info_t info[MAX_ALLOCATIONS];
    memset(info, 0, sizeof(info));

    size_t total_allocated = 0;
    while (max_count-- > 0) {
        const unsigned index = random() % MAX_ALLOCATIONS;
        alloc_info_t *item = &info[index];

        // Allocate random size, but enough for uintptr_t.
        unsigned len = random() % MAX_ALLOC_BYTES;
        if (len < sizeof(uintptr_t)) {
            len = sizeof(uintptr_t);
        }

        const bool fifty_fifty = random() & 1;
        if (item->addr) {
            //
            // 50% free, 50% realloc
            //
            total_allocated -= item->len;
            if (fifty_fifty || total_allocated >= TOTAL_BYTES_THRESHOLD) {
                if (OPTION_VERIFY && (uintptr_t)item->addr != *item->addr) {
                    throw std::runtime_error("Allocation at " + to_hex_string(item->addr) +
                                             ": bad value " + to_hex_string(*item->addr));
                }
                my_free(item->addr, item->len);
                item->addr = NULL;
                item->len = 0;
            } else {
                void *ptr = realloc(item->addr, len);
                if (ptr) {
                    total_allocated += len;
                    item->addr = (uintptr_t *)ptr;
                    item->len = len;

                    if (OPTION_TOUCH_DATA) {
                        page_touch((void *)item->addr, item->len);
                    }

                    *item->addr = (uintptr_t)item->addr; // stash address
                    if (OPTION_VERIFY && (uintptr_t)item->addr != *item->addr) {
                        throw std::runtime_error("Allocation at " + to_hex_string(item->addr) +
                                                 ": bad value " + to_hex_string(*item->addr));
                    }
                }
            }
        } else if (total_allocated < TOTAL_BYTES_THRESHOLD) {
            //
            // 50% malloc, 50% calloc
            //
            if (fifty_fifty) {
                item->addr = (uintptr_t *)my_malloc(len);
            } else {
                item->addr = (uintptr_t *)my_calloc(1, len);
            }

            if (item->addr) {
                total_allocated += len;
                if (OPTION_TOUCH_DATA) {
                    page_touch((void *)item->addr, len);
                }
                *item->addr = (uintptr_t)item->addr; // stash address
                item->len = len;
            } else {
                item->len = 0;
            }
            std::cout << ", total " << total_allocated << std::endl;
        }
    }

    unsigned index;
    for (index = 0; index < MAX_ALLOCATIONS; index++) {
        alloc_info_t *item = &info[index];
        if (item->addr) {
            if (OPTION_VERIFY && item->addr && (uintptr_t)item->addr != *item->addr) {
                throw std::runtime_error("Allocation at " + to_hex_string(item->addr) + ": bad value " +
                                         to_hex_string(*item->addr));
            }
            my_free(item->addr, item->len);
        }
    }
}

//
// Fail gracefully in case of SIGSEGV signal.
//
static void sig_segv(int)
{
    FAIL() << "Signal caught: segmentation violation";
}

TEST(mem, alloc_free)
{
    signal(SIGSEGV, sig_segv);

    const unsigned MAX_COUNT = 1000000;
    test_loop(MAX_COUNT);
}
