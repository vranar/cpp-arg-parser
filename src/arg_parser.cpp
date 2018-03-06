/**
 * @file arg_parser.cpp
 * @brief Python-like CLI arguments parser.
 * @author Ing. Roman Vrana, ivrana@fit.vutbr.cz
 * @date 2016-09-07
 */


#include <algorithm>
#include <functional>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "arg_parser.hpp"

ArgumentParser::ArgumentParser(const std::string& desc, const std::string& usage)
	: OPT_WIDTH_(25),
      exec_name_(),
      options_(),
      mandatory_(),
      prog_desc_(desc),
      usage_(usage)
{
	// automatically register help option
	this->register_option({"h", "help"}, ArgumentOption::OPTIONAL, ArgumentType::BOOL, "Show help text and exit");
}

bool ArgumentParser::register_option(const arg_key& ak,
                                     ArgumentOption opt,
                                     ArgumentType type,
                                     const std::string& desc,
									 const std::string& excl_group,
                                     const arg_default& default_value)
{
    // empty key is not valid
	if (ak.empty()) {
        return false;
    }

    // cannot inherit property from no group
    if (opt == ArgumentOption::INHERIT_GROUP && excl_group.empty()) {
        return false;
    }

    // store option
	auto ret = options_.emplace(ak, arg_opt(default_value.second, type, desc, default_value.first));

    // option was not added
	if (!ret.second) {
		return false;
	}

    // mark option as mandatory if explicitly stated
	if (opt == ArgumentOption::REQUIRED) {
		make_option_mandatory_(ak);
	}

    // store option into group
	if (!excl_group.empty() && (mtx_groups_.find(excl_group) != mtx_groups_.end())) {

        auto&& grp = mtx_groups_.at(excl_group);

        // mandatory option turns the group mandatory
        if (opt == ArgumentOption::REQUIRED) {
            grp.make_mandatory();
        }

        // make option mandatory if group is mandatory
        if (grp.mandatory() || (opt == ArgumentOption::INHERIT_GROUP && grp.mandatory())) {
            make_option_mandatory_(ak);
        }

		return insert_into_group(excl_group, ak);
	} else if (!excl_group.empty() && (mtx_groups_.find(excl_group) == mtx_groups_.end())) {
        // cannot add option to non-existent group
        return false;
    }

    // option is registered
	return true;
}

void ArgumentParser::register_positional(unsigned int count, std::vector<std::string> names)
{
	for (auto i = 0u; i < count; i++) {
		if (i < names.size()) {
			positional_.emplace_back("", names[i]);
		} else {
			positional_.emplace_back("", "ARG_" + std::to_string(i + 1));
		}
	}
}

decltype(auto) ArgumentParser::check_mandatory_options_() {
    std::vector<std::reference_wrapper<const arg_key>> missing;


    for (auto& M : mandatory_) {
        if (!option_is_mutually_exclusive_(M) && !options_.at(M).is_set) {
            missing.emplace_back(M);
        }
    }

    return missing;
}

decltype(auto) ArgumentParser::check_mandatory_option_groups_() {
    std::vector<std::reference_wrapper<const std::string>> missing;

    for (auto& G : mtx_groups_) {
        if (G.second.mandatory()) {
            if (std::all_of(G.second.begin(), G.second.end(),
                            [this](auto &opt) -> bool { return !options_.at(opt).is_set; })) {
                missing.emplace_back(G.first);
            }
        }
    }

    return missing;
}

