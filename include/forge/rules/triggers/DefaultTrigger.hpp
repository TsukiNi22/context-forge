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
    #include <string>           // std::string

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class DefaultTrigger: public forge::rules::ITrigger {
    public:
        // ---------- Pre-Function -------- //
        void load(const libconfig::Setting& s) final;
        bool trigger(const std::string& bin, const std::string& content) final;

        // ------------ Operator ---------- //
        DefaultTrigger& operator=(const DefaultTrigger& other) = delete;
        DefaultTrigger& operator=(DefaultTrigger&& other) = delete;

        // ---------- Constructor --------- //
        DefaultTrigger() = default;
        DefaultTrigger(const DefaultTrigger& other) = delete;
        DefaultTrigger(DefaultTrigger&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~DefaultTrigger() = default;
};

} // namespace end
#endif /* DEFAULTTRIGGER_H */
