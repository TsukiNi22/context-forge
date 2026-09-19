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
    #define _Encapsulation
    #include <utils/utils.hpp>  // utils::arguments::Settings, utils::encapsulation::Status
    #include "cfg/IRule.hpp"    // forge::cfg::IRule
    #include "cfg/Rules.hpp"    // forge::cfg::Rules   
    #include <sys/types.h>      // pid_t
    #include <unordered_map>    // std::unordered_map
    #include <cstddef>          // std::byte
    #include <memory>           // std::unique_ptr
    #include <vector>           // std::vector
    #include <string>           // std::string

    //----------------------------------------------------------------//
    /* DEFINE */

    /* const */
    #define CHANNEL_SIZE (sizeof(std::byte) * 4096) // aprox of command outpiut mean size
    #define CHANNEL_NUMBER 10 // around 10~ process at once should not cause problems
    #define SHM_NAME "context-forge:"

namespace forge { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Forge {
    private:
        /* arguments */
        utils::arguments::Settings _settings;

        /* server execution */
        std::unordered_map<std::string, std::unique_ptr<forge::cfg::IRule>> _rule;
        std::vector<forge::cfg::Rules> _rules;

        /* client execution */
        std::string _bin;
        std::vector<std::string> _args;
        utils::encapsulation::Status _status;

        // ---------- Pre-Function -------- //
        /* client (~failsafe) */
        void fallback(void); // replace actual process by the one to wrap without anyhting
        pid_t getServerPid(void); // call fallback or return the pid_t of the running server

        /* server */
        void load(void); // load cfg/sysprompt (only loaded one time at init)
        void loadCFG(const std::string& path);
        void loadLLM(void);
        void formatCFG(std::string& input) const;
        void formatLLM(std::string& input) const;

        /* dispatch */
        void setup(void);
        void stop(void);
        void start(void);
        void restart(void);
        void status(void);
        void remove(void);
        void install(void);
        void exec(void);
        void server(void);

    public:
        // ---------- Pre-Function -------- //
        void init(int argc, const char *const argv[]);
        void run(void);
        int exit(void) const;

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
