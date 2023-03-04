//
// Clear the console screen
//
#include <rpm/api.h>
#include <rpm/getopt.h>
#include <rpm/internal.h>
#include <rpm/fs.h>
#include <stdlib.h>

//
// Options of the listing format.
//
typedef struct {
    unsigned termwidth; // terminal width
    bool columnated;    // columnated format
    bool showhidden;    // show hidden files
    bool longform;      // long listing format
    bool recursive;     // ls subdirectories also
    bool reversesort;   // reverse whatever sort is used
    bool singlecol;     // use single column output
    bool timesort;      // sort by time vice name
    bool type;          // add type character for non-regular files
    bool toplevel;      // at top level
    bool any_output;    // if emitted any output
} options_t;

//
// Item in the single-linked list: info about one file or directory.
//
typedef struct _list_item_t {
    struct _list_item_t *next; // Next file in the list
    uint64_t nbytes;           // File size
    uint32_t mtime;            // Modification time
    uint32_t namelen;          // Strlen of the file name
    uint8_t attrib;            // Dir/System/Hidden/Readonly
    uint8_t no_print;          // Don't print
    char name[1];              // File name, dynamically allocated, zero terminated
} list_item_t;

//
// Single-linked list of files.
//
typedef struct {
    list_item_t *head; // First file in the list
    list_item_t *tail;  // Last file in the list
} file_list_t;

// Forward declaration of a recursive routine.
static void show_dir(const char *path, options_t *options);

//
// Is the list empty?
//
static bool list_is_empty(file_list_t *list)
{
    return list->head == 0;
}

//
// Free the list.
//
static void list_delete(file_list_t *list)
{
    list_item_t *item, *next;
    for (item = list->head; item != 0; item = next) {
        next = item->next;
        free(item);
    }
    list->head = 0;
    list->tail = 0;
}

//
// Append file info to the list.
// Ignore if no such file exists.
//
static void list_append_file_info(file_list_t *list, const file_info_t *info)
{
    // Get file name.
    const char *file_name = info->fname[0] ? info->fname : info->altname;
    const unsigned name_nbytes = strlen(file_name);

    // Allocate list item.
    list_item_t *file = calloc(1, sizeof(*file) + name_nbytes);
    if (!file) {
        rpm_printf("%s: Out of memory\r\n", file_name);
        return;
    }

    // Fill file data.
    strcpy(file->name, file_name);
    file->next = 0;
    file->namelen = rpm_utf8len(file_name);
    file->nbytes = info->fsize;
    file->mtime = (uint32_t)info->fdate << 16 | info->ftime;
    file->attrib = info->fattrib;

    // Append to the list.
    if (list->head == 0) {
        list->head = file;
        list->tail = file;
    } else {
        list->tail->next = file;
        list->tail = file;
    }
}

//
// Append file name to the list.
// Ignore if no such file exists.
//
static void list_append_name(file_list_t *list, const char *name)
{
    // Get file info.
    file_info_t info = {};
    fs_result_t result = f_stat(name, &info);
    if (result == FR_INVALID_NAME) {
        // Cannot stat current directory - fake it.
        info.fattrib = AM_DIR;
        strcpy(info.fname, name);
    } else if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", name, f_strerror(result));
        return;
    }

    list_append_file_info(list, &info);
}

//
// Compare list items.
// Return -1 when item a is less that item b.
//
static int compare_list_items(const list_item_t *a, const list_item_t *b, options_t *options)
{
    if (!options->reversesort) {
        if (options->timesort && a->mtime != b->mtime) {
            // Use modification time.
            return (int)b->mtime - (int)a->mtime;
        }
        return strcmp(a->name, b->name);
    } else {
        if (options->timesort && a->mtime != b->mtime) {
            // Use modification time.
            return (int)a->mtime - (int)b->mtime;
        }
        return strcmp(b->name, a->name);
    }
}

//
// Merge two single linked lists in ascending order.
//
static list_item_t *chain_merge(list_item_t *a, list_item_t *b, options_t *options)
{
    if (!a) {
        return b;
    }
    if (!b) {
        return a;
    }
    if (compare_list_items(a, b, options) < 0) {
        a->next = chain_merge(a->next, b, options);
        return a;
    } else {
        b->next = chain_merge(a, b->next, options);
        return b;
    }
}

