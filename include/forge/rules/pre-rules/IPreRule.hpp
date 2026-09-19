/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/09/2026 by @author Tsukini

File Name:
##  @file IPreRule.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef IPRERULE_H
    #define IPRERULE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../AInstruction.hpp"  // forge::rules::AInstruction
    #include <string>               // std::string

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class IPreRule: public forge::rules::AInstruction {
    public:
        // ---------- Pre-Function -------- //
        virtual void format(const std::string& bin, std::string& content);

        // ------------ Operator ---------- //
        IPreRule& operator=(const IPreRule& other) = delete;
        IPreRule& operator=(IPreRule&& other) = delete;

        // ---------- Constructor --------- //
        IPreRule() = default;
        IPreRule(const IPreRule& other) = delete;
        IPreRule(IPreRule&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~IPreRule() = default;
};

} // namespace end
#endif /* IPRERULE_H */
