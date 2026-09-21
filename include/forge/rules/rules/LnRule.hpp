/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 22/09/2026 by @author Tsukini

File Name:
##  @file LnRule.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef LNRULE_H
    #define LNRULE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "IRule.hpp"     // forge::rules::IRule
    #include <libconfig.h++>    // libconfig::Setting
    #include <string>           // std::string

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class LnRule: public forge::rules::IRule {
    public:
        // ---------- -Function -------- //
        void load(const libconfig::Setting& s) final;
        void format(const std::string& bin, std::string& content) final;

        // ------------ Operator ---------- //
        LnRule& operator=(const LnRule& other) = delete;
        LnRule& operator=(LnRule&& other) = delete;

        // ---------- Constructor --------- //
        LnRule() = default;
        LnRule(const LnRule& other) = delete;
        LnRule(LnRule&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~LnRule() = default;
};

} // namespace end
#endif /* LNRULE_H */
