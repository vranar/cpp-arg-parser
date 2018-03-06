#include <iostream>
#include <bitset>
#include "arg_parser.hpp"

class ExpectedValues final {
private:
    int int_val_;
    int hex_val_;
    std::string str_val_;
    float flt_val_;
    double dbl_val_;
public:
    ExpectedValues()
            : int_val_(1),
              hex_val_(0xFF),
              str_val_("Hello"),
              flt_val_(0.1f),
              dbl_val_(static_cast<double>(flt_val_))
    {}

    decltype(int_val_) int_val() { return int_val_; }
    decltype(hex_val_) hex_val() { return hex_val_; }
    decltype(str_val_) str_val() { return str_val_; }
    decltype(flt_val_) flt_val() { return flt_val_; }
    decltype(dbl_val_) dbl_val() { return dbl_val_; }
};

int main(int argc, char** argv)
{
    ExpectedValues expect;

	ArgumentParser args;

	args.register_option({"", "int"}, ArgumentOption::REQUIRED, ArgumentType::INT, "");
	args.register_option({"", "hex"}, ArgumentOption::REQUIRED, ArgumentType::HEX, "");
	args.register_option({"", "string"}, ArgumentOption::REQUIRED, ArgumentType::STR, "");
	args.register_option({"", "float"}, ArgumentOption::REQUIRED, ArgumentType::FLT, "");

	args.load_arguments(argc, argv);

    if (args.option_is_set("help")) {
        std::cout << "This test should not be run by hand." << std::endl;
        return EXIT_SUCCESS;
    }

    std::bitset<5> result;

    result[0] = args.parse_option<decltype(expect.int_val())>("int") ==  expect.int_val();
    result[1] = args.parse_option<decltype(expect.hex_val())>("hex") ==  expect.hex_val();
    result[2] = args.parse_option<decltype(expect.str_val())>("string") == expect.str_val();
    result[3] = [&args, &expect]()
    {
        return std::abs(args.parse_option<decltype(expect.flt_val())>("float") - expect.flt_val()) < 0.0001;
    }();
    result[4] = [&args, &expect]()
    {
        return std::abs(args.parse_option<decltype(expect.dbl_val())>("float") - expect.dbl_val()) < 0.0001;
    }();


    std::cout << std::boolalpha;
	std::cout << args.parse_option<decltype(expect.int_val())>("int") << " == " << expect.int_val()  << " : " << result[0] << std::endl;
    std::cout << args.parse_option<decltype(expect.hex_val())>("hex") << " == " << expect.hex_val() << " : " << result[1] << std::endl;
    std::cout << args.parse_option<decltype(expect.str_val())>("string") << " == " << expect.str_val() << " : " << result[2] << std::endl;
    std::cout << args.parse_option<decltype(expect.flt_val())>("float")  << " == " << expect.flt_val() << " : " << result[3] << std::endl;
    std::cout << args.parse_option<decltype(expect.dbl_val())>("float") << " == " << expect.dbl_val() << " : " << result[4] << std::endl;

	return result.all() ? EXIT_SUCCESS : EXIT_FAILURE;
}
