#include <iostream>
#include <iomanip>
#include <fstream>
#include "extern.h"
#include "uf2.h"

//
// Show block contents.
//
static void print_block(const UF2_Block &block, const size_t offset)
{
    // 32 byte header
    if (block.magicStart0 != UF2_MAGIC_START0) {
        std::cerr << "Bad magicStart0=" << std::hex << block.magicStart0
                  << std::dec << " at offset " << offset << "\n";
        exit(EXIT_FAILURE);
    }
    if (block.magicStart1 != UF2_MAGIC_START1) {
        std::cerr << "Bad magicStart1=" << std::hex << block.magicStart1
                  << std::dec << " at offset " << offset << "\n";
        exit(EXIT_FAILURE);
    }
    if (block.magicEnd != UF2_MAGIC_END) {
        std::cerr << "Bad magicEnd=" << std::hex << block.magicEnd
                  << std::dec << " at offset " << offset << "\n";
        exit(EXIT_FAILURE);
    }

    std::cout << std::hex << std::setw(5) << block.flags
              << "  " << std::setw(8) << block.targetAddr
              << "    " << std::dec << std::setw(4) << block.payloadSize
              << "    " << std::setw(6) << block.blockNo
              << "    " << std::setw(6) << block.numBlocks
              << "    " << std::hex << std::setw(8) << block.reserved;

    std::cout << std::hex << " " << (unsigned)(uint8_t)block.data[0]
              << "-" << (unsigned)(uint8_t)block.data[1]
              << "-" << (unsigned)(uint8_t)block.data[2]
              << "-...-" << (unsigned)(uint8_t)block.data[474]
              << "-" << (unsigned)(uint8_t)block.data[475]
              << "\n";
}

//
// Show contents of UF2 file.
//
void dump_file(const std::string &input_filename)
{
    std::ifstream file(input_filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << input_filename << ": Cannot not open file\n";
        exit(EXIT_FAILURE);
    }

    UF2_Block block{};
    auto const block_size = sizeof(block);
    if (block_size != 512) {
        std::cerr << "Bad UF2 block size: " << block_size << "\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Flags TargetAddr PayloadSize BlockNo NumBlocks FamilyID Data\n";
    size_t offset{};
    while (file.read((char*)&block, block_size)) {
        print_block(block, offset);
        offset += block_size;
    }
}
