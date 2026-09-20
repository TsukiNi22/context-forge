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
##  @file Ollama.hpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#ifndef OLLAMA_H
    #define OLLAMA_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #define _Network
    #define _System
    #define _Attribute
    #include <utils/utils.hpp>  // _cold, utils::system::Scheduler, utils::network::Address
    #include <httplib.h>        // httplib::Client
    #include <optional>         // std::optional
    #include <string>           // std::string

namespace forge { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class Ollama {
    private:
        mutable std::optional<httplib::Client> _cli;
        mutable utils::system::Scheduler _scheduler; // allow the ollama server to be kept alive
        std::string _model;
        std::string _sysprompt;
        bool _init = false;

        // ---------- Pre-Function -------- //
        void model_(void) const; // check if model exist
        void awake(void) const; // task scheduled to keep awake ollama model

    public:
        // ---------- Pre-Function -------- //
        void init(const utils::network::Address& addr, const std::string& model, const std::string& sysprompt);
        std::string prompt(const std::string& prompt) const;

        /* scheduler handling */
        void up(void);
        void down(void);

        // ------------ Operator ---------- //
        Ollama& operator=(const Ollama& other) = delete;
        Ollama& operator=(Ollama&& other) = delete;

        // ---------- Constructor --------- //
        Ollama() = default;
        Ollama(const Ollama& other) = delete;
        Ollama(Ollama&& other) = delete;

        // ----------- Destructor --------- //
        ~Ollama() {this->down();};
};

} // namespace end
#endif /* OLLAMA_H */
