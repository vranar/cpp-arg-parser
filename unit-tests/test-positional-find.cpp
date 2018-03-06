#include <iostream>
#include "arg_parser.hpp"

int main(int argc, char** argv)
{
    ArgumentParser args;

    args.register_positional(1, {"POSITIONAL_ARGUMENT"});

    args.load_arguments(argc, argv);

    bool ok = true;

    // try find by subscript
    if (args[0].empty()) {
        std::cerr << "Positional argument is not set." << std::endl;
        return EXIT_FAILURE;
    }


    try {
        // non-existent argument should throw std::out_of_range
        args[1];
        std::cerr << "Out of range exception not thrown for non-existent argument." << std::endl;
        ok = false;
    } catch (std::out_of_range&) {
    }


    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}