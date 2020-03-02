#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <tuple>
#include <regex>

#include "term_data_param.h"
#include "string_utils.h"

struct CapStringValue {
    double padding;
    std::string value;
    std::string name;
};
using CapStringValuePtr = std::shared_ptr<CapStringValue>;

class ControlDataState;
class DigitState;
class AnyState;

using ControlDataStatePtr = std::shared_ptr<ControlDataState>;
using DigitStatePtr = std::shared_ptr<DigitState>;
using AnyStatePtr = std::shared_ptr<AnyState>;
using CapNameMapValue = std::tuple<std::string, bool>;
using CapNameMap = std::unordered_map<std::string, CapNameMapValue>;

class ControlDataParserContext {
public:
    term_data_param_vector_t params;

    void push_param(const term_data_param_s & param) {
        params.push_back(param);
    }
};

using ControlDataParserContextPtr = std::shared_ptr<ControlDataParserContext>;

class ControlDataState : public std::enable_shared_from_this<ControlDataState> {
public:
    ControlDataState()
        : ordered_cap_name_keys_initialized {false}
        , ordered_cap_name_keys {}
        , has_empty_cap_name_key {false}
        , empty_cap_name_key_value_pair {}
        , has_three_star_cap_name_key {false}
        , three_star_cap_name_key_value_pair {}
    {
    }
    virtual ~ControlDataState() {}

public:
    virtual ControlDataStatePtr add_state(char c, ControlDataStatePtr next_state);
    virtual AnyStatePtr add_any_state(AnyStatePtr any_state);
    virtual DigitStatePtr add_digit_state(DigitStatePtr digit_state);
    virtual void reset();
    virtual ControlDataStatePtr handle(ControlDataParserContextPtr context, char c);
    virtual bool get_cap(const term_data_param_vector_t & params,
                         CapNameMapValue & cap_name_value);

    void init_ordered_cap_name_keys();
public:
    ControlDataStatePtr next_states[255];
    DigitStatePtr digit_state;
    AnyStatePtr any_state;

    bool ordered_cap_name_keys_initialized;
    using ordered_cap_name_keys_t = std::vector<std::tuple<std::string, std::regex, bool>>;

    ordered_cap_name_keys_t ordered_cap_name_keys;

    friend std::ostream& operator<<(std::ostream& os, const ControlDataState& dt) {
        for(auto & p : dt.cap_name) {
            os << "[" <<p.first << ", " << std::get<0>(p.second) << "]" << std::endl;
        }

        for(int i=0;i < 255;i++) {
            if (dt.next_states[i]) {
                os << "[" << i << "," << (char)i << "]" << std::endl;
            }
        }

        return os;
    }

    void update_cap_name(const std::string & cap_name_key,
                         const std::string & cap_str_value_name,
                         bool increase_param);
private:
    CapNameMap cap_name;

    bool has_empty_cap_name_key;
    CapNameMapValue empty_cap_name_key_value_pair;
    bool has_three_star_cap_name_key;
    CapNameMapValue three_star_cap_name_key_value_pair;
};

class DigitState : public ControlDataState {
public:
    DigitState() :
        digit_base {10}
        , has_value {false}
        , value {0}
    {
    }

    virtual ~DigitState() {}

    virtual ControlDataStatePtr handle(ControlDataParserContextPtr context, char c);

    int digit_base;
    bool has_value;
    int value;

    virtual void reset() {
        has_value = false;
    }
};

class AnyState : public ControlDataState {
public:
    virtual ~AnyState() {}
    virtual ControlDataStatePtr handle(ControlDataParserContextPtr context, char c);

    std::string v;
};

using flag_map_t = std::unordered_map<std::string, uint32_t>;
using cmd_map_t = std::unordered_map<std::string, CapStringValuePtr>;

class Cap {
public:
    Cap() :
        flags{}
        , cmds {}
        , control_data_start_state { std::make_shared<ControlDataState>() }
    {
    }

    flag_map_t flags;
    cmd_map_t cmds;
    ControlDataStatePtr control_data_start_state;
};

using CapPtr = std::shared_ptr<Cap>;

CapPtr parse_cap(const std::string & cap_str);
