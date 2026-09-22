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
#include "forge/rules/rules/ReplaceRule.hpp"
#include <libconfig.h++>
#include <regex>
#include <string>
#include <vector>

void forge::rules::ReplaceRule::load(const libconfig::Setting& s)
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

    // by
    if (!s.exists("by")) _unlikely {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s.getPath() + ": the by value is required");
    }
    if (s["by"].getType() != libconfig::Setting::TypeString) _unlikely {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["by"].getPath() + ": the by value must be a string");
    }
    this->_by = static_cast<const char*>(s["by"]);
}

_hot void forge::rules::ReplaceRule::format(_unused const std::string& bin, std::string& content)
{
    static const std::string TOKEN = "<INSERT>";

    // regex-based replacements
    for (const std::regex& pattern : this->_match) {
        std::string result;
        result.reserve(content.size());

        auto begin = std::sregex_iterator(content.begin(), content.end(), pattern);
        auto end = std::sregex_iterator();
        std::size_t last = 0;

        for (auto it = begin; it != end; ++it) {
            const std::smatch& m = *it;
            result += content.substr(last, static_cast<std::size_t>(m.position()) - last);

            std::string by = this->_by;
            std::size_t pos = by.find(TOKEN);
            if (pos != std::string::npos)
                by.replace(pos, TOKEN.size(), m.str());
            result += by;

            last = static_cast<std::size_t>(m.position() + m.length());
        }
        result += content.substr(last);
        content = std::move(result);
    }

    // exact string replacements
    for (const std::string& eq : this->_eq) {
        std::string by = this->_by;
        std::size_t token = by.find(TOKEN);
        if (token != std::string::npos)
            by.replace(token, TOKEN.size(), eq);

        std::size_t pos = 0;
        while ((pos = content.find(eq, pos)) != std::string::npos) {
            content.replace(pos, eq.size(), by);
            pos += by.size();
        }
    }
}
