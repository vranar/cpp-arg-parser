#include <iostream>
#include "arg_parser.hpp"

int main(int argc, char** argv)
{
    const auto useful = static_cast<std::string>("useful-option");
    const auto non_exist = static_cast<std::string>("non-existent");

    ArgumentParser args;

    args.register_option({"", useful}, ArgumentOption::REQUIRED, ArgumentType::BOOL, "");

    args.load_arguments(argc, argv);

    auto ok = false;


    // try find by subscript
    ok = !args[useful].empty() && args[non_exist].empty();
    if (!ok) {
        std::cerr << "subscript: Useful option is not set or non-existent option was magically created." << std::endl;
    }

    // try is_set
    ok = args.option_is_set(useful) && !args.option_is_set(non_exist);
    if (!ok) {
        std::cerr << "is_set: Useful option is not set or non-existent option was magically created." << std::endl;
    }

    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}