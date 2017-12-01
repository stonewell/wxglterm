#include "read_termdata.h"

#include <string.h>

#include <memory>
#include <vector>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <unordered_set>

using string_vector_t = std::vector<std::string>;
using entry_map_t = std::unordered_map<std::string, string_vector_t *>;

static
entry_map_t entry_map {};

static
constexpr uint32_t MAX_DEPTH = 25;

static
bool get_entry(const std::string & termcap_file_path,
               const std::string & term_name,
               std::string & term_entry,
               uint32_t depth);

static
void clear_entry_map(entry_map_t & m) {
    std::unordered_set<string_vector_t *> v;

    for(auto & it : m) {
        v.insert(it.second);
    }

    for(auto & it : v) {
        delete it;
    }
}

bool get_entry(const std::string & termcap_file_path,
               const std::string & term_name,
               std::string & term_entry)
{
    clear_entry_map(entry_map);

    bool result = get_entry(termcap_file_path,
                     term_name,
                     term_entry,
                     0);

    clear_entry_map(entry_map);
    return result;
}

static
string_vector_t tokenize(const std::string& str,
                         const std::string& delimiters,
                         bool include_delimiter = false)
{
    string_vector_t tokens;
    std::string::size_type delimPos = 0, tokenPos = 0, pos = 0;

    if(str.length()<1)  return tokens;
    while(1){
        delimPos = str.find_first_of(delimiters, pos);
        tokenPos = str.find_first_not_of(delimiters, pos);

        if(std::string::npos != delimPos){
            if(std::string::npos != tokenPos){
                if(tokenPos<delimPos){
                    tokens.push_back(str.substr(pos,delimPos-pos));
                }else{
                    tokens.push_back("");
                }
            }else{
                tokens.push_back("");
            }
            pos = delimPos+1;

            if (include_delimiter)
                tokens.push_back(str.substr(delimPos, 1));

        } else {
            if(std::string::npos != tokenPos){
                tokens.push_back(str.substr(pos));
            } else {
                tokens.push_back("");
            }
            break;
        }
    }
    return tokens;
}

static
std::string strip(const std::string & txt,
                  const std::string & delimiters = "\t ") {
    std::string::size_type pos, last_pos;

    pos = txt.find_first_not_of(delimiters);
    last_pos = txt.find_last_not_of(delimiters);

    if (pos == std::string::npos && last_pos == std::string::npos)
        return txt;
    if (pos == std::string::npos || last_pos == std::string::npos)
        return txt;

    return txt.substr(pos, last_pos + 1 - pos);
}

static
void expand_cap(const std::string & termcap_file_path,
                const std::string & cap_value,
                std::string & expanded_cap_value,
                uint32_t depth)
{
    string_vector_t cap_parts {};

    auto tokens = tokenize(cap_value, ":");

    for(auto & token : tokens) {
        std::string::size_type name_pos = token.find("tc=");
        if (name_pos == 0) {
            std::string sub_entry_value;
            if (!get_entry(termcap_file_path,
                           token.substr(strlen("tc=")),
                           sub_entry_value,
                           depth + 1)) {
                continue;
            }

            for(auto & sub_token : tokenize(sub_entry_value, ":")) {
                if (sub_token.length() > 0)
                    cap_parts.push_back(sub_token);
            }
        } else if (token.length() > 0) {
            cap_parts.push_back(token);
        }
    }

    std::stringstream ss;
    std::copy(cap_parts.begin(), cap_parts.end(),
              std::ostream_iterator<std::string>(ss, ":"));

    expanded_cap_value = ss.str();
}

bool get_entry(const std::string & termcap_file_path,
               const std::string & term_name,
               std::string & term_entry,
               uint32_t depth)
{
    entry_map_t::iterator it = entry_map.find(term_name);

    if (it != entry_map.end()) {
        expand_cap(termcap_file_path,
                   *it->second->begin(),
                   *it->second->begin(),
                   depth);

        term_entry = *it->second->begin();
        return true;
    }

    if (depth > MAX_DEPTH)
        return false;

    std::ifstream ifs(termcap_file_path);
    std::string line {};

    if (!ifs.is_open())
        return false;

    std::stringstream entry;

    while(std::getline(ifs, line)) {
        line = strip(line);

        if (line.length() == 0 || line[0] == '#')
            continue;

        if (*(line.end() - 1) == '\\') {
            entry << line.substr(0, line.length() - 1);
            continue;
        } else {
            entry << line;
        }

        auto parts = tokenize(entry.str(), "|:", true);

        std::stringstream cap;
        string_vector_t names;

        for(string_vector_t::iterator it=parts.begin(),
                    it_end = parts.end();
            it != it_end;
            it++)
        {
            auto p = *it;
            if (p == ":") {
                if (it + 1 != it_end) {
                    std::copy(it + 1, it_end,
                              std::ostream_iterator<std::string>(cap, ""));
                }
                break;
            } else if (p == "|") {
                continue;
            }
            names.push_back(p);
        }

        std::string cap_value = cap.str();
        bool term_name_found = false;
        if (std::find(names.begin(), names.end(), term_name) != names.end()) {
            term_name_found = true;
            expand_cap(termcap_file_path,
                       cap_value,
                       cap_value,
                       depth);
        }

        string_vector_t * cap_array = new string_vector_t{};
        cap_array->push_back(cap_value);

        for(auto & name : names) {
            entry_map.emplace(name, cap_array);
        }

        if (term_name_found)
            break;

        entry.clear();
    }

    term_entry = *entry_map[term_name]->begin();

    return true;
}
