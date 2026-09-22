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
##  @file DropRule.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef DROPRULE_H
    #define DROPRULE_H

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

class DropRule: public forge::rules::IRule {
    private:
        std::vector<std::regex> _pattern;

    public:
        // ---------- -Function -------- //
        void load(const libconfig::Setting& s) final;
        void format(const std::string& bin, std::string& content) final;

        // ------------ Operator ---------- //
        DropRule& operator=(const DropRule& other) = delete;
        DropRule& operator=(DropRule&& other) = delete;

        // ---------- Constructor --------- //
        DropRule() = default;
        DropRule(const DropRule& other) = delete;
        DropRule(DropRule&& other) = delete;

        // ----------- Destructor --------- //
        ~DropRule() = default;
};

} // namespace end
#endif /* DROPRULE_H */
