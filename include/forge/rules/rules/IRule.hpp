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
    #include "../AInstruction.hpp"  // forge::rules::AInstruction
    #include <string>               // std::string

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class IRule: public forge::rules::AInstruction {
    public:
        // ---------- -Function -------- //
        virtual void format(const std::string& bin, std::string& content) = 0;

        // ------------ Operator ---------- //
        IRule& operator=(const IRule& other) = delete;
        IRule& operator=(IRule&& other) = delete;

        // ---------- Constructor --------- //
        IRule() = default;
        IRule(const std::string& name): AInstruction(name) {};
        IRule(const IRule& other) = delete;
        IRule(IRule&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~IRule() = default;
};

} // namespace end
#endif /* IRULE_H */
