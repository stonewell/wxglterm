#pragma once

#include <string>
#include <vector>

using string_vector_t = std::vector<std::string>;

std::string strip(const std::string & txt,
                  const std::string & delimiters = "\t ");
string_vector_t tokenize(const std::string& str,
                         const std::string& delimiters,
                         bool include_delimiter = false);

template<class v_t, class value_type=typename v_t::value_type>
std::string join(const v_t & v, const std::string & delimiters);
template<class InputIterator, class value_type=typename InputIterator::value_type>
std::string join(const InputIterator & begin,
                 const InputIterator & end,
                 const std::string & delimiters);
