/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 21/09/2026 by @author Tsukini

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
    #include <utils/utils.hpp>  // utils::arguments::Settings, utils::encapsulation::Status, utils::encapsulation::SharedObject
    #include "rules/Rules.hpp"  // forge::rules::Rules   
    #include "Ollama.hpp"       // forge::Ollama
    #include <sys/types.h>      // pid_t
    #include <unordered_map>    // std::unordered_map
    #include <variant>          // std::variant
    #include <cstddef>          // std::size_t, std::byte
    #include <memory>           // std::unique_ptr
    #include <vector>           // std::vector
    #include <string>           // std::string

    //----------------------------------------------------------------//
    /* DEFINE */

    /* const */
    #define CHANNEL_SIZE (sizeof(std::byte) * 4096) // aprox of command outpiut mean size
    #define CHANNEL_NUMBER 10 // around 10~ process at once should not cause problems
    #define SHM_NAME "context-forge:"

    /* default */
    #define OLLAMA_DEFAULT_MODEL "qwen2.5-coder:1.5b"
    #define OLLAMA_DEFAULT_IP "localhost"
    #define OLLAMA_DEFAULT_PORT 11434

    /* type */
    #define TYPE_TRIGGER 0
    #define TYPE_PRE_RULE 1
    #define TYPE_RULE 2

namespace forge { // namespace start
//----------------------------------------------------------------//
/* CLASS */

using PluginFactory = std::variant<
    forge::rules::ITrigger* (*)(), // TYPE_TRIGGER, index = 0
    forge::rules::IPreRule* (*)(), // TYPE_PRE_RULE, index = 1
    forge::rules::IRule* (*)()     // TYPE_RULE, index = 2
>;

template<std::size_t Index>
using FactoryType = std::variant_alternative_t<Index, forge::PluginFactory>;

using TriggerFactory = forge::FactoryType<TYPE_TRIGGER>;
using PreRuleFactory = forge::FactoryType<TYPE_PRE_RULE>;
using RuleFactory = forge::FactoryType<TYPE_RULE>;

class Forge {
    private:
        /* arguments */
        utils::arguments::Settings _settings;

        /* server execution */
        std::unordered_map<std::string, std::pair<forge::PluginFactory, utils::encapsulation::SharedObject>> _plugins;
        std::vector<forge::rules::Rules> _rules;
        forge::Ollama _ollama;
        bool _llm = true;

        /* client execution */
        std::string _bin;
        std::vector<std::string> _args;
        utils::encapsulation::Status _status; // process exit status after exec

        // ---------- Pre-Function -------- //
        /* client (~failsafe) */
        void fallback(void); // replace actual process by the one to wrap without anyhting
        pid_t getServerPid(void); // call fallback or return the pid_t of the running server

        /* server */
        void load(void); // load rules/sysprompt (only loaded one time at init)
        void loadPlugin(const std::string& path);
        void loadCFG(const std::string& path);
        void loadLLM(void);
        void formatCFG(const std::string& bin, std::string& input) const;
        void formatLLM(const std::string& bin, std::string& input) const;

        /* dispatch */
        void setup(void);
        void stop(void);
        void start(void);
        void restart(void);
        void status(void);
        void remove(void);
        void install(void);
        void pull(void);
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
