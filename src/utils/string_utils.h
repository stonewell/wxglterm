#pragma once

#include <string>
#include <vector>

#include <sstream>
#include <iterator>

using string_vector_t = std::vector<std::string>;

std::string strip(const std::string & txt,
                  const std::string & delimiters = "\t ");
string_vector_t tokenize(const std::string& str,
                         const std::string& delimiters,
                         bool include_delimiter = false);

template<class InputIterator, class value_type = typename InputIterator::value_type>
std::string join(const InputIterator & begin,
                 const InputIterator & end,
                 const std::string & delimiters) {
    std::stringstream ss1;
    std::copy(begin, end,
              std::ostream_iterator<value_type>(ss1, delimiters.c_str()));

    return ss1.str();
}

template<class v_t, class value_type = typename v_t::value_type>
std::string join(const v_t & v, const std::string & delimiters) {
    return join<typename v_t::const_iterator, value_type>(v.begin(), v.end(), delimiters);
}

std::string replace(std::string str, const std::string & target, const std::string & repl);
