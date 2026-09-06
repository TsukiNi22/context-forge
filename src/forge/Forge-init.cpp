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
##  @file Forge-init.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Verbose
#define _Arguments
#include <utils/utils.hpp>
#include "forge/Forge.hpp"
#include <optional>
#include <string>

_nodiscard static std::optional<std::string> VerboseParsingHook(const std::string& option)
{
    if (option == "none" || option == "basic" || option == "advanced" || option == "debug") return std::nullopt;
    return "Invalid verbose level, should be (none|basic|advanced|debug), but got: " + option;
}

void forge::Forge::init(int argc, const char *const argv[])
{
    utils::arguments::ArgParser parser = utils::arguments::ArgParser("context-forge", "A warpper to forge the output of many thing into what you really want!");

    // Setup the usages
    parser.setUsage("context-forge_setup",
        "setup",
        false,
        {
            {"setup", true},
            {"verbose", false},
        },
        "Setup daemon and other things"
    );
    parser.setUsage("context-forge_remove",
        "remove",
        false,
        {
            {"remove", true},
            {"verbose", false},
        },
        "Remove potential data that where setup using the setup usage"
    );
    parser.setUsage("context-forge_install-ollama",
        "install-ollama",
        false,
        {
            {"install-ollama", true},
            {"verbose", false},
        },
        "Launch the ollama installation script"
    );
    parser.setUsage("context-forge_exec",
        "exec",
        false,
        {
            {"exec", true},
            {"verbose", false},
            {"command", true},
        },
        "Client usage, start a warpper around the given commands, communicate with the server to establish connection with other service"
    );
    parser.setUsage("context-forge_server",
        "server",
        false,
        {
            {"server", true},
            {"rules", false},
            {"ip", false},
            {"port", false},
            {"model", false},
            {"system-prompt", false},
            {"verbose", false},
        },
        "Server usage, launch the server mode, allow the client to properly run and communicate with other service"
    );

    // Setup the option
    parser.setOption("setup",
        "setup",
        "Switch to the setup mode"
    );
    parser.setOption("remove",
        "remove",
        "Switch to the remove mode"
    );
    parser.setOption("install-ollama",
        "install-ollama",
        "Switch to the installation of ollama"
    );
    parser.setOption("exec",
        "exec",
        "Switch to the client/execution mode"
    );
    parser.setOption("server",
        "server",
        "Switch to the server mode"
    );

    // Setup the flags
    parser.setFlag("verbose",
        {"v", "", "verbose", "VERBOSE"},
        {
            {"level", true, VerboseParsingHook}
        },
        "Set the verbose level none|basic|advanced|debug (default: basic)"
    );
    parser.setFlag("command",
        {"c", "cmd", "command", ""},
        {
            {"bin", true, utils::arguments::defaultTrueParsingHook},
            {"argument", false, utils::arguments::defaultTrueParsingHook}
        },
        "Command to exec during the client runtime",
        true
    );
    parser.setFlag("rules",
        {"r", "rules", "rules", ""},
        {
            {"path", true, utils::arguments::defaultFileParsingHook}
        },
        "Path for the rules used to edit the context"
    );
    parser.setFlag("ip",
        {"a", "ip", "ip", "CONTEXT_FORGE_HOST_IPV4"},
        {
            {"ip", true, utils::arguments::defaultTrueParsingHook}
        },
        "Ip used to connect to ollama"
    );
    parser.setFlag("port",
        {"p", "port", "port", "CONTEXT_FORGE_HOST_PORT"},
        {
            {"port", true, utils::arguments::defaultTrueParsingHook}
        },
        "Port used to connect to ollama"
    );
    parser.setFlag("model",
        {"m", "model", "model", "CONTEXT_FORGE_MODEL"},
        {
            {"path", true, utils::arguments::defaultTrueParsingHook}
        },
        "Model that will be used by ollama"
    );
    parser.setFlag("system-prompt",
        {"s", "", "system-prompt", ""},
        {
            {"path", true, utils::arguments::defaultFileParsingHook}
        },
        "Path for the system-prompt used for the ollama model"
    );

    // Extract settings
    utils::arguments::ParsedUsages usages = parser.parse(argc, argv);
    for (const auto& [id, type, options]: usages.front().arguments) {
        const std::string& value = (options.empty() ? "" : options.front());
        if (type) this->_settings.add("mode", value); // detect mode from first options
        else if (id == "verbose") {
            if      (value == "none")     set_verbose(None)
            else if (value == "basic")    set_verbose(Basic)
            else if (value == "advanced") set_verbose(Advanced)
            else if (value == "debug")    set_verbose(Debug)
        }
        else if (id == "rules") this->_settings.add("rules", value);
        else if (id == "ip") this->_settings.add("ip", value);
        else if (id == "port") this->_settings.cast<utils::arguments::CastType::UInt16>("port", value);
        else if (id == "model") this->_settings.add("model", value);
        else if (id == "system-prompt") this->_settings.add("system-prompt", value);
        else if (id == "command") {
            this->_bin = value;
            this->_args = options;
        }
    }
    onDebugVerbose("Selected mode [" << (std::string)this->_settings.at("mode") << "]");
}
