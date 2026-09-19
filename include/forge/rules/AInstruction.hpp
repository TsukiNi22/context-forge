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
##  @file AInstruction.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef AINSTRUCTION_H
    #define AINSTRUCTION_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #define _Attribute
    #include <utils/utils.hpp>  // _cold, _nodiscard
    #include "IInstruction.hpp" // forge::rules::IInstruction
    #include <string>           // std::string

namespace forge::rules { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class AInstruction: public forge::rules::IInstruction {
    private:
        // used only for debug purpose on server side
        std::string _name = "[none]";

    public:
        // ------------ Function ---------- //
        _cold _nodiscard std::string name(void) const final {return this->_name;};

        // ------------ Operator ---------- //
        AInstruction& operator=(const AInstruction& other) = delete;
        AInstruction& operator=(AInstruction&& other) = delete;

        // ---------- Constructor --------- //
        AInstruction() = default;
        AInstruction(const std::string& name): _name{name} {};
        AInstruction(const AInstruction& other) = delete;
        AInstruction(AInstruction&& other) = delete;

        // ----------- Destructor --------- //
        ~AInstruction() = default;
};

} // namespace end
#endif /* AINSTRUCTION_H */
