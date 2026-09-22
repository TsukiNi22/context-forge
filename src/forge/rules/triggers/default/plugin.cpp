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
#include "forge/rules/triggers/DefaultTrigger.hpp"
#include <libconfig.h++>
#include <algorithm>
#include <string>
#include <regex>

void forge::rules::DefaultTrigger::load(const libconfig::Setting& s)
{
    // bin
    if (s.exists("bin")) {
        // check type
        const libconfig::Setting& bin = s["bin"];
        if (!bin.isArray()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["bin"].getPath() + ": the bin value must be an array of string [\"...\", ...]");
        }

        // extract values
        for (int i = 0; i < bin.getLength(); ++i) {
            if (!bin[i].isString()) _unlikely {
                throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["bin"].getPath() + ": the bin values must be only composed of string [\"...\", ...]");
            }
            this->_bin.push_back(static_cast<const char*>(bin[i]));
        }
    }

    // contains
    if (s.exists("contains")) {
        // check type
        const libconfig::Setting& contains = s["contains"];
        if (!contains.isString()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["contains"].getPath() + ": the contains value must be a string");
        }

        // extract values
        this->_contains.emplace(static_cast<const char*>(contains));
    }

    // match
    if (s.exists("match")) {
        // check type
        const libconfig::Setting& match = s["match"];
        if (!match.isString()) _unlikely {
            throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["match"].getPath() + ": the match value must be a string");
        }

        // extract values
        this->_match.emplace(static_cast<const char*>(match));
    }
}

_hot _nodiscard bool forge::rules::DefaultTrigger::trigger(const std::string& bin, const std::string& content)
{
    return (!this->_bin.empty() && std::find(this->_bin.begin(), this->_bin.end(), bin) != this->_bin.end())
        || (this->_contains && std::regex_search(content, *this->_contains))
        || (this->_match && std::regex_match(content, *this->_match))
        || (this->_bin.empty() && !this->_contains && !this->_match)
    ;
}
