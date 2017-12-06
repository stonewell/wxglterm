#include "parse_termdata.h"
#include <algorithm>

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

ControlDataStatePtr ControlDataState::handle(ControlDataParserContextPtr context, char c) {
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

static
std::string build_cmp_key(const std::string & key)
{
    std::stringstream ss;
    ss << std::count(key.begin(), key.end(), '*') << key;

    return ss.str();
}

void ControlDataState::init_ordered_cap_name_keys()
{
    if (ordered_cap_name_keys_initialized)
        return;

    ordered_cap_name_keys_initialized = true;

    for (auto & p : this->cap_name) {
        auto k = p.first;

        auto re_str = replace(k, "***", "(.:)");
        re_str = replace(re_str, ",**", "(,[0-9]+)?");
        re_str = replace(re_str, "**", "([0-9]+)?");
        re_str = replace(re_str, "*", "[0-9]+");
        re_str = replace(re_str, "?", "*");
        re_str = replace(re_str, ":", "*");

        this->ordered_cap_name_keys.push_back(std::make_tuple(k,
                                                              std::regex(re_str),
                                                              k.find("*") != std::string::npos
                                                              ));
    }

    std::sort(this->ordered_cap_name_keys.begin(),
              this->ordered_cap_name_keys.end(),
              [](const ordered_cap_name_keys_t::value_type & a, const ordered_cap_name_keys_t::value_type & b) {
                  auto a_key = build_cmp_key(std::get<0>(a));
                  auto b_key = build_cmp_key(std::get<0>(b));

                  return a_key.compare(b_key);
              });
}

bool ControlDataState::get_cap(const term_data_param_vector_t & params,
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

    init_ordered_cap_name_keys();

    for(auto & p : this->ordered_cap_name_keys) {
        if (!std::get<2>(p)) {
            if (std::get<0>(p) == str_match) {
                cap_name_value = this->cap_name[std::get<0>(p)];
                return true;
            }
            continue;
        }

        if (std::regex_match(str_match, std::get<1>(p))) {
            cap_name_value = this->cap_name[std::get<0>(p)];
            return true;
        }
    }

    return false;
#undef CHECK_AND_RETURN
}

ControlDataStatePtr DigitState::handle(ControlDataParserContextPtr context, char c)
{
    if (digit_base == 2 && c >= '0' && c <= '1') {
        if (!has_value) { value = 0; has_value = true; }
        value = value * digit_base + c - '0';
    } else if (digit_base == 8 && c >= '0' && c <= '7') {
        if (!has_value) { value = 0; has_value = true; }
        value = value * digit_base + c - '0';
    } else if (digit_base == 10 && c >= '0' && c <= '9') {
        if (!has_value) { value = 0; has_value = true; }
        value = value * digit_base + c - '0';
    } else if (digit_base == 16 &&
               ((c >= '0' && c <= '9')
                || ( c >= 'A' && c <= 'Z')
                || ( c >= 'a' && c <= 'z'))) {
        if (!has_value) { value = 0; has_value = true; }
        value = value * digit_base;

        if (c >= '0' && c <= '9')
            value += c - '0';
        if (c >= 'A' && c <= 'Z')
            value += c - 'A';
        if (c >= 'a' && c <= 'z')
            value += c - 'a';
    } else {
        if (has_value)
            context->push_param(value);
        has_value = false;

        return ControlDataState::handle(context, c);
    }

    return shared_from_this();
}

ControlDataStatePtr AnyState::handle(ControlDataParserContextPtr context, char c) {
    auto next_state = ControlDataState::handle(context, c);

    if (next_state)
        return next_state;

    context->push_param(c);

    return shared_from_this();
}
