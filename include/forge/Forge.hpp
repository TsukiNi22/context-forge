/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 16/07/2026 by @author Tsukini

File Name:
##  @file Forge.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef CORE_H
    #define CORE_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    //#include <iostream>

namespace forge { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Forge {
    private:
        /* Nothing */

    public:
        // ---------- Pre-Function -------- //
        void init(int argc, const char *const argv[]);
        void run(void);

        // ------------ Function ---------- //

        // ------------ Operator ---------- //
        Forge& operator=(const Forge& other) = delete;
        Forge& operator=(Forge&& other) = delete;

        // ---------- Constructor --------- //
        Forge() = default;
        Forge(const Forge& other) = delete;
        Forge(Forge&& other) = delete;

        // ----------- Destructor --------- //
        ~Forge() = default;
};

} // namespace end
#endif /* CORE_H */