//
// Find middle point of a single linked list.
//
static list_item_t *chain_mid_point(list_item_t *head)
{
    if (!head || !head->next) {
        return head;
    }
    list_item_t *fast = head;
    list_item_t *slow = head;
    while (fast && fast->next) {
        fast = fast->next;
        if (!fast->next) {
            break;
        }
        fast = fast->next;
        slow = slow->next;
    }
    return slow;
}

//
// Sort a linked list using recursive merge algorithm.
//
static list_item_t *chain_sort(list_item_t *head, options_t *options)
{
    if (!head || !head->next) {
        return head;
    }

    // Divide the linked list into two equal linked lists.
    list_item_t *mid = chain_mid_point(head);
    list_item_t *a = head;
    list_item_t *b = mid->next;
    mid->next = 0;

    // Recursively sort the smaller linked lists.
    a = chain_sort(a, options);
    b = chain_sort(b, options);

    // Merge the sorted linked lists.
    return chain_merge(a, b, options);
}

//
// Sort a linked list.
//
static void list_sort(file_list_t *list, options_t *options)
{
    list->head = chain_sort(list->head, options);

    // Restore tail.
    for (list->tail = list->head; list->tail != 0; list->tail = list->tail->next) {
        if (list->tail->next == 0) {
            break;
        }
    }
}

//
// Return true when the file is a symlink.
//
static bool is_link(list_item_t *item)
{
    //TODO: implement symlinks in RP/M.
    return false;
}

//
// Print symbol for file type.
//
static int print_type(list_item_t *item)
{
    if (item->attrib & AM_DIR) {
        rpm_putchar('/');
        return 1;
    }
    if (is_link(item)) {
        rpm_putchar('@');
        return 1;
    }
    return 0;
}

//
// Print file name and, optionally, file type symbol.
// return # of characters printed, no trailing characters.
//
static int print_filename(list_item_t *item, options_t *options)
{
    rpm_puts(item->name);
    int chcnt = item->namelen;
    if (options->type)
        chcnt += print_type(item);
    return chcnt;
}

//
// Print file attributes.
//
static void print_attrib(unsigned attrib)
{
    rpm_putchar((attrib & AM_ARC) ? 'a' : '-');
    rpm_putchar((attrib & AM_DIR) ? 'd' : '-');
    rpm_putchar((attrib & AM_SYS) ? 's' : '-');
    rpm_putchar((attrib & AM_HID) ? 'h' : '-');
    rpm_putchar((attrib & AM_RDO) ? 'r' : '-');
}

//
// Print date and time as yyyy/mm/dd hh:mm.
//
static void print_time(unsigned mtime)
{
    union {
        uint32_t word;
        struct {
            unsigned second2  : 5; // Second / 2 (0..29, e.g. 25 for 50)
            unsigned minute   : 6; // Minute (0..59)
            unsigned hour     : 5; // Hour (0..23)
            unsigned day      : 5; // Day of the month (1..31)
            unsigned month    : 4; // Month (1..12)
            unsigned year1980 : 7; // Year origin from the 1980 (0..127, e.g. 37 for 2017)
        } field;
    } u;
    u.word = mtime;

    rpm_printf("%4u/%u/%u ", u.field.year1980 + 1980, u.field.month, u.field.day);
    if (u.field.month < 10)
        rpm_putchar(' ');
    if (u.field.day < 10)
        rpm_putchar(' ');
    rpm_printf("%2u:%02u  ", u.field.hour, u.field.minute);
}

//
// Print symlink.
//
static void print_link(list_item_t *item)
{
    //TODO: read link contents from the file and print the path.
}

//
// Print files in -1 format.
//
static void print_single_column(file_list_t *list, options_t *options)
{
    list_item_t *item;
    for (item = list->head; item; item = item->next) {
        if (item->no_print)
            continue;

        print_filename(item, options);
        rpm_puts("\r\n");
    }
}

//
// Print integer number using comma as thousands separator.
//
static void print_size(uint64_t n, unsigned width)
{
    uint64_t n2 = 0;
    uint64_t scale = 1;
    while (n >= 1000) {
        n2 = n2 + scale * (n % 1000);
        n /= 1000;
        scale *= 1000;
        width -= 4;
    }
    rpm_printf("%*u", width, (unsigned) n);
    while (scale != 1) {
        scale /= 1000;
        n = n2 / scale;
        n2 = n2  % scale;
        rpm_printf(",%03u", (unsigned) n);
    }
}

