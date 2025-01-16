#include <string>
#include <map>
#include <array>
#include <cstdint>

//
// Sector size is 4096 bytes for Flash memory.
//
static const unsigned SECTOR_SIZE = 4096;

//
// Data buffer for one sector.
//
using SectorData = std::array<uint8_t, SECTOR_SIZE>;

//
// Size of filesystem in bytes.
//
extern unsigned fs_nbytes;

//
// Collection of sectors, ordered by sector number.
//
extern std::map<unsigned, SectorData> fs_image;

void dump_file(const std::string &input_filename);

//
// Create filesystem with optional contents.
//
void format_filesystem(const std::string &input_filename, size_t flash_bytes,
                       const std::string &contents_dir, const std::string &output_filename);
