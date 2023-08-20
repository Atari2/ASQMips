#include "CPU.h"
#include "DataParser.h"
#include "InstructionParser.h"
#include <ArgParser.hpp>
#include <Printer.hpp>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

using namespace ARLib;
int main(int argc, char** argv) {
    bool print_instructions = false;
    String rodata_file;
    String code_file;
    ArgParser parser{argc, argv};
    parser.add_version(1, 0);
    parser.add_option("--rodata", "filename", "ROData file to read", rodata_file);
    parser.add_option("--code", "filename", "Code file to read", code_file);
    parser.add_option("--insn", "Print the instructions as they're being executed", print_instructions);
    auto ec = parser.parse();
    if (ec.is_error()) {
        Printer::print("Error parsing arguments: {}", ec.to_error().error_string());
        return EXIT_FAILURE;
    }
    if (parser.help_requested()) {
        parser.print_help();
        return EXIT_SUCCESS;
    }
    if (rodata_file.is_empty()) {
        Printer::print("No rodata file specified");
        return EXIT_FAILURE;
    }
    if (code_file.is_empty()) {
        Printer::print("No code file specified");
        return EXIT_FAILURE;
    }
    CPU cpu{};
    if (auto m_err = cpu.initialize(code_file, rodata_file); m_err.is_error()) {
        Printer::print("Error initializing CPU: {}", m_err.to_error());
        return EXIT_FAILURE;
    };
    cpu.run(print_instructions);
    return EXIT_SUCCESS;
}