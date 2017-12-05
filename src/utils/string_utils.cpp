#include "string_utils.h"

#include <sstream>
#include <iterator>

string_vector_t tokenize(const std::string& str,
                         const std::string& delimiters,
                         bool include_delimiter)
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

std::string strip(const std::string & txt,
                  const std::string & delimiters) {
    std::string::size_type pos, last_pos;

    pos = txt.find_first_not_of(delimiters);
    last_pos = txt.find_last_not_of(delimiters);

    if (pos == std::string::npos && last_pos == std::string::npos)
        return txt;
    if (pos == std::string::npos || last_pos == std::string::npos)
        return txt;

    return txt.substr(pos, last_pos + 1 - pos);
}

template<class InputIterator, class value_type>
std::string join(const InputIterator & begin,
                 const InputIterator & end,
                 const std::string & delimiters) {
    std::stringstream ss1;
    std::copy(begin, end,
              std::ostream_iterator<value_type>(ss1, delimiters.c_str()));

    return ss1.str();
}

template<class v_t, class value_type>
std::string join(const v_t & v, const std::string & delimiters) {
    return join<v_t::iterator, value_type>(v.begin(), v.end(), delimiters);
}
