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
##  @file IInstruction.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef IINSTRUCTION_H
    #define IINSTRUCTION_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include <utils/security/observer/Observer.hpp> // utils::security::observer::Observer
    #include <libconfig.h++>                        // libconfig::Setting
    #include <string>                               // std::string

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

// class that will probably be in shared object (.so) -> apply observer for potential memory leak
class IInstruction: private utils::security::observer::Observer<"IInstruction"> {
    public:
        // ---------- Pre-Function -------- //
        virtual std::string name(void) const; // used only for debug purpose on server side
        virtual void load(const libconfig::Setting& s);

        // ------------ Operator ---------- //
        IInstruction& operator=(const IInstruction& other) = delete;
        IInstruction& operator=(IInstruction&& other) = delete;

        // ---------- Constructor --------- //
        IInstruction() = default;
        IInstruction(const IInstruction& other) = delete;
        IInstruction(IInstruction&& other) = delete;

        // ----------- Destructor --------- //
        virtual ~IInstruction() = default;
};

} // namespace end
#endif /* IINSTRUCTION_H */
