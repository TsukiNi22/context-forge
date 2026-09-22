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
##  @file InsertRule.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef INSERTRULE_H
    #define INSERTRULE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "IRule.hpp"        // forge::rules::IRule
    #include <libconfig.h++>    // libconfig::Setting
    #include <optional>         // std::optional
    #include <string>           // std::string

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class InsertRule: public forge::rules::IRule {
    private:
        std::optional<std::string> _before;
        std::optional<std::string> _after;

    public:
        // ---------- -Function -------- //
        void load(const libconfig::Setting& s) final;
        void format(const std::string& bin, std::string& content) final;

        // ------------ Operator ---------- //
        InsertRule& operator=(const InsertRule& other) = delete;
        InsertRule& operator=(InsertRule&& other) = delete;

        // ---------- Constructor --------- //
        InsertRule(): IRule("insert") {};
        InsertRule(const InsertRule& other) = delete;
        InsertRule(InsertRule&& other) = delete;

        // ----------- Destructor --------- //
        ~InsertRule() = default;
};

} // namespace end
#endif /* INSERTRULE_H */