decltype(auto) ArgumentParser::check_option_conflicts_() {
    std::vector<std::reference_wrapper<const std::string>> conflicts;

    for (auto& G : mtx_groups_) {
        if (std::count_if(G.second.begin(), G.second.end(), [this](auto& ak) {return options_.at(ak).is_set; }) > 1) {
            conflicts.emplace_back(G.first);
        }
    }

    return conflicts;
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

	exec_name_ = exe;


	for (auto i = 1; i < argc; i++) {
		args.emplace_back(std::string(argv[i]));
	}

	auto&& opt = options_.end();

	for (auto&& A : args) {
		if ((A[0] == '-' || A.substr(0, 2) == "--") && !pos) {
			auto opt_key = A.substr(A.find_first_not_of('-'));
			opt = std::find_if(options_.begin(),
			                   options_.end(),
			                   [opt_key](const auto& a)
			                   { return a.first.shr == opt_key || a.first.lng == opt_key; });

			if (opt != options_.end())
				opt->second.is_set = true;

			if (opt->second.type == ArgumentType::BOOL)
				opt = options_.end();
		} else if (!(A[0] == '-' || A.substr(0, 2) == "--")) {
			if (opt != options_.end()) {
				opt->second.value = A;
				opt = options_.end();
			} else {
				if (positional_.capacity()) {
					positional_[pos++].value = A;
				}
			}
		} else {
			throw std::logic_error("Positional arguments cannot precede options.");
		}
	}

	// check for help and return if specified
	if (!option_is_set("help")) {
        // check for missing mandatory options (exclude mtx)
		auto missing_args = check_mandatory_options_();
        auto missing_grp = check_mandatory_option_groups_();
        auto conflicting_opts = check_option_conflicts_();

        std::string err_str;

		if (!missing_args.empty()) {
            std::string req_options("Missing required options:\n");
            for (auto &&M : missing_args) {
                req_options.append((M.get().shr.empty() ? "-" : "-" + M.get().shr)
                                   + "/"
                                   + (M.get().lng.empty() ? "-" : "--" + M.get().lng)
                                   + "\n");
            }
            err_str += req_options;
        }

        if (!missing_grp.empty()) {
            std::string req_groups("At least one option from these groups must be set:\n");
            for (auto&& G : missing_grp) {
                req_groups.append(G.get() + "\n");
                for (auto&& O : mtx_groups_.at(G)) {
                    req_groups.append("\t"
                                      + (O.shr.empty() ? "-" : "-" + O.shr)
                                      + "/"
                                      + (O.lng.empty() ? "-" : "--" + O.lng)
                                      + "\n");
                }
            }
            err_str += req_groups;
		}
        if (!err_str.empty()) {
            throw std::logic_error(err_str);
        }

        if (!conflicting_opts.empty()) {
            std::string X_groups("Conflicting options used in these groups:\n");
            for (auto&& G : conflicting_opts) {
                X_groups.append(G.get() + "\n");
                for (auto&& O : mtx_groups_.at(G)) {
                    if (option_is_set(O)) {
                        X_groups.append("\t"
                                        + (O.shr.empty() ? "-" : "-" + O.shr)
                                        + "/"
                                        + (O.lng.empty() ? "-" : "--" + O.lng)
                                        + "\n");
                    }
                }
            }
            throw std::logic_error(X_groups);
        }

		if (pos < (static_cast<size_t>(positional_.size()))) {
			throw std::logic_error("Missing positional arguments. Check program usage ");
		}
	}
}

void ArgumentParser::print_usage_text()
{
    auto req = static_cast<std::string>("");
    auto opt = static_cast<std::string>("");

    std::cout << "Usage: ";

    std::cout << exec_name_ << " ";

    if (!usage_.empty()) {
        std::cout << usage_ << std::endl;
    } else {
        for (auto&& O : options_) {
            auto arg = static_cast<std::string>("");

            if (O.first.shr == "h" || O.first.lng == "help") {
                //
                continue;
            }

            if (O.first.shr != "" && O.first.lng != "")
                arg += "-" + O.first.shr + " | " + "--" + O.first.lng;
            else if (O.first.shr != "")
                arg += "-" + O.first.shr;
            else if (O.first.lng != "")
                arg += "--" + O.first.lng;

            switch (O.second.type) {
                case ArgumentType::HEX:
                    arg += " [0x]<HEX>";
                    break;
                case ArgumentType::INT:
                    arg += " <INT>";
                    break;
                case ArgumentType::FLT:
                    arg += " <FLOAT>";
                    break;
                case ArgumentType::STR:
                    arg += " <STRING>";
                    break;
                default:
                    break;
            }

            if (std::find(this->mandatory_.begin(),
                          this->mandatory_.end(),
                          O.first)
                != this->mandatory_.end())
            {
                req += arg + " ";
            }
            else
            {
                opt += "[ " + arg +" ] ";
            }


        }

        std::cout << req << opt;

        for (auto P : this->positional_) {
            std::cout << P.name << " ";
        }

        std::cout << '\b' << std::endl;
    }
}

void ArgumentParser::print_help_text()
{
    print_usage_text();

	std::cout << prog_desc_ << std::endl << std::endl;

	std::cout << "Available options_:" << std::endl;

	for (auto&& O : options_) {
        size_t pos;
		std::string opt;
		if (O.first.shr != "" && O.first.lng != "")
			opt = "-" + O.first.shr + ", " + "--" + O.first.lng;
		else if (O.first.shr != "")
			opt = "-" + O.first.shr;
		else if (O.first.lng != "")
			opt = "--" + O.first.lng;

		size_t next;
		std::cout << std::left << std::setw(OPT_WIDTH_) << opt;
		pos = O.second.desc.find_first_of('\n');
		std::cout << O.second.desc.substr(0, pos);
		next = pos + 1;

		while (pos != std::string::npos) {
			std::cout << std::endl;
			pos = O.second.desc.find_first_of('\n', next);
			std::cout << std::left << std::setw(OPT_WIDTH_) << " ";
			std::cout << O.second.desc.substr(next, pos) << std::endl;
			next = pos + 1;
		}
		std::cout << std::endl;

		if (O.second.has_default) {
			std::cout << std::left << std::setw(OPT_WIDTH_) << " ";
			std::cout << "Default value: " << O.second.value << std::endl;
		}

		std::cout << std::endl;
	}
}


