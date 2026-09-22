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
#include "forge/rules/rules/InsertRule.hpp"
#include <libconfig.h++>
#include <string>

void forge::rules::InsertRule::load(const libconfig::Setting& s)
{
    // before
    if (s.exists("before")) {
        if (s["before"].getType() != libconfig::Setting::TypeString) _unlikely {
            throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["before"].getPath() + ": the before value must be a string");
        }
        this->_before = static_cast<const char*>(s["before"]);
    }

    // after
    if (s.exists("after")) {
        if (s["after"].getType() != libconfig::Setting::TypeString) _unlikely {
            throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["after"].getPath() + ": the after value must be a string");
        }
        this->_after = static_cast<const char*>(s["after"]);
    }
}

_hot void forge::rules::InsertRule::format(_unused const std::string& bin, std::string& content)
{
    if (this->_before) content = *this->_before + content;
    if (this->_after) content += *this->_after;
}
