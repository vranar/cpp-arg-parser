//
// Created by ivrana on 02/03/18.
//

#include <iostream>
#include "arg_parser.hpp"

int main(int argc, char** argv)
{
    ArgumentParser args("Unit test for mutually exclusive groups.");

    args.add_mutually_exclusive_group("mtx", true);
    args.add_mutually_exclusive_group("mtx2", false);

    args.register_option({"a",""}, ArgumentOption::INHERIT_GROUP, ArgumentType::BOOL, "", "mtx");
    args.register_option({"b",""}, ArgumentOption::INHERIT_GROUP, ArgumentType::BOOL, "", "mtx2");
    args.register_option({"c",""}, ArgumentOption::INHERIT_GROUP, ArgumentType::BOOL, "", "mtx2");

    try {
        args.load_arguments(argc, argv);
    } catch (std::logic_error& ex) {
        std::cerr << ex.what() << std::endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}