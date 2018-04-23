# cpp-arg-parser
CPP Library for parsing CLI arguments with python-like interface.

This is a simple library designed to process CLI arguments. It provides interface
inspired by python argparse module so the workflow is something like this:

* Create `ArgumentParser` object with program description and optionally usage.
* Register your CLI arguments.
* Load CLI arguments to the object.
* Access the arguments like a map or retrive the in parsed form.

The ArgumentParser will process the arguments and store them for the duration of the program. It will also
check if some options cannot be used together if they belong to the same groups. The library will also report
if some options are missing.

# Basic usage example

```cpp
#include <iostream>
#include <cppargparser/arg_parser.hpp>

int main(int argc, char** argv)
{
	ArgumentParser args("ArgumentParser Demo");

	// lets register a simple argument
	args.register_option({"o", "option"}, ArgumentOption::REQUIRED, ArgumentType::BOOL, "I'm an option.");

	// error if option is not set
	try {
		args.load_arguments(argc, argv);
	} catch (std::runtime_error& ex) {
		std::cerr << "Invalid command-line arguments" << std::endl;
		std::cerr << ex.what() << std::endl;
		args.print_help_text();
		return 1;
	}

	if (args.option_is_set("option")) {
		std::cout << "Option 'o' is set.\n";
	}

	return 0;
}
```

# Options

The library allows processing of both options and positional arguments. Option are registered as shown
in an example and the method has this prototype:

```cpp
	bool register_option(const arg_key& ak,
                         ArgumentOption opt,
                         ArgumentType type,
                         const std::string& desc,
                         const std::string& excl_group = "",
                         const arg_default& default_value = arg_default());
```

The first argument specifies a key under which the option is stored. This is used to retrive the value. The key object
contains both `short key` and `long key`. You can leave one of the keys empty if you do not want to use it.

The second argument specifies, whether the option is required or not. Allowed values are specified in an `enum class ArgumentOption`
and are as follows:
```cpp
    ArgumentOption::REQUIRED      // Option must be specified
    ArgumentOption::OPTIONAL      // Option can be left out
    ArgumentOption::INHERIT_GROUP // Option will follow the rules defined by the group it belongs to
```

The third argument specifies type of the option. Following types are allowed:
```cpp
    ArgumentType::BOOL  // Boolean True or False (Only is_set flag is used)
    ArgumentType::INT   // Integral number
    ArgumentType::HEX   // Integral number writen in hex
    ArgumentType::FLOAT // Floating point number
    ArgumentType::STR   // String
```

The fourth argument is used for description of the option. This description is shown in help text.

The fifth argument specifies a group if the option is supposed to be mutually exclusive with some other option.

The last option is used to provide default value. This is done using an `arg_default` object.

Fully specified option will look something like this:
```cpp
	args.register_option({"o", "option"}, ArgumentOption::REQUIRED, ArgumentType::INT, "I'm an option.", "group", arg_default("1"));
```

# Positional arguments

Positional arguments are specified in a single method and only the number has to be provided. Optinally you can provide
an braced `initializer_list` with names that will be used in help text.

```cpp
	// prototype
	bool register_positional(unsigned count, std::vector<std::string> names=std::vector<std::string>());

	// lets register some positinals
	args.register_positional(2, {"HELLO", "WORLD"});
```

# Accessing arguments
To access any of the options the class `ArgumentParser` provides overload to `[]` operator. The same operator can also
be used to access positional argument if tou provide and integral index.

```cpp
	auto x = args["option"]; // returns option 'option' as std::string
	auto y = args[0];        // returns first positional as std::string
```

The library also proivdes methods to convert the arguments to other types if needed. For this you can use
`parse_option` and `parse_positional` methods. The methods use strignstream to convert the options so 
defining stream input and output operators should be sufficient for other types.

```cpp
	auto x = args.parse_option<int>("option"); // returns option 'option' as an int
	auto y = args.parse_positional<double>(0); // returns first positional as double
```

# Groups and mutual exclusion
When registering options, you create groups for options that should be mutually exclusive. This is done using method
`add_mutually_exclusive_group`. Then you can either add the option through `insert_into_group` method or you can specify
the group when registering the option. Options that belong to the same group then cannot be used together.

The requirement status of the group **overrides** the option specification so if you add optional option to required group,
then the option becomes required as well.

```cpp
	// let's add group
	args.add_mutually_exclusive_group("group", true);

	args.register_option({"o", "option"}, ArgumentOption::INHERIT_GROUP, ArgumentType::BOOL, "I'm an option.", "group");
	args.register_option({"m", "m-option"}, ArgumentOption::INHERIT_GROUP, ArgumentType::BOOL, "I'm an another option.", "group");

	// exception will be thrown when arguments are loaded and both options are present.
```