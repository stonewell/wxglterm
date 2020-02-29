#include "parse_termdata.h"
#include <algorithm>

ControlDataStatePtr ControlDataState::add_state(char c, ControlDataStatePtr next_state) {
    auto & cur_state = this->next_states[(int)(c & 0xFF)];

    if (cur_state)
        return cur_state;

    cur_state = next_state;

    return cur_state;
}

AnyStatePtr ControlDataState::add_any_state(AnyStatePtr _any_state) {
    if (this->any_state)
        return this->any_state;

    this->any_state = _any_state;
    return _any_state;
}

DigitStatePtr ControlDataState::add_digit_state(DigitStatePtr _digit_state) {
    if (this->digit_state)
        return this->digit_state;

    this->digit_state = _digit_state;
    return _digit_state;
}

void ControlDataState::reset() {
}

ControlDataStatePtr ControlDataState::handle(ControlDataParserContextPtr context, char c) {
    auto & it = this->next_states[(int)(c & 0xFF)];

    if (it) {
        if (it->digit_state)
            it->digit_state->reset();

        return it;
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

    this->ordered_cap_name_keys.clear();

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

                  return a_key.compare(b_key) < 0;
              });
}

bool ControlDataState::get_cap(const term_data_param_vector_t & params,
                               CapNameMapValue & cap_name_value) {
#define CHECK_AND_RETURN(name) \
    { \
        auto it = this->cap_name.find(name); \
        if (it != this->cap_name.end()) { \
            cap_name_value = it->second; \
            return true; \
        } \
    }

    if (params.size() == 0) {
        if (has_empty_cap_name_key) {
            cap_name_value = empty_cap_name_key_value_pair;
            return true;
        }

        if (has_three_star_cap_name_key) {
            cap_name_value == three_star_cap_name_key_value_pair;
            return true;
        }

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

    if (next_state) {
        context->push_param(v);
        v.clear();
        return next_state;
    }

    v.append(&c, 1);

    return shared_from_this();
}

void ControlDataState::update_cap_name(const std::string & cap_name_key,
                                       const std::string & cap_str_value_name,
                                       bool increase_param) {
    auto pair = std::make_tuple(cap_str_value_name, increase_param);
    auto result =
            this->cap_name.insert(std::make_pair(cap_name_key,
                                                 pair));
    if (!result.second) {
        if (std::get<0>(result.first->second) != cap_str_value_name ||
            std::get<1>(result.first->second) != increase_param) {
            std::stringstream sss;
            sss << "same parameter for different cap name:["
                << cap_name_key
                << "],"
                << cap_str_value_name;

            throw sss.str();
        }
    }

    if (cap_name_key == "") {
        has_empty_cap_name_key = true;
        empty_cap_name_key_value_pair = pair;
    } else if (cap_name_key == "***") {
        has_three_star_cap_name_key = true;
        three_star_cap_name_key_value_pair = pair;
    }
}
