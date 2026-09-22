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
##  @file ITrigger.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef ITRIGGER_H
    #define ITRIGGER_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "../AInstruction.hpp"  // forge::rules::AInstruction
    #include <string>               // std::string

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class ITrigger: public forge::rules::AInstruction {
    public:
        // ---------- Pre-Function -------- //
        virtual bool trigger(const std::string& bin, const std::string& content) = 0;

        // ------------ Operator ---------- //
        ITrigger& operator=(const ITrigger& other) = delete;
        ITrigger& operator=(ITrigger&& other) = delete;

        // ---------- Constructor --------- //
        ITrigger() = default;
        ITrigger(const std::string& name): AInstruction(name) {};
        ITrigger(const ITrigger& other) = delete;
        ITrigger(ITrigger&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~ITrigger() = default;
};

} // namespace end
#endif /* ITRIGGER_H */
