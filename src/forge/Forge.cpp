/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 14/09/2026 by @author Tsukini

File Name:
##  @file Forge.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#define _Exception
#define _Verbose
#define _Encapsulation
#include <utils/utils.hpp>
#include "forge/Forge.hpp"

void forge::Forge::run(void)
{
    // Dispatch to selected mode
    const std::string& mode = this->_settings.at("mode");
    if (mode == "setup") this->setup();
    else if (mode == "remove") this->remove();
    else if (mode == "install-ollama") this->install();
    else if (mode == "exec") this->exec();
    else if (mode == "server") this->server();
    else _unlikely {
        throw utils::exception::FatalException(utils::exception::ExternalCode::UnknownMode);
    }
}

void forge::Forge::setup(void)
{
}

void forge::Forge::remove(void)
{
}

void forge::Forge::install(void)
{
    // Check permission, should be executed has root
    if (::geteuid() != 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Ollama installation requires root privileges");

    // Spawn the sub-process that will install ollama
    onBasicVerbose("Starting ollama installation...");
    utils::encapsulation::Process proc;
    std::vector<std::string> args = {"-c", "curl -fsSL https://ollama.com/install.sh | sh"};
    proc.spawn("/bin/bash", args);

    // Wait until the end of the ollama installation process
    onBasicVerbose("Waiting for ollama installation to complete...");
    const utils::encapsulation::Status& status = proc.wait();

    // Check the result
    if (status.exited && status.code == 0) {
        onBasicVerbose("Ollama installed successfully!");
    } else {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, std::to_string(status.code));
    }
}

void forge::Forge::exec(void)
{
}

void forge::Forge::server(void)
{
}
