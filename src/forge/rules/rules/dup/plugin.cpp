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
#include "forge/rules/rules/DupRule.hpp"
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

void forge::rules::DupRule::load(const libconfig::Setting& s)
{
    // match
    if (s.exists("match")) {
        const libconfig::Setting& match = s["match"];
        if (!match.isArray()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["match"].getPath() + ": the match value must be an array of string [\"...\", ...]");
        }
        for (int i = 0; i < match.getLength(); ++i) {
            if (!match[i].isString()) _unlikely {
                throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["match"].getPath() + ": the match values must be only composed of string [\"...\", ...]");
            }
            this->_match.emplace_back(static_cast<const char*>(match[i]));
        }
    }

    // eq
    if (s.exists("eq")) {
        const libconfig::Setting& eq = s["eq"];
        if (!eq.isArray()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["eq"].getPath() + ": the eq value must be an array of string [\"...\", ...]");
        }
        for (int i = 0; i < eq.getLength(); ++i) {
            if (!eq[i].isString()) _unlikely {
                throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["eq"].getPath() + ": the eq values must be only composed of string [\"...\", ...]");
            }
            this->_eq.emplace_back(static_cast<const char*>(eq[i]));
        }
    }

    // keep
    if (s.exists("keep")) {
        if (s["keep"].getType() != libconfig::Setting::TypeInt) _unlikely {
            throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["keep"].getPath() + ": the keep value must be an integer");
        }
        this->_keep = static_cast<int>(s["keep"]);
    }

    // invert
    if (s.exists("invert")) {
        if (s["invert"].getType() != libconfig::Setting::TypeBoolean) _unlikely {
            throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["invert"].getPath() + ": the invert value must be a boolean");
        }
        this->_invert = static_cast<bool>(s["invert"]);
    }
}

_hot void forge::rules::DupRule::format(_unused const std::string& bin, std::string& content)
{
    if (this->_match.empty() && this->_eq.empty()) return;

    // init vars
    std::vector<std::string> lines = splitLines(content);
    std::size_t groups = this->_match.size() + this->_eq.size();

    // find which group (if any) a line belongs to, -1 if none
    auto groupOf = [&](const std::string& line) -> int {
        for (std::size_t i = 0; i < this->_match.size(); ++i) {
            if (std::regex_search(line, this->_match[i]))
                return static_cast<int>(i);
        }
        for (std::size_t i = 0; i < this->_eq.size(); ++i) {
            if (line == this->_eq[i])
                return static_cast<int>(this->_match.size() + i);
        }
        return -1;
    };

    // pre-count total matches per group (needed for invert)
    std::vector<int> total(groups, 0);
    std::vector<int> group(lines.size(), -1);
    for (std::size_t i = 0; i < lines.size(); ++i) {
        group[i] = groupOf(lines[i]);
        if (group[i] >= 0)
            ++total[static_cast<std::size_t>(group[i])];
    }

    // walk the lines and keep only the first (or last, if invert) `keep` matches per group
    std::vector<int> seen(groups, 0);
    std::vector<std::string> result;
    result.reserve(lines.size());

    for (std::size_t i = 0; i < lines.size(); ++i) {
        if (group[i] < 0) {
            result.push_back(lines[i]);
            continue;
        }

        std::size_t g = static_cast<std::size_t>(group[i]);
        ++seen[g];

        bool keepLine = this->_invert
            ? (seen[g] > total[g] - this->_keep) // keep the LAST `keep` matches
            : (seen[g] <= this->_keep);           // keep the FIRST `keep` matches

        if (keepLine)
            result.push_back(lines[i]);
    }

    // store the result
    content = joinLines(result);
}
