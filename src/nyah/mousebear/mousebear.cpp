#include <mousebear/grammar.hpp>
#include <mousebear/source_builder.hpp>

#include <nyah/options.hpp>

#include <chilon/print.hpp>

#include <iostream>

// #define MOUSEBEAR_VERSION "1000 (just don't get in a car, and stay away from my family)"
// #define MOUSEBEAR_VERSION "999 (she's also known as darwinius)"
// #define MOUSEBEAR_VERSION "998 (grind machine)"
// #define MOUSEBEAR_VERSION "997 (420 mishap)"
// #define MOUSEBEAR_VERSION "996 (super mousebear 4)"
// #define MOUSEBEAR_VERSION "995 (ibuki mousebear)"
// #define MOUSEBEAR_VERSION "994 (maryam the butcher)"
#define MOUSEBEAR_VERSION "993 (friendly beard)"

namespace nyah { namespace mousebear {

inline int main(int argc, char *argv[]) {
    options opts;

    char const header[] = "mousebear "   MOUSEBEAR_VERSION "\nusage: mousebear [arguments] <grammar files to process>";
    int nPositionals = opts.parse_command_line(header, argc, argv);

    if (0 == nPositionals) return 1;

    for (int i = 1; i <= nPositionals; ++i) {
        opts.verbose("parsing grammar ", argv[i]);
        nyah_stream stream;
        if (! stream.load(argv[i])) {
            std::cerr << "could not load file " << argv[i] << " exiting\n";
            return 1;
        }

        stream.skip_whitespace();

        store<Grammar> storer;

        if (storer(stream)) {
            stream.skip_whitespace();
            if (! stream.empty()) {
                std::cerr << argv[i] << ": partial match\n";
                continue;
            }
            else {
                opts.verbose(argv[i], ": parsed grammar");
            }
        }
        else {
            std::cerr << argv[i] << ": invalid grammar\n";
            continue;
        }

        if (opts.print_ast_) {
            chilon::print(argv[i], " grammar: ", storer.value_.value_);
        }

        source_builder build_source(opts);
        build_source(argv[i], storer.value_);
    }

    return 0;
}

} }

int main(int argc, char *argv[]) {
    return nyah::mousebear::main(argc, argv);
}
