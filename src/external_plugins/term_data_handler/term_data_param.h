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
};

using term_data_param_vector_t = std::vector<term_data_param_s>;
