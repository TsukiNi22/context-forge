/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/09/2026 by @author Tsukini

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
    #include <utils/utils.hpp>  // _hot, _nodiscard
    #include "IRule.hpp"        // forge::cfg::IRule
    #include <unordered_map>    // std::unordered_map
    #include <memory>           // std::unique_ptr
    #include <vector>           // std::vector
    #include <string>           // std::string

namespace forge::cfg { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Rules {
    private:
        const std::unordered_map<std::string, std::unique_ptr<forge::cfg::IRule>>& _rule;
        std::vector<std::string> _rules;

    public:
        // ---------- Pre-Function -------- //
        _hot _nodiscard bool trigger(const std::string& bin, const std::string& content) const;
        _hot void apply(std::string& content) const;

        // ------------ Function ---------- //
        _cold void push(const std::string& name) {this->_rules.push_back(name);};

        // ------------ Operator ---------- //
        Rules& operator=(const Rules& other) = delete;
        Rules& operator=(Rules&& other) = delete;

        // ---------- Constructor --------- //
        Rules(const std::unordered_map<std::string, std::unique_ptr<forge::cfg::IRule>>& rule): _rule{rule} {};
        Rules(const Rules& other) = delete;
        Rules(Rules&& other) = delete;

        // ----------- Destructor --------- //
        ~Rules() = default;
};

} // namespace end
#endif /* RULES_H */
