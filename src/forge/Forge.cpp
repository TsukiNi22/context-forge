/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 17/09/2026 by @author Tsukini

File Name:
##  @file Forge.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Manip
#define _Attribute
#define _Exception
#define _Verbose
#define _Encapsulation
#include <utils/utils.hpp>
#include "forge/Forge.hpp"
#include <stdlib.h>
#include <string_view>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <array>

void forge::Forge::run(void)
{
    // Dispatch to selected mode
    if (!this->_settings.contains("mode"))
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "No mode specified, use (setup|remove|install-ollama|exec|server)");

    const std::string& mode = this->_settings.at("mode");
    if (mode == "setup") this->setup();
    else if (mode == "stop") this->stop();
    else if (mode == "start") this->start();
    else if (mode == "restart") this->restart();
    else if (mode == "status") this->status();
    else if (mode == "remove") this->remove();
    else if (mode == "install-ollama") this->install();
    else if (mode == "exec") this->exec();
    else if (mode == "server") this->server();
    else _unlikely {
        throw utils::exception::FatalException(utils::exception::ExternalCode::UnknownMode);
    }
}

static constexpr std::array<std::string_view, 2> BINARY_CANDIDATES = {
    "/usr/local/bin/context-forge", // installed via the repo (cmake)
    "/usr/bin/context-forge",       // installed via package (cpack)
};

_cold _nodiscard static std::string find_binary_path(void)
{
    for (const std::string_view& candidate: BINARY_CANDIDATES)
        if (std::filesystem::exists(candidate)) return std::string(candidate);
    throw utils::exception::ErrorException(utils::exception::ExternalCode::MissingBinary, "Can't find the context-forge binary in any known install location");
}

void forge::Forge::setup(void)
{
    onBasicVerbose("Starting setup of systemd user daemon...");

    // Get the service path
    std::string home = ::getenv("HOME");
    if (home.empty())
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "HOME environment variable not set");
    std::string systemd_dir = home + "/.config/systemd/user";
    std::string service_file_path = systemd_dir + "/context-forge.service";

    // Locate the installed binary
    onAdvancedVerbose("Locating the installed context-forge binary...");
    std::string binary_path = find_binary_path();
    onAdvancedVerbose("Binary found: " << binary_path);

    // Setup the service emplacement
    onBasicVerbose("Creating systemd directory: " << systemd_dir);
    utils::encapsulation::Process proc_mkdir;
    proc_mkdir.spawn("bash", {"-c", "mkdir -p " + systemd_dir});
    proc_mkdir.wait();

    // Build the service content
    std::ostringstream service_content;
    service_content << "[Unit]" << std::endl;
    service_content << "Description=context-forge daemon" << std::endl;
    service_content << "After=network.target" << std::endl;

    service_content << std::endl;
    service_content << "[Service]" << std::endl;
    service_content << "Type=simple" << std::endl;
    service_content << "Restart=always" << std::endl;
    service_content << "RestartSec=2" << std::endl;
    service_content << "ExecStart=" << binary_path << " server";

    if (this->_settings.contains("verbose")) service_content << " --verbose " << (std::string)this->_settings.at("verbose");
    if (this->_settings.contains("rules"))   service_content << " --rules " << (std::string)this->_settings.at("rules");
    if (this->_settings.contains("ip"))      service_content << " --ip "    << (std::string)this->_settings.at("ip");
    if (this->_settings.contains("port"))    service_content << " --port "  << (std::uint16_t)this->_settings.at("port");
    if (this->_settings.contains("model"))   service_content << " --model " << (std::string)this->_settings.at("model");
    if (this->_settings.contains("system-prompt")) service_content << " --system-prompt " << (std::string)this->_settings.at("system-prompt");
    service_content << std::endl;

    service_content << std::endl;
    service_content << "[Install]" << std::endl;
    service_content << "WantedBy=default.target" << std::endl;

    // Write the service file
    std::ofstream service_file(service_file_path);
    if (!service_file.is_open())
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Failed to create service file: " + service_file_path);
    service_file << service_content.str();
    service_file.close();
    onAdvancedVerbose("Service file created at: " << service_file_path);

    // Relead daemon
    onBasicVerbose("Reloading systemd user daemon config...");
    utils::encapsulation::Process daemon_reload;
    daemon_reload.spawn("bash", {"-c", "systemctl --user daemon-reload"});
    daemon_reload.wait();

    // Enable it to be able to start with the session
    onBasicVerbose("Enabling context-forge service...");
    utils::encapsulation::Process enable_proc;
    enable_proc.spawn("bash", {"-c", "systemctl --user enable context-forge.service"});
    enable_proc.wait();

    // Start for the current session
    onBasicVerbose("Starting context-forge service...");
    utils::encapsulation::Process start_proc;
    start_proc.spawn("bash", {"-c", "systemctl --user restart context-forge.service"});
    const utils::encapsulation::Status& start_status = start_proc.wait();

    // Check the starting status
    if (!start_status.exited || start_status.code != 0) {
        onBasicVerbose("Warning: Service may not have started properly (exit code: " << start_status.code << ", exit sig: " << start_status.sig << ")");
        onBasicVerbose("Use 'systemctl --user status context-forge.service' to check status");
    } else {
        onBasicVerbose("Setup completed successfully!");
        onBasicVerbose("Daemon will start on login and auto-restart on failure and exit");
    }
}

