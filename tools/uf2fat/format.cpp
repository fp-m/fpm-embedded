#include <iostream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <fts.h>
#include <fpm/fs.h>
#include "extern.h"
#include "uf2.h"

static unsigned num_dirs, num_files;

//
// Create a directory.
//
static void add_directory(char *name)
{
    fs_result_t result = f_mkdir(name);
    if (result != FR_OK) {
        std::cerr << name << ": Cannot create directory\n";
        exit(EXIT_FAILURE);
    }
}

//
// Copy regular file to filesystem.
//
static void add_file(const char *path, const std::string &dirname)
{
    // Build local file path.
    std::string host_path;
    if (dirname.empty()) {
        // Use filename relative to current directory.
        host_path = std::string(path);
    } else {
        // Concatenate directory name and file name.
        host_path = dirname;
        if (host_path.back() != '/' && path[0] != '/') {
            host_path += "/";
        }
        host_path += std::string(path);
    }

    // Read file content.
    std::ifstream file(host_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << host_path << ": Cannot read file\n";
        exit(EXIT_FAILURE);
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                        (std::istreambuf_iterator<char>()));
    file.close();

    // Create file.
    auto fp = (file_t*) alloca(f_sizeof_file_t());
    auto result = f_open(fp, path, FA_WRITE | FA_CREATE_ALWAYS);
    if (result != FR_OK) {
        std::cerr << path << ": Cannot create file\n";
        exit(EXIT_FAILURE);
    }

    // Write data.
    unsigned nbytes = content.size();
    unsigned written = 0;
    f_write(fp, content.data(), nbytes, &written);
    if (nbytes != written) {
        std::cerr << path << ": Failed to write " << nbytes << " bytes\n";
        exit(EXIT_FAILURE);
    }

    // Close the file.
    f_close(fp);
}

//
// Compare two entries of file traverse scan.
//
#ifdef __FreeBSD__
static int ftsent_compare(const FTSENT *const *a, const FTSENT *const *b)
{
    return strcmp((*a)->fts_name, (*b)->fts_name);
}
#else
static int ftsent_compare(const FTSENT **a, const FTSENT **b)
{
    return strcmp((*a)->fts_name, (*b)->fts_name);
}
#endif

//
// Scan the directory and add each file to the target filesystem.
//
void add_contents(const std::string &dirname)
{
    // Open directory.
    char *argv[2] = { (char*)dirname.c_str(), NULL };
    FTS *dir = fts_open(argv, FTS_PHYSICAL | FTS_NOCHDIR, &ftsent_compare);
    if (!dir) {
        std::cerr << dirname << ": Cannot read directory\n";
        exit(EXIT_FAILURE);
    }
    auto prefix_len = dirname.length();

    for (;;) {
        // Read next directory entry.
        FTSENT *node = fts_read(dir);
        if (!node)
            break;

        char *path = node->fts_path + prefix_len;
        if (path[0] == 0)
            continue;

        switch (node->fts_info) {
        case FTS_D:
            // Directory.
            // std::cout << "--- add directory " << path << "\n";
            add_directory(path);
            num_dirs++;
            break;

        case FTS_F:
            // Regular file.
            // std::cout << "--- add file " << path << " from directory '" << dirname << "'\n";
            add_file(path, dirname);
            num_files++;
            break;

        default:
            // Ignore all other variants.
            break;
        }
    }
    fts_close(dir);
    std::cout << "Install " << num_dirs << " directories, " << num_files << " files\n";
}

//
// Scan one UF2 block.
//
static void scan_block(const UF2_Block &block, unsigned &flash_start,
                       unsigned &prog_end, unsigned &family_id)
{
    // Check header.
    if (block.magicStart0 != UF2_MAGIC_START0 ||
        block.magicStart1 != UF2_MAGIC_START1 ||
        block.magicEnd != UF2_MAGIC_END) {
        std::cerr << "Invalid UF2 image\n";
        exit(EXIT_FAILURE);
    }

    // Get start and end locations.
    auto start = block.targetAddr;
    auto end   = start + block.payloadSize;
    if (start < flash_start) {
        flash_start = start;
    }
    if (end > prog_end) {
        prog_end = end;
    }

    // Get family ID when present.
    if ((block.flags & UF2_FLAG_FAMILY_ID) && family_id == 0) {
        family_id = block.reserved;
    }
}

//
// Scan UF2 file.
// Get program start/end addresses and family ID.
//
static void get_location_and_family(const std::string &input_filename, unsigned &flash_start,
                                    unsigned &prog_end, unsigned &family_id)
{
    std::ifstream file(input_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << input_filename << ": Cannot not read file\n";
        exit(EXIT_FAILURE);
    }

    // Scan all blocks one by one.
    UF2_Block block{};
    flash_start = ~0u;
    prog_end = 0;
    family_id = 0;
    while (file.read((char*)&block, sizeof(block))) {
        if (block.flags & UF2_FLAG_NOFLASH) {
            continue;
        }
        scan_block(block, flash_start, prog_end, family_id);
        if (block.blockNo + 1 == block.numBlocks) {
            // The last block.
            break;
        }
    }

    std::cout << input_filename << ": Program start 0x" << std::hex << flash_start
              << ", size " << std::dec << (prog_end - flash_start) << " bytes";
    if (family_id != 0) {
        std::cout << ", family 0x" << std::hex << family_id << std::dec;
    }
    std::cout << "\n";
}

