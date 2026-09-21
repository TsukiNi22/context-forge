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
##  @file Forge-init.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Verbose
#define _Arguments
#include <utils/utils.hpp>
#include "forge/Forge.hpp"
#include <fcntl.h>
#include <charconv>
#include <optional>
#include <string>

_nodiscard static std::optional<std::string> VerboseParsingHook(const std::string& option)
{
    if (option == "none" || option == "basic" || option == "advanced" || option == "debug") return std::nullopt;
    return "Invalid verbose level, should be (none|basic|advanced|debug), but got: " + option;
}

_nodiscard static std::optional<std::string> ModeParsingHook(const std::string& option)
{
    if (option == "local" || option == "dist") return std::nullopt;
    return "Invalid mode, should be (local|dist), but got: " + option;
}

_nodiscard static std::optional<std::string> FdParsingHook(const std::string& option)
{
    int fd = 0;
    auto [ptr, ec] = std::from_chars(option.data(), option.data() + option.size(), fd);
    if (ec != std::errc() || ptr != option.data() + option.size()) return "Invalid fd, expected an integer, but got: " + option;
    if (fd < 0) return "Invalid fd, must be non-negative, but got: " + option;
    if (::fcntl(fd, F_GETFD) == -1) return "Invalid fd, not an open file descriptor: " + option;
    return std::nullopt;
}

void forge::Forge::init(int argc, const char *const argv[])
{
    utils::arguments::ArgParser parser = utils::arguments::ArgParser("context-forge", "A warpper to forge the output of many thing into what you really want!");

    // Setup the usages
    parser.setUsage("context-forge_check",
        "check",
        false,
        {
            {"check", true},
            {"mode", true},
            {"plugins", false},
            {"rules", false},
            {"recursive", false},
            {"ip", false},
            {"port", false},
            {"model", false},
            {"system-prompt", false},
            {"verbose", false},
        },
        "Check the loading of rules & ollama"
    );
    parser.setUsage("context-forge_setup",
        "setup",
        false,
        {
            {"setup", true},
            {"copy", false},
            {"plugins", false},
            {"rules", false},
            {"recursive", false},
            {"ip", false},
            {"port", false},
            {"model", false},
            {"system-prompt", false},
            {"verbose", false},
        },
        "Setup daemon and other things"
    );
    parser.setUsage("context-forge_stop",
        "stop",
        false,
        {
            {"stop", true},
            {"verbose", false},
        },
        "Stop the server"
    );
    parser.setUsage("context-forge_start",
        "start",
        false,
        {
            {"start", true},
            {"verbose", false},
        },
        "Start the server"
    );
    parser.setUsage("context-forge_restart",
        "restart",
        false,
        {
            {"restart", true},
            {"verbose", false},
        },
        "Restart the server"
    );
    parser.setUsage("context-forge_status",
        "status",
        false,
        {
            {"status", true},
            {"ip", false},
            {"port", false},
        },
        "Display global status"
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
    parser.setUsage("context-forge_pull",
        "pull",
        true,
        {
            {"pull", true},
            {"model-name", true},
        },
        "Launch the pull of an ollama model"
    );
    parser.setUsage("context-forge_exec",
        "exec",
        false,
        {
            {"exec", true},
            {"verbose", false},
            {"redirect", false},
            {"command", true},
        },
        "Client usage, start a warpper around the given commands, communicate with the server to establish connection with other service"
    );
    parser.setUsage("context-forge_server",
        "server",
        false,
        {
            {"server", true},
            {"plugins", false},
            {"rules", false},
            {"recursive", false},
            {"ip", false},
            {"port", false},
            {"model", false},
            {"system-prompt", false},
            {"verbose", false},
        },
        "Server usage, launch the server mode, allow the client to properly run and communicate with other service"
    );

    // Setup the option
    parser.setOption("check",
        "check",
        "Switch to the check mode"
    );
    parser.setOption("mode",
        "mode",
        ModeParsingHook,
        "Select the mode to use for the check"
    );
    parser.setOption("setup",
        "setup",
        "Switch to the setup mode"
    );
    parser.setOption("stop",
        "stop",
        "Stop the server"
    );
    parser.setOption("start",
        "start",
        "Start the server"
    );
    parser.setOption("restart",
        "restart",
        "Restart the server"
    );
    parser.setOption("status",
        "status",
        "Display global status"
    );
    parser.setOption("remove",
        "remove",
        "Switch to the remove mode"
    );
    parser.setOption("install-ollama",
        "install-ollama",
        "Switch to the installation of ollama"
    );
    parser.setOption("pull",
        "pull",
        "Pull a model for ollama"
    );
    parser.setOption("model-name",
        "model",
        utils::arguments::defaultTrueParsingHook,
        "Model to pull"
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
    parser.setFlag("redirect",
        {"d", "fd", "redirect", ""},
        {
            {"fd", true, FdParsingHook}
        },
        "The file descriptor to redirect (default: stderr)"
    );
    parser.setFlag("copy",
        {"c", "cp", "copy", ""},
        {},
        "Enable the copy of the file to a internal place during setup (default: disable)"
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
    parser.setFlag("plugins",
        {"P", "so", "plugins", ""},
        {
            {"path", true, utils::arguments::defaultDirectoryParsingHook}
        },
        "Path of the directory for the rules's shared object (plugins) used to edit the context (<path>/*.so)"
    );
    parser.setFlag("rules",
        {"r", "rules", "rules", ""},
        {
            {"path", true, utils::arguments::defaultDirectoryParsingHook}
        },
        "Path of the directory for the rules used to edit the context (<path>/*.cfg)"
    );
    parser.setFlag("recursive",
        {"R", "rec", "recursive", ""},
        {},
        "Enable the recursive search for rules/plugins in the given directorys"
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
            {"port", true, utils::arguments::defaultSizetParsingHook}
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
        {"s", "", "system-prompt", "CONTEXT_FORGE_SYSTEM_PROMPT"},
        {
            {"path", true, utils::arguments::defaultFileParsingHook}
        },
        "Path for the system-prompt used for the ollama model"
    );

    // Extract settings
    utils::arguments::ParsedUsages usages = parser.parse(argc, argv, true);
    for (const auto& [id, type, options]: usages.front().arguments) {
        const std::string& value = (options.empty() ? "" : options.front());
        if (type) {
            if (id == "model-name") this->_settings.add("model", value);
            else if (id == "mode") this->_settings.add("check-mode", value);
            else this->_settings.add("mode", value); // detect mode from first options
        } else if (id == "verbose") {
            this->_settings.add("verbose", value);
            if      (value == "none")     set_verbose(None)
            else if (value == "basic")    set_verbose(Basic)
            else if (value == "advanced") set_verbose(Advanced)
            else if (value == "debug")    set_verbose(Debug)
        }
        else if (id == "redirect") this->_settings.cast<utils::arguments::CastType::Int32>("redirect", value);
        else if (id == "plugins") this->_settings.add("plugins", value);
        else if (id == "rules") this->_settings.add("rules", value);
        else if (id == "recursive") this->_settings.add("recursive", true);
        else if (id == "copy") this->_settings.add("copy", true);
        else if (id == "ip") this->_settings.add("ip", value);
        else if (id == "port") this->_settings.cast<utils::arguments::CastType::UInt16>("port", value);
        else if (id == "model") this->_settings.add("model", value);
        else if (id == "system-prompt") this->_settings.add("system-prompt", value);
        else if (id == "command") {
            this->_bin = value;
            this->_args.assign(options.begin() + 1, options.end());
        }
    }
    onDebugVerbose("Selected mode [" << (std::string)this->_settings.at("mode") << "]");
}