//
// Estimate length of integer number printed using comma as thousands separator.
//
static unsigned size_len(uint64_t n)
{
    unsigned len = 0;
    while (n >= 1000) {
        n /= 1000;
        len += 4;
    }
    len += (n >= 100) ? 3 : (n >= 10) ? 2 : 1;
    return len;
}

//
// Print files in -l format.
//
static void print_long(file_list_t *list, options_t *options,
                       uint64_t kbytes_total, uint64_t maxsize)
{
    if (!options->toplevel && options->longform)
        rpm_printf("Total %ju kbytes\r\n", (uintmax_t)kbytes_total);

    unsigned size_width = size_len(maxsize);
    list_item_t *item;
    for (item = list->head; item; item = item->next) {
        if (item->no_print)
            continue;

        print_attrib(item->attrib);
        rpm_puts("  ");
        if (item->attrib & AM_DIR) {
            rpm_printf("%*s", size_width, "-");
        } else {
            print_size(item->nbytes, size_width);
        }
        rpm_puts("  ");
        print_time(item->mtime);
        rpm_puts(item->name);
        if (options->type) {
            print_type(item);
        }
        if (is_link(item)) {
            print_link(item);
        }
        rpm_puts("\r\n");
    }
}

//
// Print files in columns.
//
static void print_columnized(file_list_t *list, options_t *options,
                             int printable_entries, unsigned maxlen, uint64_t kbytes_total)
{
    //
    // Have to do random access in the linked list -- build a table of pointers.
    //
    list_item_t **array = alloca(printable_entries * sizeof(list_item_t *));
    unsigned num = 0;
    list_item_t *item;
    for (item = list->head; item; item = item->next) {
        if (!item->no_print) {
            array[num++] = item;
        }
    }

    // Compute column width.
    unsigned colwidth = maxlen;
    if (options->type) {
        colwidth += 1;
    }
    colwidth += 2;
    if (options->termwidth < 2 * colwidth) {
        // Not enough width for two columns.
        print_single_column(list, options);
        return;
    }

    // Compute number of columns and rows.
    unsigned numcols = options->termwidth / colwidth;
    unsigned numrows = num / numcols;
    if (num % numcols) {
        ++numrows;
    }

    if (!options->toplevel && options->longform)
        rpm_printf("Total %ju kbytes\r\n", (uintmax_t)kbytes_total);

    unsigned row;
    for (row = 0; row < numrows; ++row) {
        unsigned endcol = colwidth;
        unsigned base = row;
        unsigned char_count = 0;
        unsigned col;

        for (col = 0; col < numcols; ++col) {
            char_count += print_filename(array[base], options);
            base += numrows;
            if (base >= num)
                break;

            while (char_count < endcol) {
                rpm_putchar(' ');
                char_count++;
            }
            endcol += colwidth;
        }
        rpm_puts("\r\n");
    }
}

//
// Print list of files, according to given options.
//
static void show_files(file_list_t *list, options_t *options)
{
    if (list_is_empty(list)) {
        return;
    }

    // Sort the list.
    list_sort(list, options);

    uint64_t kbytes_total = 0;
    uint64_t maxsize = 0;
    unsigned maxlen = 0;
    int printable_entries = 0;
    list_item_t *item;
    for (item = list->head; item; item = item->next) {
        if (item->attrib & AM_HID) {
            // Only display hidden files if -a set.
            if (!options->showhidden) {
                item->no_print = 1;
                continue;
            }
        }
        if (item->attrib & AM_DIR) {
            // At top level, directories will be displayed later.
            if (options->toplevel) {
                item->no_print = 1;
                continue;
            }
        }
        if (item->namelen > maxlen)
            maxlen = item->namelen;

        if (item->nbytes > maxsize)
            maxsize = item->nbytes;

        kbytes_total += (item->nbytes + 1023) / 1024;

        ++printable_entries;
    }

    if (!printable_entries)
        return;

    // Select a print function.
    if (options->singlecol) {
        print_single_column(list, options);
    } else if (options->longform) {
        print_long(list, options, kbytes_total, maxsize);
    } else {
        print_columnized(list, options, printable_entries, maxlen, kbytes_total);
    }

    options->any_output = 1;
}

