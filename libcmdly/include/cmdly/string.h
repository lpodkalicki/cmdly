/*
 * Copyright (c) 2023 by Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 */

#ifndef CMDLY_STRING_H
#define CMDLY_STRING_H

#include <cstdint>
#include <string>
#include <string_view>
#include <regex>
#include <sstream>

namespace cmdly::string {

template<typename T>
static inline void format_helper(std::ostringstream &oss, std::string_view &str, const T &value)
{
    std::size_t openBracket = str.find('{');
    if (openBracket==std::string::npos) { return; }
    std::size_t closeBracket = str.find('}', openBracket + 1);
    if (closeBracket==std::string::npos) { return; }
    oss << str.substr(0, openBracket) << value;
    str = str.substr(closeBracket + 1);
}

template<typename... Targs>
static inline std::string format(std::string_view str, Targs...args)
{
    std::ostringstream oss;
    (format_helper(oss, str, args), ...);
    oss << str;
    return oss.str();
}

static inline std::string replace(std::string& s, const std::string& search, const std::string& replace)
{
    return std::regex_replace(s, std::regex(search), replace);
}

static inline std::string& ltrim(std::string& s, const char* t = " \t\n\r")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

static  inline std::string& rtrim(std::string& s, const char* t = " \t\n\r")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

static inline std::string& trim(std::string& s, const char* t = " \t\n\r")
{
    return ltrim(rtrim(s, t), t);
}

} /* End of namespace cmdly::string */

#endif /* !CMDLY_STRING_H */
