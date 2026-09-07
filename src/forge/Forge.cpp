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
##  @file Forge.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#define _Exception
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
}

void forge::Forge::exec(void)
{
}

void forge::Forge::server(void)
{
}