//
// Create filesystem of given size and fill it with contents.
//
static void build_filesystem(const std::string &contents_dir)
{
    SectorData buf;

    fs_image.clear();
    fs_result_t result = f_mkfs("0:", FM_FAT | FM_SFD, buf.data(), SECTOR_SIZE);
    if (result != FR_OK) {
        std::cerr << "Cannot create filesystem\n";
        exit(EXIT_FAILURE);
    }

    // Mount drive.
    result = f_mount("0:");
    if (result != FR_OK) {
        std::cerr << "Cannot mount filesystem\n";
        exit(EXIT_FAILURE);
    }

    // Set disk label.
    f_setlabel("FP/M");

    // Add contents from the specified directory.
    if (!contents_dir.empty()) {
        add_contents(contents_dir);
    }

    // Unmount.
    result = f_unmount("0:");
    if (result != FR_OK) {
        std::cerr << "Cannot unmount filesystem\n";
        exit(EXIT_FAILURE);
    }
}

//
// Write one sector to UF2 file.
//
static void write_sector(std::fstream &out, unsigned address, const SectorData &data, unsigned family_id)
{
    UF2_Block block{};
    block.magicStart0 = UF2_MAGIC_START0;
    block.magicStart1 = UF2_MAGIC_START1;
    block.magicEnd = UF2_MAGIC_END;
    block.targetAddr = address;
    block.payloadSize = 256;
    block.numBlocks = SECTOR_SIZE / block.payloadSize;

    if (family_id != 0) {
        block.flags |= UF2_FLAG_FAMILY_ID;
        block.reserved = family_id;
    }

    // Write a sequence of blocks.
    for (unsigned offset = 0; block.blockNo < block.numBlocks; block.blockNo++) {
        memcpy(&block.data[0], &data[offset], block.payloadSize);
        out.write((char*)&block, sizeof(block));
        offset += block.payloadSize;
        block.targetAddr += block.payloadSize;
    }
}

//
// Write UF2 image.
//
static void save_image(const std::string &input_filename, const std::string &output_filename,
                       const unsigned fs_start, const unsigned family_id)
{
    std::fstream out;
    if (output_filename.empty()) {
        // Add to input file.
        out.open(input_filename, std::ios::binary | std::ios::in | std::ios::out);

        // Skip existing file contents.
        UF2_Block block{};
        while (out.read((char*)&block, sizeof(block))) {
            if (block.flags & UF2_FLAG_NOFLASH) {
                continue;
            }
            if (block.blockNo + 1 == block.numBlocks) {
                // The last block.
                break;
            }
        }

        // Truncate to remove previous filesystem image, if any.
        std::filesystem::resize_file(input_filename, out.tellg());
    } else {
        // Create new file.
        out.open(output_filename, std::ios::binary | std::ios::out);

        // Copy input file to output.
        std::ifstream in(input_filename, std::ios::binary);
        UF2_Block block{};
        while (in.read((char*)&block, sizeof(block))) {
            out.write((char*)&block, sizeof(block));
            if (block.flags & UF2_FLAG_NOFLASH) {
                continue;
            }
            if (block.blockNo + 1 == block.numBlocks) {
                // The last block.
                break;
            }
        }
    }

    // Write filesystem image.
    for (const auto &pair : fs_image) {
        auto addr = fs_start + (pair.first * SECTOR_SIZE);
        write_sector(out, addr, pair.second, family_id);
    }
}

//
// Create filesystem with optional contents.
//
void format_filesystem(const std::string &input_filename, size_t flash_bytes,
                       const std::string &contents_dir, const std::string &output_filename)
{
    // Scan input file and obtain program start/end addresses and family ID.
    unsigned flash_start, prog_end, family_id;
    get_location_and_family(input_filename, flash_start, prog_end, family_id);

    // Check available space.
    unsigned fs_start = (prog_end + 0x10000) & ~0xffff; // align to 64 kbytes
    if (flash_start + flash_bytes < fs_start + 5 * SECTOR_SIZE) {
        // Need at least 5 blocks for FAT12.
        std::cerr << input_filename << ": Not enough space for filesystem\n";
        exit(EXIT_FAILURE);
    }

    // Create filesystem.
    fs_nbytes = flash_start + flash_bytes - fs_start;
    std::cout << "Create filesystem at 0x" << std::hex << fs_start
              << ", size " << std::dec << (fs_nbytes / 1024) << " kbytes\n";
    build_filesystem(contents_dir);

    // Save Flash image.
    save_image(input_filename, output_filename, fs_start, family_id);
}
