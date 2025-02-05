#include <iostream>
#include <string>
#include <cstring>
#include <getopt.h>
#include "extern.h"

//
// CLI options.
//
static const struct option long_options[] = {
    // clang-format off
    { "help",           no_argument,        nullptr,    'h' },
    { "version",        no_argument,        nullptr,    'V' },
    { "output",         required_argument,  nullptr,    'o' },
    {},
    // clang-format on
};

//
// Print usage message.
//
static void print_usage(std::ostream &out, const char *prog_name)
{
    out << "UF2 FAT Filesystem Tool, Version 0.1\n";
    out << "Usage:\n";
    out << "    " << prog_name << " [options...] format input.uf2 size [contents]\n";
    out << "    " << prog_name << " [options...] dump input.uf2\n";
    out << "Commands:\n";
    out << "    format                  Create filesystem with optional contents\n";
    out << "    dump                    Show contents of UF2 file\n";
    out << "Arguments:\n";
    out << "    input.uf2               Input file in UF2 format\n";
    out << "    size                    Target size in bytes, with optional suffix k/M\n";
    out << "    contents                Optional directory with filesystem contents\n";
    out << "Options:\n";
    out << "    -o FILE, --output=FILE  Write output to given file instead of input.uf2\n";
    out << "    -h, --help              Display this help and exit\n";
    out << "    -V, --version           Show version information and exit\n";
}

//
// Parse number with optional suffix k/M.
//
static size_t parse_size(std::string str)
{
    std::size_t pos{};
    auto num = std::stoul(str, &pos, 0);
    if (pos < str.size()) {
        // Scale by suffix.
        auto ch = str[pos];
        if (ch == 'k' || ch == 'K') {
            num *= 1024;
        } else if (ch == 'm' || ch == 'M') {
            num *= 1024 * 1024;
        } else {
            std::cerr << "Bad size suffix: " << str << "\n";
            exit(EXIT_FAILURE);
        }
    }
    return num;
}

//
// Main routine of the simulator,
// when invoked from a command line.
//
int main(int argc, char *argv[])
{
    // Get the program name.
    const char *prog_name = strrchr(argv[0], '/');
    if (prog_name == nullptr) {
        prog_name = argv[0];
    } else {
        prog_name++;
    }

    // Parse command line options.
    std::string command, input_filename, output_filename, contents_dir;
    size_t flash_bytes = 0;

    for (;;) {
        switch (getopt_long(argc, argv, "-hvo:", long_options, nullptr)) {
        case EOF:
            break;

        case 0:
            continue;

        case 1:
            // Regular argument.
            if (command.empty()) {
                command = optarg;
            } else if (input_filename.empty()) {
                input_filename = optarg;
            } else if (flash_bytes == 0) {
                flash_bytes = parse_size(optarg);
            } else if (contents_dir.empty()) {
                contents_dir = optarg;
            } else {
                print_usage(std::cout, prog_name);
                exit(EXIT_FAILURE);
            }
            continue;

        case 'h':
            // Show usage message and exit.
            print_usage(std::cout, prog_name);
            exit(EXIT_SUCCESS);

        case 'V':
            // Show version and exit.
            std::cout << "Version 0.1\n";
            exit(EXIT_SUCCESS);

        case 'o':
            // Output file name.
            output_filename = optarg;
            continue;

        default:
fail:       print_usage(std::cerr, prog_name);
            exit(EXIT_FAILURE);
        }
        break;
    }

    // Must specify a command.
    if (command.empty() || input_filename.empty()) {
        goto fail;
    }

    if (command == "format") {
        if (flash_bytes == 0) {
            goto fail;
        }
        format_filesystem(input_filename, flash_bytes, contents_dir, output_filename);
    } else if (command == "dump") {
        dump_file(input_filename);
    } else {
        goto fail;
    }

    return EXIT_SUCCESS;
}
