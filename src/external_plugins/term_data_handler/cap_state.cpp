#include "parse_termdata.h"

ControlDataStatePtr ControlDataState::add_state(char c, ControlDataStatePtr next_state) {

    auto result = this->next_states.insert(std::make_pair(c, next_state));

    return result.first->second;
}

AnyStatePtr ControlDataState::add_any_state(AnyStatePtr any_state) {
    if (this->any_state)
        return this->any_state;

    this->any_state = any_state;
    return any_state;
}

DigitStatePtr ControlDataState::add_digit_state(DigitStatePtr digit_state) {
    if (this->digit_state)
        return this->digit_state;

    this->digit_state = digit_state;
    return digit_state;
}

void ControlDataState::reset() {
}

ControlDataStatePtr ControlDataState::handle(ControlDataParserContext context, char c) {
    auto it = this->next_states.find(c);

    if (it != this->next_states.end()) {
        auto next_state = it->second;

        if (next_state->digit_state)
            next_state->digit_state->reset();

        return next_state;
    }

    if (this->digit_state) {
        auto next_state = this->digit_state->handle(context, c);

        if (next_state) {
            return next_state;
        }
    }

    if (this->any_state) {
        return this->any_state->handle(context, c);
    }

    return ControlDataStatePtr {};
}

bool ControlDataState::get_cap(term_data_param_vector_t params,
                               CapNameMapValue & cap_name_value) {
#define CHECK_AND_RETURN(name) \
    { \
        auto it = this->cap_name.find(""); \
        if (it != this->cap_name.end()) { \
            cap_name_value = it->second; \
            return true; \
        } \
    }

    if (params.size() == 0) {
        CHECK_AND_RETURN("");
        CHECK_AND_RETURN("***");

        return false;
    }

    std::string str_match = join(params, ",");

    CHECK_AND_RETURN(str_match);

    return false;
#undef CHECK_AND_RETURN
}
