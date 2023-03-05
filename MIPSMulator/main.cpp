#include "DataParser.h"
#include "InstructionParser.h"
#include <ArgParser.h>
#include <Printer.h>

using namespace ARLib;
int main(int argc, char** argv) {
    String rodata_file;
    String code_file;
    ArgParser parser{argc, argv};
    parser.add_version(1, 0);
    parser.add_option("--rodata", "filename", "ROData file to read", rodata_file);
    parser.add_option("--code", "filename", "Code file to read", code_file);
    auto ec = parser.parse();
    if (ec.is_error()) {
        Printer::print("Error parsing arguments: {}", ec.to_error().error);
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
    BinaryData rodata{rodata_file};
    InstructionData code{code_file};
    for (auto& instruction : code.instructions) {
        printf("Instruction: %08x\n", instruction.opcode);
    }
    for (auto& data : rodata.data) {
        printf("Data: %016llx\n", data);
    }
    return EXIT_SUCCESS;
}