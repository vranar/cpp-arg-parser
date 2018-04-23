/**
 * @file arg_parser.hpp
 * @brief Python-like CLI arguments parser -- header.
 * @author Ing. Roman Vrana, ivrana@fit.vutbr.cz
 * @date 2016-09-07
 */

#include <algorithm>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <set>

/**
 * @brief Argument type enumerator
 */
enum class ArgumentType {
    /*@{*/
    BOOL, ///< Boolean option
    INT,  ///< Integer option
    HEX,  ///< Hexadecimal format option
    FLT,  ///< Float option
    STR,  ///< String option
    /*@}*/
};

enum class ArgumentOption {
    REQUIRED,
    OPTIONAL,
    INHERIT_GROUP
};


/**
 * @brief Search key for loaded options
 */
struct arg_key
{
    std::string shr; ///< short option
    std::string lng; ///< long option


    /**
     * @brief Default constructor
     */
    arg_key() : shr(), lng() { }

    /**
     * @brief Constructor of the option key
     *
     * @param s short option name
     * @param l long option name
     */
    arg_key(std::string s, std::string l) : shr(std::move(s)), lng(std::move(l)) { }

    /**
     * @brief Copy-constructor of the option key
     *
     * @param other Input structure
     */
    arg_key(const arg_key& other) = default;

    /**
     * @brief Copy-assignment operator of the option key
     *
     * @param other Input structure
     */
    arg_key& operator=(const arg_key& other) = default;

    arg_key(arg_key&& other) = default;

    arg_key& operator=(arg_key&& other) = default;

    bool empty() const { return shr.empty() && lng.empty(); }


    /**
     * @brief Less-than operator for option key for std::map
     *
     * @param other RH-side structure.
     *
     * @return true if either short name is lexicographicaly shorter or long name is shorter
     *         while short names are the same, false otherwise.
     */
    inline bool operator< (const arg_key& other) const {
        if (shr < other.shr)
            return true;
        else if(shr == other.shr && lng < other.lng)
            return true;
        return false;
    }

    /**
     * @brief Equality operator for option key.
     *
     * @param other RH-side structure.
     *
     * @return true if both short and long names are the same.
     */
    inline bool operator== (const arg_key& other) const {
        return shr == other.shr && lng == other.lng;
    }

    inline bool operator != (const arg_key& other) const {
        return !(*this == other);
    }
};

/**
 * @brief Option data structure.
 */
struct arg_opt {
    std::string value;                 ///< option value
    ArgumentType type;                 ///< option type (see ArgumentType)
    bool is_set;                       ///< flag if option is set
    bool has_default;                  ///< flag if option has default value
    std::string desc;                  ///< description for the option used in help text

    /**
     * @brief Defualt constructor of the option data
     */
    arg_opt() : value(), type(), is_set(false), has_default(false), desc() { }

    /**
     * @brief Constructor of the option data
     *
     * @param v option value
     * @param t option type
     * @param d option description
     */
     arg_opt(std::string v, ArgumentType t, std::string d, bool def = false)
            : value(std::move(v)),
              type(t),
              is_set(def),
              has_default(def),
              desc(std::move(d))
    { }

    /**
     * @brief Copy-constructor of the option data structure
     *
     * @param other Input structure
     */
    arg_opt(const arg_opt& other) = default;

    /**
     * @brief Copy-assignment operator for the option data structure
     *
     * @param other Input structure
     */
    inline arg_opt& operator= (const arg_opt& other) = default;
};

/**
* @brief Positional argument data
*/
struct arg_pos {
    std::string value; ///< argument value
    std::string name;  ///< argument name used in help text

    /**
     * @brief Default constructor of positional argument structure.
     */
    arg_pos() : value(), name() {}

    /**
     * @brief Constructor of positional argument structure.
     *
     * @param v Argument value
     * @param n Optional argument name
     */
    explicit arg_pos(std::string v, std::string n="") : value(std::move(v)), name(std::move(n)) {}

    /**
     * @brief Copy-constructor of positional argument structure
     *
     * @param other Input structure
     */
    arg_pos(const arg_pos& other)= default;

    /**
     * @brief Copy-assignment operator for positional argument structure.
     *
     * @param other Input structure.
     */
    inline arg_pos& operator= (const arg_pos& other) = default;
};

struct arg_default : std::pair<bool, std::string> {
    arg_default() : std::pair<bool,std::string>(false, "") {}
    explicit arg_default(const std::string& v) : std::pair<bool,std::string>(true, v) {}
};

struct arg_group : std::set<arg_key>
{
protected:
    bool mandatory_;

public:
    explicit arg_group(bool m = false) : std::set<arg_key>(), mandatory_(m) {}

    bool mandatory() { return mandatory_; }

    void make_mandatory() {
        mandatory_ = true;
    }
};

/**
 * @brief Argument parser class -- Command line argument parser
 */
class ArgumentParser
{
protected:
    using options_t = std::map<arg_key, arg_opt>;
    const unsigned int OPT_WIDTH_;       ///< option name field width

    std::string exec_name_;                                 ///< executable name
    std::vector<arg_pos> positional_;                       ///< positional arguments
    options_t options_;                                     ///< options
    std::set<arg_key> mandatory_;                           ///< mandatory options
    std::unordered_map<std::string, arg_group> mtx_groups_; ///< Mutually exclusive groups

    std::string prog_desc_;              ///< program description
    std::string usage_;                  ///< program usage

    bool option_is_mutually_exclusive_(const arg_key& ak)
    {
        return std::any_of(mtx_groups_.begin(),
                           mtx_groups_.end(),
                           [&ak](auto& group)
                           {
                               return group.second.find(ak) != group.second.end();
                           }
        );
    }

