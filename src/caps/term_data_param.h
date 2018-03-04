#pragma once

#include <string>
#include <iostream>
#include <vector>

struct term_data_param_s {
    std::string str_value;
    bool has_str_value;
    int int_value;
    bool has_int_value;

    term_data_param_s() :
        str_value {}
        , has_str_value{false}
        , int_value {0}
        , has_int_value {false}
    {
    }

    term_data_param_s(int v) :
        str_value {}
        , has_str_value{false}
        , int_value {v}
        , has_int_value {true}
    {
    }

    term_data_param_s(const std::string & v) :
        str_value {v}
        , has_str_value{true}
        , int_value {0}
        , has_int_value {false}
    {
    }

    term_data_param_s(const term_data_param_s & v) :
        str_value {v.str_value}
        , has_str_value{v.has_str_value}
        , int_value {v.int_value}
        , has_int_value {v.has_int_value}
    {
    }

    operator int () const {
        return int_value;
    }

    operator const char * () const {
        return str_value.c_str();
    }

    friend std::ostream& operator<<(std::ostream& os, const term_data_param_s& dt) {
        if (dt.has_int_value)
            os << dt.int_value;
        else if (dt.has_str_value)
            os << dt.str_value;
        else
            os << "None Value";

        return os;
    }

    term_data_param_s & operator = (const term_data_param_s & v) {
        this->str_value = v.str_value;
        this->has_str_value = v.has_str_value;
        this->int_value = v.int_value;
        this->has_int_value = v.has_int_value;
        return *this;
    }
};

using term_data_param_vector_t = std::vector<term_data_param_s>;