//
// For every directory in the list, call show_dir().
//
static void show_directories(file_list_t *list, options_t *options, const char *parent)
{
    if (list_is_empty(list)) {
        return;
    }

    // Allocate buffer for the directory path.
    unsigned parent_len = strlen(parent);
    char *path = alloca(parent_len + 1 + FF_LFN_BUF + 1);
    unsigned offset = *parent ? (parent_len + 1) : 0;
    strcpy(path, parent);
    path[parent_len] = '/';

    list_item_t *item;
    for (item = list->head; item; item = item->next) {
        if (item->attrib & AM_DIR) {
            strcpy(path + offset, item->name);

            // Print directory path.
            if (options->any_output) {
                // If already output something, put out a newline as a separator.
                rpm_printf("\r\n%s:\r\n", path);
            } else if (!options->toplevel || list->head->next != 0) {
                // If multiple arguments, precede each directory with its name.
                rpm_printf("%s:\r\n", path);
                options->any_output = 1;
            }

            show_dir(path, options);
        }
    }
}

//
// Get contents of directory with given path.
//
static void scan_directory(file_list_t *list, const char *path)
{
    // Scan the directory.
    directory_t *dir = alloca(f_sizeof_directory_t());
    fs_result_t result = f_opendir(dir, path);
    if (result != FR_OK) {
        rpm_printf("%s: %s\r\n", path, f_strerror(result));
        return;
    }
    file_info_t info = {};
    for (;;) {
        // Get directory entry.
        result = f_readdir(dir, &info);
        if (result != FR_OK || !info.fname[0]) {
            // End of directory.
            break;
        }
        list_append_file_info(list, &info);
    }
    f_closedir(dir);
}

//
// Get contents of the directory and show it.
//
static void show_dir(const char *path, options_t *options)
{
    // Scan the directory.
    file_list_t list = {};
    scan_directory(&list, path);

    // Print contents.
    options->toplevel = false;
    show_files(&list, options);
    if (options->recursive) {
        show_directories(&list, options, path);
    }
    list_delete(&list);
}

void rpm_cmd_dir(int argc, char *argv[])
{
    static const struct rpm_option long_opts[] = {
        { "help", RPM_NO_ARG, NULL, 'h' },
        {},
    };
    file_list_t list = {};
    options_t options = {};
    struct rpm_opt opt = {};
    unsigned argcount = 0;

    options.termwidth = 80;    // Default terminal width
    options.type = true;       // Append '/' for directories

    if (strcasecmp(argv[0], "dir") == 0) {
        // dir - long format by default.
        options.longform = true;
    } else {
        // ls - columnated by default.
        options.columnated = true;
    }

    while (rpm_getopt(argc, argv, "1lRahrt", long_opts, &opt) >= 0) {
        switch (opt.ret) {
        case 1:
            list_append_name(&list, opt.arg);
            argcount++;
            break;
        case '1':
            options.singlecol = true;
            options.columnated = false;
            options.longform = false;
            break;
        case 'l':
            options.longform = true;
            options.columnated = false;
            options.singlecol = false;
            break;
        case 'R':
            options.recursive = true;
            break;
        case 'a':
            options.showhidden = true;
            break;
        case 'r':
            options.reversesort = true;
            break;
        case 't':
            options.timesort = true;
            break;
        case '?':
            // Unknown option: message already printed.
            rpm_puts("\r\n");
            return;
        case 'h':
            rpm_puts("Usage:\r\n"
                     "    ls [options] [file ...]\r\n"
                     "    dir [options] [file ...]\r\n"
                     "Options:\r\n"
                     "    -a      Include hidden files and directories\r\n"
                     "    -l      List files in the long format\r\n"
                     "    -R      Recursively list subdirectories\r\n"
                     "    -r      Reverse the order of the sort\r\n"
                     "    -t      Sort by modification time\r\n"
                     "    -1      Force output to be one entry per line\r\n"
                     "\n");
            return;
        }
    }

    if (argcount == 0) {
        // Called without arguments - list current directory.
        list_append_name(&list, ".");
    } else if (list_is_empty(&list)) {
        // No valid arguments.
        return;
    }
    options.toplevel = true;
    show_files(&list, &options);
    show_directories(&list, &options, "");

    // Deallocate.
    list_delete(&list);

    if (options.any_output)
        rpm_puts("\r\n");
}
