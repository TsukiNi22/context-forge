/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 06/09/2026 by @author Tsukini

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
    #define _Arguments
    #include <utils/utils.hpp>  // utils::arguments::Settings
    #include <vector>           // std::vector
    #include <string>           // std::string

namespace forge { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Forge {
    private:
        /* arguments */
        utils::arguments::Settings _settings;
        std::string _bin;
        std::vector<std::string> _args;

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
