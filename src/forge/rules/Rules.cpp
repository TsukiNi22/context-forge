/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 21/09/2026 by @author Tsukini

File Name:
##  @file Rules.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Verbose
#define _Exception
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/Rules.hpp"
#include <libconfig.h++>
#include <cstddef>
#include <vector>
#include <string>

_cold void forge::rules::Rules::loadBlock(const libconfig::Setting& s)
{
    this->_enable = true;

    // ln & char
    if (s.exists("ln") && s.exists("char")) _unlikely {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, "Tshe hard-coded plugin 'block' doesn't allow ln & char value at the same time");
    }

    // ln
    if (s.exists("ln") && (!s.lookupValue("ln", this->_ln) || this->_ln <= 0)) _unlikely {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["ln"].getPath() + ": only a integer over 0 is allowed");
    }

    // char
    if (s.exists("char") && (!s.lookupValue("char", this->_char) || this->_char <= 0)) _unlikely {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["char"].getPath() + ": only a integer over 0 is allowed");
    }

    // show
    if (s.exists("show") && (!s.lookupValue("show", this->_show) || this->_show <= 0)) _unlikely {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["show"].getPath() + ": only a integer over 0 is allowed");
    }

    // sep
    if (s.exists("sep") && !s.lookupValue("sep", this->_sep)) _unlikely {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, s["sep"].getPath() + ": only a string is allowed");
    }
}

_hot _nodiscard bool forge::rules::Rules::trigger(const std::string& bin, const std::string& content) const
{
    // check if any trigger is valid
    for (const std::unique_ptr<forge::rules::ITrigger>& trigger: this->_triggers) {
        if (trigger->trigger(bin, content)) {
            onDebugVerbose("[trigger] " << trigger->name() << ": OK")
            return true;
        }
        onDebugVerbose("[trigger] " << trigger->name() << ": KO")
    }

    return (this->_triggers.size() == 0);
}

_hot _nodiscard static std::vector<std::string> split_ln(const std::string& content, const std::size_t n)
{
    std::vector<std::string> out;

    // find new line token and cut them each n new line
    std::size_t start = 0;
    std::size_t count = 0;
    for (std::size_t pos = content.find('\n'); pos != std::string::npos; pos = content.find('\n', pos + 1)) {
        if (++count == n) {
            out.emplace_back(content, start, pos - start);   // sans le '\n' final du bloc
            start = pos + 1;
            count = 0;
        }
    }

    // if some line/content are still left
    if (start < content.size())
        out.emplace_back(content, start);

    return out;
}

_hot _nodiscard static std::vector<std::string> split_char(const std::string& content, const std::size_t n)
{
    std::vector<std::string> out;

    // split every n char
    for (std::size_t i = 0; i < content.size(); i += n)
        out.emplace_back(content, i, n);

    return out;
}

_hot _nodiscard std::vector<std::string> forge::rules::Rules::applyBlock(const std::string& content) const
{
    std::vector<std::string> blocks;

    // only if the block plugin is enable
    if (!this->_enable) {
        blocks.push_back(content);
        return blocks;
    }

    // separate by ln|char
    if (this->_ln > 0) blocks = split_ln(content, this->_ln);
    else if (this->_char > 0) blocks = split_char(content, this->_char);
    else blocks.push_back(content);

    // keep only n block or all
    if (this->_show > 0 && blocks.size() > static_cast<std::size_t>(this->_show)) blocks.resize(this->_show);

    return blocks;
}

_hot void forge::rules::Rules::apply(const std::string& bin, std::string& content) const
{
    std::vector<std::string> blocks = this->applyBlock(content);
    content.clear();

    // for each block
    for (std::string& block: blocks) {
        // apply pre-rules
        for (const std::unique_ptr<forge::rules::IPreRule>& pre: this->_pre) {
            onDebugVerbose("[pre-rule] apply: " << pre->name());
            pre->format(bin, block);
        }

        // apply rules
        for (const std::unique_ptr<forge::rules::IRule>& rule: this->_rules) {
            onDebugVerbose("[rule] apply: " << rule->name());
            rule->format(bin, block);
        }
    }

    // get the size of the output to build
    std::size_t total = this->_sep.size() * (blocks.empty() ? 0 : blocks.size() - 1);
    for (const std::string& s: blocks) total += s.size();
    content.reserve(total);

    // build the output
    for (std::size_t i = 0; i < blocks.size(); ++i) {
        if (i != 0) _likely {content += this->_sep;}
        content += blocks[i];
    }
}
