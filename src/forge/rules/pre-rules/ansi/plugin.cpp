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

#define _IOManip
#define _Exception
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/pre-rules/AnsiPreRule.hpp"
#include <libconfig.h++>
#include <string>

void forge::rules::AnsiPreRule::load(const libconfig::Setting& s)
{
    // check type
    if (s.getType() != libconfig::Setting::TypeBoolean) _unlikely {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s.getPath() + ": the ansi value must be a boolean");
    }

    // extract values
    this->_enable = static_cast<bool>(s);
}

_hot void forge::rules::AnsiPreRule::format(_unused const std::string& bin, std::string& content)
{
    if (this->_enable) return;

    // init vars
    std::string result;
    result.reserve(content.size());

    // fin any esc and check for the sequence after it
    for (size_t i = 0; i < content.size(); ++i) {
        if (content[i] != static_cast<char>(utils::iomanip::Char::ESC)) {
            result += content[i];
            continue;
        }

        // Look for after escape
        if (i + 1 >= content.size())
            break; // if there is nothing (last char of the string)

        // Fin the end of the ansi sequence
        char next = content[i + 1];
        if (next == '[') {
            std::size_t j = i + 2;
            while (j < content.size() && !(content[j] >= '@' && content[j] <= '~'))
                ++j;
            i = j;
        } else if (next == ']') {
            std::size_t j = i + 2;
            while (j < content.size() && content[j] != static_cast<char>(utils::iomanip::Char::BEL) && !(content[j] == static_cast<char>(utils::iomanip::Char::ESC) && j + 1 < content.size() && content[j + 1] == '\\'))
                ++j;
            if (j < content.size() && content[j] == static_cast<char>(utils::iomanip::Char::BEL))
                i = j;
            else if (j + 1 < content.size())
                i = j + 1;
            else
                i = j;
        } else {
            i += 1;
        }
    }

    // store the result
    content = std::move(result);
}
