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
##  @file AnsiPreRule.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef ANSIPRERULE_H
    #define ANSIPRERULE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "IPreRule.hpp"     // forge::rules::IPreRule
    #include <libconfig.h++>    // libconfig::Setting
    #include <string>           // std::string

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class AnsiPreRule: public forge::rules::IPreRule {
    private:
        bool _enable = true;

    public:
        // ---------- Pre-Function -------- //
        void load(const libconfig::Setting& s) final;
        void format(const std::string& bin, std::string& content) final;

        // ------------ Operator ---------- //
        AnsiPreRule& operator=(const AnsiPreRule& other) = delete;
        AnsiPreRule& operator=(AnsiPreRule&& other) = delete;

        // ---------- Constructor --------- //
        AnsiPreRule(): IPreRule("ansi") {};
        AnsiPreRule(const AnsiPreRule& other) = delete;
        AnsiPreRule(AnsiPreRule&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~AnsiPreRule() = default;
};

} // namespace end
#endif /* ANSIPRERULE_H */
