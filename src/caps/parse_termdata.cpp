#include "parse_termdata.h"

#include <sstream>

static
void parse_str_cap(const std::string & field,
                   CapPtr cap);

CapPtr parse_cap(const std::string & cap_str)
{
    CapPtr cap = std::make_shared<Cap>();

    auto cap_fields = tokenize(cap_str, ":");

    for(auto & field : cap_fields) {
        if (field.length() == 0)
            continue;

        if (field.find("=") != std::string::npos) {
            parse_str_cap(field,
                          cap);
        } else if (field.find("#") != std::string::npos) {
            auto parts = tokenize(field, "#");

            cap->flags.emplace(parts[0], atoi(parts[1].c_str()));
        } else {
            cap->flags.emplace(field, 1);
        }
    }

    return cap;
}

static
void parse_padding(std::string & value,
                   std::string::size_type & pos,
                   double & padding)
{
    pos = 0;
    padding = 0.0;

    if (!(value[0] >= '0' && value[0] <= '9')) {
        return;
    }

    std::string padding_chars;
    bool get_dot = false;

    for(;pos < value.length(); pos++) {
        if (value[pos] == '*') {
            pos++;
            break;
        } else if (value[pos] == '.') {
            if (get_dot)
                break;
            get_dot = true;
            padding_chars.append(".");
        } else if (value[pos] >= '0' && value[pos] <= '9') {
            padding_chars.append(value.substr(pos, 1));
        } else {
            break;
        }
    }

    padding = atof(padding_chars.c_str());
}

static
void build_parser_state_machine(CapStringValuePtr & cap_str_value,
                                ControlDataStatePtr & state)
{
    std::string value = cap_str_value->value;

    std::string::size_type pos = 0;
    ControlDataStatePtr cur_state = state;
    ControlDataStatePtr repeat_state {};
    char repeat_char = 0;
    bool is_repeat_state = false;
    ControlDataStatePtr repeat_enter_state {};

    bool increase_param = false;
    bool is_digit_state = false;
    int digit_base = 10;
    string_vector_t params;
    std::string cap_value;
    bool const_digit_state = false;
    std::string const_digit;

     while (pos < value.length()) {
        char c = value[pos];

        switch (c) {
        case '\\': {
            pos ++;

            if (pos >= value.length())
                throw "unterminated cap str value";

            c = value[pos];

            switch(c){
            case 'E':
                c = 0x1B;
                break;
            case '\\':
                c = '\\';
                break;
            case '(':
                is_repeat_state = true;
                repeat_enter_state = cur_state;
                pos++;
                continue;
            case ')': {
                if (!repeat_state || repeat_char == 0) {
                    std::stringstream ss;
                    ss << "invalid repeat state:"
                       << pos
                       << ","
                       << value;
                    throw ss.str();
                }

                cur_state->add_state(repeat_char, repeat_state);
                repeat_char = 0;
                repeat_state = nullptr;
                is_repeat_state = false;
                pos++;
                continue;
            }
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': {
                int v = 0;
                while (pos < value.length() && c >= '0' && c <= '9') {
                    v = v * 8 + c - '0';
                    pos++;
                    if (pos < value.length())
                        c = value[pos];
                }

                if (!(c >= '0' && c <= '9'))
                    pos--;

                c = v & 0xFF;
            }
                break;
            default:{
                std::stringstream ss;
                ss << "invalid escape string:"
                   << pos
                   << ","
                   << value;
                throw ss.str();
            }
                break;
            }
        }
            break;
        case '^':
            pos++;
            if (pos >= value.length())
                throw "unterminated cap str value";
            c = value[pos] - 'A' + 1;
            break;
        case '%': {
            pos++;
            if (pos >= value.length())
                throw "unterminated cap str value";

            c = value[pos];
            switch(c){
            case '%':
                c = '%';
                break;
            case 's':
                cur_state = cur_state->add_any_state(std::make_shared<AnyState>());
                params.push_back("***");
                pos++;
                continue;
            case 'i':
                increase_param = true;
                pos++;
                continue;
            case 'd':
                is_digit_state = true;
                digit_base = 10;
                break;
            case 'X':
            case 'x':
                is_digit_state = true;
                digit_base = 16;
                break;
            default:{
                std::stringstream ss;
                ss << "invalid format string:"
                   << pos
                   << ","
                   << value;
                throw ss.str();
            }
                break;
            }
        }
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            int v = 0;
            while (pos < value.length() && c >= '0' && c <= '9') {
                v = v * 10 + c - '0';
                pos++;
                if (pos < value.length())
                    c = value[pos];
            }

            if (!(c >= '0' && c <= '9'))
                pos--;

            std::stringstream ss;
            ss << v;

            params.push_back(ss.str());
            const_digit_state = true;
            const_digit = ss.str();
        }
            break;
        default:
            break;
        }//switch

        if (is_digit_state) {
            DigitStatePtr digit_state = cur_state->add_digit_state(std::make_shared<DigitState>());
            digit_state->digit_base = digit_base;
            cur_state = digit_state;

            if (is_repeat_state)
                params.push_back("**");
            else
                params.push_back("*");
        } else if (const_digit_state) {
            DigitStatePtr digit_state =
                    cur_state->add_digit_state(std::make_shared<DigitState>());
            cur_state = digit_state;
            digit_state->digit_base = 10;
            cap_value.append(const_digit);
        } else {
            cur_state = cur_state->add_state(c, std::make_shared<ControlDataState>());
            cap_value.append(&c, 1);
        }

        if (is_repeat_state && !repeat_state) {
            repeat_state = cur_state;
            repeat_char = c;
        }

        if (!is_repeat_state && repeat_enter_state) {
            ControlDataStatePtr old_cur_state = cur_state;
            cur_state = repeat_enter_state->add_state(c, cur_state);

            if (cur_state != old_cur_state) {
                throw "should put generic pattern before special pattern";
            }

            repeat_enter_state = nullptr;
        }

        is_digit_state = false;
        const_digit_state = false;
        pos++;
    }//while

    cap_str_value->value = cap_value;

    std::string cap_name_key = join(params, ",");

    cur_state->update_cap_name(cap_name_key,
                               cap_str_value->name,
                               increase_param);
}

void parse_str_cap(const std::string & field,
                   CapPtr cap)
{
    CapStringValuePtr cap_str_value = std::make_shared<CapStringValue>();

    std::string::size_type pos = field.find("=");
    cap_str_value->name = field.substr(0, pos);
    std::string value = cap_str_value->value = field.substr(pos + 1);

    parse_padding(value, pos, cap_str_value->padding);
    value = cap_str_value->value = value.substr(pos);

    build_parser_state_machine(cap_str_value, cap->control_data_start_state);

    cap->cmds.emplace(cap_str_value->name,
                     cap_str_value);
}
