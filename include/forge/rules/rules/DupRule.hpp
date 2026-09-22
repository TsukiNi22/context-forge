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
##  @file DupRule.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef DUPRULE_H
    #define DUPRULE_H

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

class DupRule: public forge::rules::IRule {
    private:
        std::vector<std::regex> _match;
        std::vector<std::string> _eq;
        int _keep = 1;
        bool _invert = false;

    public:
        // ---------- -Function -------- //
        void load(const libconfig::Setting& s) final;
        void format(const std::string& bin, std::string& content) final;

        // ------------ Operator ---------- //
        DupRule& operator=(const DupRule& other) = delete;
        DupRule& operator=(DupRule&& other) = delete;

        // ---------- Constructor --------- //
        DupRule() = default;
        DupRule(const DupRule& other) = delete;
        DupRule(DupRule&& other) = delete;

        // ----------- Destructor --------- //
        ~DupRule() = default;
};

} // namespace end
#endif /* DUPRULE_H */