    void make_option_mandatory_(const arg_key& ak)
    {
        mandatory_.emplace(ak);
    }

    decltype(auto) check_mandatory_options_();
    decltype(auto) check_mandatory_option_groups_();
    decltype(auto) check_option_conflicts_();

    const options_t::iterator find_option_(const std::string& key) {
        return std::find_if(options_.begin(),
                            options_.end(),
                            [&key](const std::pair<arg_key, arg_opt>& a) -> bool
                            {
                                return a.first.shr == key || a.first.lng == key;
                            }
        );
    }

    const options_t::iterator find_option_(const arg_key& ak) {

        auto&& opt = find_option_(ak.shr);

        if (opt == options_.end()) {
            find_option_(ak.lng);
        }

        return opt;
    }

public:

    /**
     * @brief Constructor of the ArgumentParser 
     *
     * @param desc program description
     * @param usage program usage
     */
    explicit ArgumentParser(const std::string& desc = "", const std::string& usage = "");

    /**
     * @brief Method for registering option to ArgumentParser. 
     *
     * @param short_opt short option name
     * @param long_opt long option name
     * @param required required flag
     * @param type option type
     * @param desc option description
     *
     * @return true if option was successfully registered in ArgumentParser
     */

    bool register_option(const arg_key& ak,
                         ArgumentOption opt,
                         ArgumentType type,
                         const std::string& desc,
                         const std::string& excl_group = "",
                         const arg_default& default_value = arg_default());

    /**
     * @brief Method for registering positional arguments. 
     *
     * @param count Number of positional arguments.
     * @param names Vector of positional arguments names. Names will be used in usage text.
     */
    void register_positional(unsigned count,
                             std::vector<std::string> names=std::vector<std::string>());

    bool add_mutually_exclusive_group(const std::string& grp_name, bool required = false) {
        return mtx_groups_.emplace(grp_name, arg_group(required)).second;
    }

    bool insert_into_group(const std::string& grp_name, const arg_key& ak) {
        if (mtx_groups_.count(grp_name)) {
            mtx_groups_.at(grp_name).emplace(ak).second;
            return true;
        }

        return false;
    }

    /**
     * @brief Method for loading CLI arguments. 
     *
     * @param argc argument count
     * @param argv argument vector
     */
    void load_arguments(int argc, char **argv);

    template<typename T> bool has_option(const T& key) {
        return find_option_(key) != options_.end();
    }

    template<typename T> bool option_is_set(const T& key) {
        const auto opt = find_option_(key);

        return has_option(key) && opt->second.is_set;
    }

    /**
     * @brief Operator for getting the option value.
     *
     * The operator tries to find the option by key and returns its value as
     * string without any conversion. To convert the option to a different type
     * use method ArgumentParser::parse_option instead.
     *
     * @param key Key to the option. It can be either short name or long name.
     *
     * @return Option value on success, empty string otherwise.
     */
    const std::string operator[] (const std::string& key) const
    {
        auto opt = std::find_if(options_.begin(),
                                options_.end(),
                                [&key](const std::pair<arg_key, arg_opt>& a) -> bool
                                {
                                    return a.first.shr == key || a.first.lng == key;
                                }
        );
        if (opt != options_.end()) {
            return opt->second.value;
        }

        return "";
    }

    /**
     * @brief Operator for getting the positional parameter value.
     *
     * The operator returns the positional value at given index without confersion.
     * To convert the value to a different type, use method ArgumentParser::parse_positional
     * instead.
     *
     * @param idx Index of positional argument.
     *
     * @return Value of positional parameter.
     */
    const std::string& operator[] (size_t idx) const
    {
        return positional_.at(idx).value;
    }

    /**
     * @brief Method for getting option value.
     *
     * Method gets the value for option and automatically converts it to desired type.
     *
     * @tparam T return type.
     * @param opt option name
     *
     * @return option value
     */
    template<typename T> decltype(auto) parse_option(const std::string& opt)
    {
        auto&& val = std::find_if(this->options_.begin(),
                                  this->options_.end(),
                                  [opt](const std::pair<arg_key, arg_opt>& a)
                                  {
                                      return a.first.shr == opt || a.first.lng == opt;
                                  }
        );

        std::stringstream ss;

        if (val != options_.end() && val->second.is_set) {
            ss << val->second.value;
            T opt_val;
            switch (val->second.type) {
                case ArgumentType::BOOL:
                    opt_val = val->second.is_set;
                    break;
                case ArgumentType::HEX:
                        ss << std::hex;
                        ss >> opt_val;
                    break;
                default:
                    if (!(ss >> opt_val)) {
                        throw std::logic_error("Cannot convert option to given type. (" + ss.str() +")");
                    }
            }
            return opt_val;
        }
        return T();
    }

    /**
     * @brief Method for getting positional argument value. 
     *
     * Method gets the value for option and automatically converts it to desired type.
     *
     * @tparam T return type
     * @param idx positional argument index
     *
     * @return argument value
     */
    template<typename T> decltype(auto) parse_positional(int idx)
    {
        std::stringstream ss;

        if (idx > static_cast<int>((this->positional_.size() - 1)) ||  idx < 0) {
            throw std::logic_error("Positional argument index out of range.");
        }

        ss << positional_[idx].value;

        T opt_val;
        if (!(ss >> opt_val)) {
            throw std::logic_error("Cannot convert positional " + std::to_string(idx) + " to given type. (" + ss.str() + ")");
        }

        return opt_val;
    }

    /**
     * @brief Method for printing help text.
     */
    void print_help_text();

    /**
     * @brief Method for printing usage text.
     */
    void print_usage_text();

    /**
     * @brief Method for setting usage text. 
     *
     * @param txt new usage text
     */
    void set_usage_text(const std::string& txt) { this->usage_ = txt; }
};

