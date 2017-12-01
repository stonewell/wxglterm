#pragma once

#include <string>
#include <vector>

using string_vector_t = std::vector<std::string>;

std::string strip(const std::string & txt,
                  const std::string & delimiters = "\t ");
string_vector_t tokenize(const std::string& str,
                         const std::string& delimiters,
                         bool include_delimiter = false);