void forge::Forge::stop(void)
{
    onBasicVerbose("Stopping the server...");
    utils::encapsulation::Process stop_proc;
    stop_proc.replace("bash", {"-c", "systemctl --user stop context-forge.service"});
}

void forge::Forge::start(void)
{
    onBasicVerbose("Starting the server...");
    utils::encapsulation::Process stop_proc;
    stop_proc.replace("bash", {"-c", "systemctl --user start context-forge.service"});
}

void forge::Forge::restart(void)
{
    onBasicVerbose("Restarting the server...");
    utils::encapsulation::Process stop_proc;
    stop_proc.replace("bash", {"-c", "systemctl --user restart context-forge.service"});
}

void forge::Forge::status(void)
{
    auto exec = [](const std::string& command) -> std::string {
        // exec the command
        utils::encapsulation::Pipe pipe; pipe.trigger();
        utils::encapsulation::Process proc;
        proc.dup(pipe.getWrite(), STDOUT_FILENO);
        proc.spawn("bash", {"-c", command});
        proc.wait();
        /*const utils::encapsulation::Status& status = proc.wait();
        if (!status.exited || status.code != 0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "code: " + std::to_string(status.code) + ", sig: " + std::to_string(status.sig));
        }*/

        // read result
        std::array<char, 256> buffer{};
        ssize_t n = ::read(pipe.getRead(), buffer.data(), buffer.size());
        if (n <= 0) _unlikely {
            throw utils::exception::ErrorException(utils::exception::InternalCode::Read, ::strerror(errno));
        }

        // remove any line return
        std::string output(buffer.data(), n);
        while (!output.empty() && (output.back() == '\n' || output.back() == '\r'))
            output.pop_back();

        return output;
    };

    // check the server status
    const std::string service_unit = exec("systemctl --user list-unit-files context-forge.service 2>/dev/null");
    const bool service_installed = (service_unit.find("context-forge.service") != std::string::npos);
    const std::string server_status = exec("systemctl --user is-active context-forge.service 2>/dev/null");
    std::cout << utils::smanip::format("<strong>server:<> ");
    if (!service_installed)
        std::cout << utils::iomanip::color_rgb(149, 165, 166) << "not installed" << utils::iomanip::reset() << " (run: context-forge setup)";
    else if (server_status == "active")
        std::cout << utils::iomanip::color_rgb(46, 204, 113)  << "running";
    else if (server_status == "inactive")
        std::cout << utils::iomanip::color_rgb(230, 126, 34)  << "stopped" << utils::iomanip::reset() << " (run: context-forge start)";
    else if (server_status == "failed")
        std::cout << utils::iomanip::color_rgb(231, 76, 60)   << "crashed" << utils::iomanip::reset() << " (good luck o_o)";
    else if (server_status == "activating")
        std::cout << utils::iomanip::color_rgb(241, 196, 15)  << "starting";
    else if (server_status == "deactivating")
        std::cout << utils::iomanip::color_rgb(241, 196, 15)  << "stopping";
    else
        std::cout << utils::iomanip::color_rgb(155, 89, 182)  << server_status << utils::iomanip::reset() << " (good luck o_O, your are on your own!)";
    std::cout << utils::iomanip::reset() << std::endl;

    // check if ollama is installed
    std::cout << utils::smanip::format("<strong>ollama:<> ");
    const std::string ollama_path = exec("command -v ollama 2>/dev/null");
    if (ollama_path.empty()) {
        std::cout << utils::iomanip::color_rgb(149, 165, 166) << "not installed";
        std::cout << utils::iomanip::reset() << std::endl;
        return;
    }
    std::cout << utils::iomanip::color_rgb(149, 165, 166) << "installed" << utils::smanip::format("<> / ");

    // check if the ollama server is running
    const std::string ollama_status = exec("curl -s --max-time 1 http://127.0.0.1:11434/api/tags >/dev/null && echo running || echo stopped");
    if (ollama_status == "running")
        std::cout << utils::iomanip::color_rgb(46, 204, 113) << "running";
    else
        std::cout << utils::iomanip::color_rgb(230, 126, 34) << "stopped" << utils::iomanip::reset() << " (run: ollama serve)";
    std::cout << utils::iomanip::reset() << std::endl;
}

