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
##  @file IRule.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef IRULE_H
    #define IRULE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <utils/security/observer/Observer.hpp> // utils::security::observer::Observer
    #include <libconfig.h++>                        // libconfig::Setting
    #include <string>                               // std::string

namespace forge::cfg { // namespace start
//----------------------------------------------------------------//
/* CLASS */

// class that will probably be in shared object (.so) -> apply observer for potential memory leak
class IRule: private utils::security::observer::Observer<"IRule"> {
    public:
        // ---------- Pre-Function -------- //
        virtual void std::string name(void) const;
        virtual void load(cons libconfig::Setting& s);
        virtual void format(std::string& s);

        // ------------ Operator ---------- //
        IRule& operator=(const IRule& other) = delete;
        IRule& operator=(IRule&& other) = delete;

        // ---------- Constructor --------- //
        IRule() = default;
        IRule(const IRule& other) = delete;
        IRule(IRule&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~IRule() = default;
};

} // namespace end
#endif /* IRULE_H */
