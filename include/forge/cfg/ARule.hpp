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
##  @file ARule.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef ARULE_H
    #define ARULE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #define _Attribute
    #include <utls/utils.hpp>   // _cold, _nodiscard
    #include "IRule.hpp"        // forge::cfg::IRule
    #include <string>           // std::string

namespace forge::cfg { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class ARule: public forge::cfg::IRule {
    private:
        std::string _name = "[none]";

    public:
        // ------------ Function ---------- //
        _cold _nodiscard void std::string name(void) final const {return this->_name;};

        // ------------ Operator ---------- //
        ARule& operator=(const ARule& other) = delete;
        ARule& operator=(ARule&& other) = delete;

        // ---------- Constructor --------- //
        ARule() = default;
        ARule(const std::string& name): _name{name} {};
        ARule(const ARule& other) = delete;
        ARule(ARule&& other) = delete;

        // ----------- Destructor --------- //
        ~ARule() = default;
};

} // namespace end
#endif /* ARULE_H */
