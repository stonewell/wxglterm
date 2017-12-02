#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include "term_data_param.h"

class CapStringValue {
};
using CapStringValuePtr = std::shared_ptr<CapStringValue>;

class ControlDataState {
public:
    virtual ~ControlDataState() {}
};

class DigitState : public ControlDataState {
public:
    virtual ~DigitState() {}
};

class AnyState : public ControlDataState {
public:
    virtual ~AnyState() {}
};

using ControlDataStatePtr = std::shared_ptr<ControlDataState>;
using DigitStatePtr = std::shared_ptr<DigitState>;
using AnyStatePtr = std::shared_ptr<AnyState>;

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

class ControlDataParserContext {
public:
    term_data_param_vector_t params;

    void push_param(const term_data_param_s & param) {
        params.push_back(param);
    }
};

using ControlDataParserContextPtr = std::shared_ptr<ControlDataParserContext>;
