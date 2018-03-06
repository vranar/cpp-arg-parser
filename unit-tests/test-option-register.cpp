#include <iostream>
#include "arg_parser.hpp"

int main(int argc, char** argv)
{
    ArgumentParser args;

    const arg_key ak{"o", "some-option"};

    args.register_option(ak, ArgumentOption::OPTIONAL, ArgumentType::BOOL, "");
    args.register_option({"s", ""}, ArgumentOption::OPTIONAL, ArgumentType::BOOL, "");
    args.register_option({"", "long"}, ArgumentOption::OPTIONAL, ArgumentType::BOOL, "");

    args.load_arguments(argc, argv);

    bool ok(true);

    // both keys are set --> both can be used
    if (!args.has_option(ak.shr)) {
        ok = false;
        std::cerr << "Option cannot be found by its short key '" << ak.shr << "'." << std::endl;
    }
    if (!args.has_option(ak.lng)) {
        ok = false;
        std::cerr << "Option cannot be found by its long key '" << ak.lng << "'." << std::endl;
    }

    if (!args.has_option("s")) {
        ok = false;
        std::cerr << "Option cannot be found by its short key 's'." << std::endl;
    }
    if (args.has_option("short")) {
        ok = false;
        std::cerr << "Option with empty long key can be found." << std::endl;
    }

    if (args.has_option("l")) {
        ok = false;
        std::cerr << "Option with empty short key can be found." << std::endl;
    }
    if (!args.has_option("long")) {
        ok = false;
        std::cerr << "Option cannot be found by long key 'long'." << std::endl;
    }


    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}