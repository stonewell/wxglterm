#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <tuple>

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
using CapStateMap = std::unordered_map<char, ControlDataStatePtr>;

class ControlDataParserContext {
public:
    term_data_param_vector_t params;

    void push_param(const term_data_param_s & param) {
        params.push_back(param);
    }
};

using ControlDataParserContextPtr = std::shared_ptr<ControlDataParserContext>;

class ControlDataState {
public:
    virtual ~ControlDataState() {}

public:
    virtual ControlDataStatePtr add_state(char c, ControlDataStatePtr next_state);
    virtual AnyStatePtr add_any_state(AnyStatePtr any_state);
    virtual DigitStatePtr add_digit_state(DigitStatePtr digit_state);
    virtual void reset();
    virtual ControlDataStatePtr handle(ControlDataParserContext context, char c);
    virtual bool get_cap(term_data_param_vector_t params,
                         CapNameMapValue & cap_name_value);
public:
    CapNameMap cap_name;
    CapStateMap next_states;
    DigitStatePtr digit_state;
    AnyStatePtr any_state;
};

class DigitState : public ControlDataState {
public:
    DigitState() :
        digit_base {10} {
    }

    virtual ~DigitState() {}

    int digit_base;
};

class AnyState : public ControlDataState {
public:
    virtual ~AnyState() {}
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
