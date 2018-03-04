#pragma once

#include <string>

bool get_entry(const std::string & termcap_file_path,
               const std::string & term_name,
               std::string & term_entry);
