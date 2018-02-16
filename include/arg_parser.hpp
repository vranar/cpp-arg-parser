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
    arg_key(const std::string& s, const std::string& l) : shr(s), lng(l) { }

    /**
     * @brief Copy-constructor of the option key
     *
     * @param other Input structure
     */
    arg_key(const arg_key& other) : shr(other.shr), lng(other.lng) { }

    bool empty() const { return shr.empty() && lng.empty(); }

    /**
     * @brief Copy-assignment operator of the option key 
     *
     * @param other Input structure
     */
    inline arg_key& operator= (const arg_key& other) {
        this->shr = other.shr;
        this->lng = other.lng;

        return *this;
    }

    /**
     * @brief Less-than operator for option key for std::map 
     *
     * @param other RH-side structure.
     *
     * @return true if either short name is lexicographicaly shorter or long name is shorter
     *         while short names are the same, false otherwise.
     */
    inline bool operator< (const arg_key& other) const {
        if (this->shr < other.shr)
            return true;
        else if(this->shr == other.shr && this->lng < other.lng)
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
        return this->shr == other.shr && this->lng == other.lng;
    }

    inline bool operator != (const arg_key& other) const {
        return !(*this == other);
    }
};

/**
 * @brief Option data structure.
 */
struct arg_opt {
    std::string value; ///< option value
    int type;          ///< option type (ARG_{BOOL,INT,FLOAT,STR}
    bool is_set;       ///< flag if option is set
    bool has_default;  ///< flag if option has default value
    std::string desc;  ///< description for the option used in help text

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
    arg_opt(const std::string& v, int t, const std::string& d, bool def = false)
        : value(v), type(t), is_set(false), has_default(def), desc(d)
    { 
        if (has_default) {
            is_set = true;
        }
    }

    /**
     * @brief Copy-constructor of the option data structure
     *
     * @param other Input structure
     */
    arg_opt(const arg_opt& other)
        : value(other.value),
          type(other.type),
          is_set(other.is_set),
          has_default(other.has_default),
          desc(other.desc)
    { }

    /**
     * @brief Copy-assignment operator for the option data structure
     *
     * @param other Input structure
     */
    inline arg_opt& operator= (const arg_opt& other) {
        this->value = other.value;
        this->type = other.type;
        this->is_set = other.is_set;
        this->has_default = other.has_default;
        this->desc = other.desc;

        return *this;
    }
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
    arg_pos(const std::string& v, const std::string& n="") : value(v), name(n) {}

    /**
     * @brief Copy-constructor of positional argument structure 
     *
     * @param other Input structure
     */
    arg_pos(const arg_pos& other) : value(other.value), name(other.name) {}

    /**
     * @brief Copy-assignment operator for positional argument structure.
     *
     * @param other Input structure.
     */
    inline arg_pos& operator= (const arg_pos& other) {
        this->value = other.value;
        this->name = other.name;

        return *this;
    }
};

struct arg_default : std::pair<bool, std::string> {
    arg_default() : std::pair<bool,std::string>(false, "") {}
    explicit arg_default(const std::string& v) : std::pair<bool,std::string>(true, v) {}
};

/**
 * @brief Argument parser class -- Command line argument parser
 */
class ArgumentParser
{
protected:
    typedef std::map<arg_key, arg_opt> options_t;

    const unsigned int OPT_WIDTH;       ///< option name field width

    std::string exec_name;              ///< executable name
    std::vector<arg_pos> positional;    ///< positional arguments
    options_t options;                  ///< options
    std::vector<arg_key> mandatory;     ///< mandatory options

    std::string prog_desc;              ///< program description
    std::string usage;                  ///< program usage
    int req_positional;                 ///< requiered positional arguments count

public:
    /**
     * @brief Argument type enumerator
     */
    enum {
        /*@{*/
        ARG_BOOL, ///< Boolean option
        ARG_INT,  ///< Integer option
        ARG_FLT,  ///< Float option
        ARG_STR,  ///< String option
        /*@}*/
    };

    /**
     * @brief Constructor of the ArgumentParser 
     *
     * @param desc program description
     * @param usage program usage
     */
    ArgumentParser(const std::string& desc = "", const std::string& usage = "");

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
                          bool required,
                          int type,
                          std::string desc,
                          const arg_default& default_value = arg_default());

    /**
     * @brief Method for registering positional arguments. 
     *
     * @param count Number of positional arguments.
     * @param names Vector of positional arguments names. Names will be used in usage text.
     */
    void register_positional(unsigned count,
                             std::vector<std::string> names=std::vector<std::string>());

    /**
     * @brief Method for loading CLI arguments. 
     *
     * @param argc argument count
     * @param argv argument vector
     */
    void load_arguments(int argc, char **argv);

    const options_t::iterator find_option(const std::string& key) {
        return std::find_if(this->options.begin(), this->options.end(),
                            [&key](const std::pair<arg_key, arg_opt>& a)
                            { return a.first.shr == key || a.first.lng == key; });
    }

    bool has_option(const std::string& key) {
        return this->find_option(key) != this->options.end();
    }

    bool option_is_set(const std::string& key) {
        const auto opt = this->find_option(key);

        return opt != this->options.end() && opt->second.is_set;
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
        auto opt = std::find_if(this->options.begin(), this->options.end(),
                                [&key](const std::pair<arg_key, arg_opt>& a)
                                { return a.first.shr == key || a.first.lng == key; });
        if (opt != this->options.end()) {
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
    const std::string& operator[] (int idx) const
    {
        return this->positional[idx].value;
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
    template<typename T> T parse_option(const std::string& opt)
    {
        auto&& val = std::find_if(this->options.begin(),
                                  this->options.end(),
                                  [opt](const std::pair<arg_key, arg_opt>& a)
                                  { return a.first.shr == opt || a.first.lng == opt; });

        std::stringstream ss;

        if (val != this->options.end() && val->second.is_set) {
            ss << val->second.value;
            T opt_val;
            if (val->second.type == ARG_BOOL) {
                opt_val = val->second.is_set;
            } else {
                if (!(ss >> opt_val))
                    throw std::logic_error("Cannot convert option to given type. (" + ss.str() +")");
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
    template<typename T> T parse_positional(int idx)
    {
        std::stringstream ss;

        if (idx > static_cast<int>((this->positional.size() - 1)) ||  idx < 0) {
            throw std::logic_error("Positional argument index out of range.");
        }

        ss << this->positional[idx].value;

        T opt_val;
        if (!(ss >> opt_val))
            throw std::logic_error("Cannot convert positional "+ std::to_string(idx) +" to given type. (" + ss.str() +")");

        return opt_val;
    }

    /**
     * @brief Method for printing help text.
     */
    void print_help_text();

    /**
     * @brief Method for setting usage text. 
     *
     * @param txt new usage text
     */
    void set_usage_text(const std::string& txt) { this->usage = txt; }
};
