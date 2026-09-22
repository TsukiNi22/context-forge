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
##  @file ReplaceRule.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef REPLACERULE_H
    #define REPLACERULE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "IRule.hpp"        // forge::rules::IRule
    #include <libconfig.h++>    // libconfig::Setting
    #include <regex>            // std::regex
    #include <string>           // std::string
    #include <vector>           // std::vector

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class ReplaceRule: public forge::rules::IRule {
    private:
        std::vector<std::regex> _match;
        std::vector<std::string> _eq;
        std::string _by;

    public:
        // ---------- -Function -------- //
        void load(const libconfig::Setting& s) final;
        void format(const std::string& bin, std::string& content) final;

        // ------------ Operator ---------- //
        ReplaceRule& operator=(const ReplaceRule& other) = delete;
        ReplaceRule& operator=(ReplaceRule&& other) = delete;

        // ---------- Constructor --------- //
        ReplaceRule(): IRule("replace") {};
        ReplaceRule(const ReplaceRule& other) = delete;
        ReplaceRule(ReplaceRule&& other) = delete;

        // ----------- Destructor --------- //
        ~ReplaceRule() = default;
};

} // namespace end
#endif /* REPLACERULE_H */
