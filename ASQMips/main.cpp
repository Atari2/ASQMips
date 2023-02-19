#include "Parser.h"
#include "Tokenizer.h"
#include <ArgParser.h>
#include <cstdio>

using namespace ARLib;

int main(int argc, char** argv) {
    bool debug = false;
    ArgParser argparse{argc, argv};
    argparse.add_version(1, 0);
    argparse.allow_unmatched(1);
    argparse.add_usage_string("./ASQMips.exe [options] <file>.clt");
    argparse.add_option("-d", "Enable debug output", debug);
    if (argparse.parse()) {
        if (argparse.help_requested()) {
            argparse.print_help();
            return 0;
        }
        const auto& unmatched = argparse.unmatched();
        if (unmatched.size() != 1) {
            argparse.print_help();
            return 1;
        }
        Tokenizer tok{unmatched[0]};
        if (auto res = tok.open(); res.is_error()) {
            Printer::print("Error opening file: {}", res.to_error());
            return 1;
        }
        if (auto res = tok.tokenize(); res.is_error()) {
            Printer::print("Error tokenizing file: {}", res.to_error());
            return 1;
        }
        Parser parser{tok};
        parser.parse();
        if (debug) {
            for (const auto& instruction : parser.instructions()) {
                // FIXME: replace with my own printf once the fix for the bug in arlib is merged
                char buf[256]{};
                int ret = std::snprintf(buf, sizeof(buf), "0x%04X: ", instruction.address());
                buf[ret] = '\0';
                Printer::print("{}{}", StringView{buf}, instruction);
            }
            if (!parser.dump_binary_data()) {
                Printer::print("Error dumping binary data because {}", last_error());
                return 1;
            }
        }
    }
}