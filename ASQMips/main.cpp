#include "Parser.h"
#include "Tokenizer.h"
#include <ArgParser.h>
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

using namespace ARLib;

int main(int argc, char** argv) {
    bool dump_labels = false;
    bool dump_rodata = false;
    bool dump_tokens = false;
    bool dump_instructions = false;
    bool not_encode_instructions = false;
    ArgParser argparse{argc, argv};
    argparse.add_version(1, 0);
    argparse.allow_unmatched(1);
    argparse.add_usage_string("./ASQMips.exe [options] <file>.s");
    argparse.add_option("--labels", "Dump labels", dump_labels);
    argparse.add_option("--rodata", "Dump ro-data file", dump_rodata);
    argparse.add_option("--tokens", "Dump tokens", dump_tokens);
    argparse.add_option("--instructions", "Dump instructions", dump_instructions);
    argparse.add_option("--no-encode", "Do not encode instructions", not_encode_instructions);
    if (argparse.parse()) {
        if (argparse.help_requested()) {
            argparse.print_help();
            return EXIT_SUCCESS;
        }
        const auto& unmatched = argparse.unmatched();
        if (unmatched.size() != 1) {
            argparse.print_help();
            return EXIT_FAILURE;
        }
        Tokenizer tok{unmatched[0]};
        if (auto res = tok.open(); res.is_error()) {
            Printer::print("Error opening file: {}", res.to_error());
            return EXIT_FAILURE;
        }
        if (auto res = tok.tokenize(); res.is_error()) {
            Printer::print("Error tokenizing file: {}", res.to_error());
            return EXIT_FAILURE;
        }
        if (dump_tokens) { tok.dump_tokens(); }
        Parser parser{tok};
        parser.parse();
        if (dump_labels) { parser.dump_labels(); }
        if (dump_rodata) {
            if (!parser.dump_binary_data()) {
                Printer::print("Error dumping binary data because {}", last_error());
                return EXIT_FAILURE;
            }
        }
        if (dump_instructions) { parser.dump_instructions(); }
        if (!not_encode_instructions) { parser.encode_instructions(); }
        Printer::print("File {} finished assembling successfully", unmatched[0]);
    }
    return EXIT_SUCCESS;
}