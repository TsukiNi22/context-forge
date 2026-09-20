/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/09/2026 by @author Tsukini

File Name:
##  @file Rules.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef RULES_H
    #define RULES_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #define _Attribute
    #include <utils/utils.hpp>          // _hot, _nodiscard
    #include "triggers/ITrigger.hpp"    // forge::rules::ITrigger
    #include "pre-rules/IPreRule.hpp"   // forge::rules::IPreRule
    #include "rules/IRule.hpp"          // forge::rules::IRule
    #include <libconfig.h++>            // libconfig::Setting
    #include <memory>                   // std::unique_ptr
    #include <vector>                   // std::vector
    #include <string>                   // std::string

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Rules {
    private:
        std::vector<std::unique_ptr<forge::rules::ITrigger>> _triggers; // no trigger == always true
        std::vector<std::unique_ptr<forge::rules::IPreRule>> _pre; // optional step
        std::vector<std::unique_ptr<forge::rules::IRule>> _rules;

    public:
        // ---------- Pre-Function -------- //
        void loadBlock(const libconfig::Setting& s);
        bool trigger(const std::string& bin, const std::string& content) const;
        void apply(const std::string& bin, std::string& content) const;

        // ------------ Function ---------- //
        _cold void push(std::unique_ptr<forge::rules::ITrigger>&& trigger) {this->_triggers.emplace_back(std::move(trigger));};
        _cold void push(std::unique_ptr<forge::rules::IPreRule>&& pre) {this->_pre.emplace_back(std::move(pre));};
        _cold void push(std::unique_ptr<forge::rules::IRule>&& rule) {this->_rules.emplace_back(std::move(rule));};

        // ------------ Operator ---------- //
        Rules& operator=(const Rules& other) = delete;
        Rules& operator=(Rules&& other);

        // ---------- Constructor --------- //
        Rules() = default;
        Rules(const Rules& other) = delete;
        Rules(Rules&& other);

        // ----------- Destructor --------- //
        ~Rules() = default;
};

} // namespace end
#endif /* RULES_H */
