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
##  @file DefaultTrigger.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef DEFAULTTRIGGER_H
    #define DEFAULTTRIGGER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "ITrigger.hpp"     // forge::rules::ITrigger
    #include <libconfig.h++>    // libconfig::Setting
    #include <optional>         // std::optional
    #include <vector>           // std::vector
    #include <string>           // std::string
    #include <regex>            // std::regex

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class DefaultTrigger: public forge::rules::ITrigger {
    private:
        std::vector<std::string> _bin;
        std::optional<std::regex> _contains;
        std::optional<std::regex> _match;

    public:
        // ---------- Pre-Function -------- //
        void load(const libconfig::Setting& s) final;
        bool trigger(const std::string& bin, const std::string& content) final;

        // ------------ Operator ---------- //
        DefaultTrigger& operator=(const DefaultTrigger& other) = delete;
        DefaultTrigger& operator=(DefaultTrigger&& other) = delete;

        // ---------- Constructor --------- //
        DefaultTrigger(): ITrigger("trigger") {};
        DefaultTrigger(const DefaultTrigger& other) = delete;
        DefaultTrigger(DefaultTrigger&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~DefaultTrigger() = default;
};

} // namespace end
#endif /* DEFAULTTRIGGER_H */
