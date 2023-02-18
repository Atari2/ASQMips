#include "Parser.h"
#include "Tokenizer.h"
#include <ArgParser.h>

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
        if (debug) {
            for (const Token& t : tok.tokens()) {
                Printer::print("{}", t);
            }
        }
        Parser parser{tok};
        parser.parse();
    }
}