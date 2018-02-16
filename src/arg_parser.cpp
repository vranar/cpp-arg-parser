/**
 * @file arg_parser.cpp
 * @brief Python-like CLI arguments parser.
 * @author Ing. Roman Vrana, ivrana@fit.vutbr.cz
 * @date 2016-09-07
 */


#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "arg_parser.hpp"

ArgumentParser::ArgumentParser(const std::string& desc, const std::string& usage)
	: OPT_WIDTH(25), exec_name(), options(), mandatory(), prog_desc(desc), usage(usage), req_positional()
{
	// automatically register help option
	this->register_option({"h", "help"}, false, ArgumentParser::ARG_BOOL, "Show help text and exit");
}

bool ArgumentParser::register_option(const arg_key& ak,
                                     bool required,
                                     int type,
                                     std::string desc,
                                     const arg_default& default_value)
{
	if (ak.empty())
		return false;

	arg_key opt_key(ak);

	auto ret = this->options.emplace(std::make_pair(opt_key, arg_opt(default_value.second, type, desc, default_value.first)));

	if (!ret.second)
		return false;

	if (required) {
		auto&& opt = std::find_if(this->mandatory.begin(),
		                          this->mandatory.end(),
		                          [opt_key](arg_key& k) { return opt_key == k; });

		if (opt == this->mandatory.end())
			this->mandatory.emplace_back(opt_key);
	}

	return true;
}

void ArgumentParser::register_positional(unsigned int count, std::vector<std::string> names)
{
	for (unsigned int i = 0; i < count; i++) {
		if (i < names.size()) {
			this->positional.emplace_back("", names[i]);
		} else {
			this->positional.emplace_back("", "ARG_" + std::to_string(i + 1));
		}
	}

	this->req_positional = count;
}


void ArgumentParser::load_arguments(int argc, char **argv)
{
	std::vector<std::string> args;
	size_t pos = 0;

	//store executable name
	std::string exe(argv[0]);
#if defined(_WIN32) || defined(WIN32)
	pos = exe.find_last_of('\\');
#else
	pos = exe.find_last_of('/');
#endif

	if (pos != std::string::npos) {
		exe = exe.substr(pos + 1);
	}

	pos = 0;

	this->exec_name = exe;


	for (int i = 1; i < argc; i++) {
		args.emplace_back(std::string(argv[i]));
	}

	auto&& opt = this->options.end();

	for (auto&& A : args) {
		if ((A[0] == '-' || A.substr(0, 2) == "--") && !pos) {
			auto opt_key = A.substr(A.find_first_not_of('-'));
			opt = std::find_if(this->options.begin(),
			                   this->options.end(),
			                   [opt_key](const std::pair<arg_key, arg_opt>& a)
			                   { return a.first.shr == opt_key || a.first.lng == opt_key; });

			if (opt != this->options.end())
				opt->second.is_set = true;

			if (opt->second.type == ARG_BOOL)
				opt = this->options.end();
		} else if (!(A[0] == '-' || A.substr(0, 2) == "--")) {
			if (opt != this->options.end()) {
				opt->second.value = A;
				opt = this->options.end();
			} else {
				if (this->positional.capacity()) {
					this->positional[pos++].value = A;
				}
			}
		} else {
			throw std::logic_error("Positional arguments cannot precede options.");
		}
	}

	// check for help and return if specified
	if (!this->parse_option<bool>("help")) {
		std::string missing_args;
		for (auto &&M : this->mandatory) {
			if (!this->options.find(M)->second.is_set)
				missing_args.append((M.shr.empty() ? "-" : "-" + M.shr)
				                    + "/"
				                    + (M.lng.empty() ? "-" : "--" + M.lng)
				                    + "\n");
		}

		if (missing_args.length() != 0) {
			missing_args.erase(missing_args.end() - 1);
			throw std::logic_error("Missing mandatory options:\n" + missing_args);
		}

		if (pos < (static_cast<size_t>(this->req_positional)))
			throw std::logic_error("Missing required positional arguments. Check program usage.");
	}
}

void ArgumentParser::print_help_text()
{
	std::string req;
	std::string opt;

	std::cout << "Usage: ";

	std::cout << this->exec_name << " ";

	if (this->usage != "") {
		std::cout << this->usage << std::endl;
	} else {
		for (auto&& O : this->options) {
			std::string arg("");

			if (O.first.shr == "h" || O.first.lng == "help")
				continue;

			if (O.first.shr != "" && O.first.lng != "")
				arg += "-" + O.first.shr + " | " + "--" + O.first.lng;
			else if (O.first.shr != "")
				arg += "-" + O.first.shr;
			else if (O.first.lng != "")
				arg += "--" + O.first.lng;

			switch (O.second.type) {
				case ArgumentParser::ARG_INT:
					arg += " <INT>";
					break;
				case ArgumentParser::ARG_FLT:
					arg += " <FLOAT>";
					break;
				case ArgumentParser::ARG_STR:
					arg += " <STRING>";
					break;
				default:
					break;
			}

			if (std::find(this->mandatory.begin(),
			              this->mandatory.end(),
			              O.first)
			    != this->mandatory.end())
			{
				req += arg + " ";
			}
			else
			{
				opt += "[ " + arg +" ] ";
			}


		}

		std::cout << req << opt;

		for (auto P : this->positional)
			std::cout << P.name << " ";

		std::cout << '\b' << std::endl;
	}


	std::cout << this->prog_desc << std::endl << std::endl;

	std::cout << "Available options:" << std::endl;

	for (auto&& O : this->options) {
        size_t pos;
		std::string opt;
		if (O.first.shr != "" && O.first.lng != "")
			opt = "-" + O.first.shr + ", " + "--" + O.first.lng;
		else if (O.first.shr != "")
			opt = "-" + O.first.shr;
		else if (O.first.lng != "")
			opt = "--" + O.first.lng;

		size_t next;
		std::cout << std::left << std::setw(OPT_WIDTH) << opt;
		pos = O.second.desc.find_first_of('\n');
		std::cout << O.second.desc.substr(0, pos);
		next = pos + 1;

		while (pos != std::string::npos) {
			std::cout << std::endl;
			pos = O.second.desc.find_first_of('\n', next);
			std::cout << std::left << std::setw(OPT_WIDTH) << " ";
			std::cout << O.second.desc.substr(next, pos) << std::endl;
			next = pos + 1;
		}
		std::cout << std::endl;

		if (O.second.has_default) {
			std::cout << std::left << std::setw(OPT_WIDTH) << " ";
			std::cout << "Default value: " << O.second.value << std::endl;
		}

		std::cout << std::endl;
	}
}

