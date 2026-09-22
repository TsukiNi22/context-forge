/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 23/09/2026 by @author Tsukini

File Name:
##  @file plugin.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/rules/DropRule.hpp"
#include <libconfig.h++>
#include <regex>
#include <string>
#include <vector>

_hot _nodiscard static std::vector<std::string> splitLines(const std::string& content)
{
    std::vector<std::string> lines;
    std::size_t start = 0;

    while (start <= content.size()) {
        std::size_t end = content.find('\n', start);
        if (end == std::string::npos) {
            lines.emplace_back(content.substr(start));
            break;
        }
        lines.emplace_back(content.substr(start, end - start));
        start = end + 1;
    }
    return lines;
}

_hot _nodiscard static std::string joinLines(const std::vector<std::string>& lines)
{
    std::string result;

    for (std::size_t i = 0; i < lines.size(); ++i) {
        result += lines[i];
        if (i + 1 < lines.size())
            result += '\n';
    }
    return result;
}

void forge::rules::DropRule::load(const libconfig::Setting& s)
{
    // check type
    if (!s.isArray()) _unlikely {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s.getPath() + ": the drop value must be an array of string [\"...\", ...]");
    }

    // extract values
    for (int i = 0; i < s.getLength(); ++i) {
        if (!s[i].isString()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s.getPath() + ": the drop values must be only composed of string [\"...\", ...]");
        }
        this->_pattern.emplace_back(static_cast<const char*>(s[i]));
    }
}

_hot void forge::rules::DropRule::format(_unused const std::string& bin, std::string& content)
{
    if (this->_pattern.empty()) return;

    // init vars
    std::vector<std::string> lines = splitLines(content);
    std::vector<std::string> result;
    result.reserve(lines.size());

    // drop any line matching at least one pattern
    for (const std::string& line : lines) {
        bool drop = false;

        for (const std::regex& pattern : this->_pattern) {
            if (std::regex_search(line, pattern)) {
                drop = true;
                break;
            }
        }

        if (!drop)
            result.push_back(line);
    }

    // store the result
    content = joinLines(result);
}