void forge::Forge::remove(void)
{
    onBasicVerbose("Starting removal of systemd user daemon...");

    // Get the service path
    std::string home = ::getenv("HOME");
    if (home.empty())
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "HOME environment variable not set");
    std::string service_file_path = home + "/.config/systemd/user/context-forge.service";

    // Stop the service
    onBasicVerbose("Stopping context-forge service...");
    utils::encapsulation::Process stop_proc;
    stop_proc.spawn("bash", {"-c", "systemctl --user stop context-forge.service"});
    stop_proc.wait();

    // Disable the service (won't restart with the session)
    onBasicVerbose("Disabling context-forge service...");
    utils::encapsulation::Process disable_proc;
    disable_proc.spawn("bash", {"-c", "systemctl --user disable context-forge.service"});
    disable_proc.wait();

    // Remove the service file
    onBasicVerbose("Removing service file: " << service_file_path);
    utils::encapsulation::Process rm_proc;
    rm_proc.spawn("bash", {"-c", "rm -f " + service_file_path});
    rm_proc.wait();

    // Relead daemon
    onBasicVerbose("Reloading systemd user daemon config...");
    utils::encapsulation::Process daemon_reload;
    daemon_reload.spawn("bash", {"-c", "systemctl --user daemon-reload"});
    daemon_reload.wait();

    onBasicVerbose("Removal completed successfully!");
}

void forge::Forge::install(void)
{
    // Check permission, should be executed has root
    if (::geteuid() != 0)
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Ollama installation requires root privileges");

    // Spawn the sub-process that will install ollama
    onBasicVerbose("Starting ollama installation...");
    utils::encapsulation::Process proc;
    proc.spawn("bash", {"-c", "curl -fsSL https://ollama.com/install.sh | sh"});

    // Wait until the end of the ollama installation process
    onBasicVerbose("Waiting for ollama installation to complete...");
    const utils::encapsulation::Status& status = proc.wait();

    // Check the result
    if (status.exited && status.code == 0) {
        onBasicVerbose("Ollama installed successfully!");
    } else {
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "code: " + std::to_string(status.code) + ", sig: " + std::to_string(status.sig));
    }
}

int forge::Forge::exit(void) const
{
    if (this->_status.exited) return this->_status.code; // exited
    else if (!this->_status.unknown) return this->_status.sig; // signal
    return KO; // unknow
}
