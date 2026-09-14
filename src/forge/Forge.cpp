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
#include <fstream>
#include <sstream>

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
    onBasicVerbose("Starting setup of systemd user daemon...");

    // Get the service path
    std::string home = ::getenv("HOME");
    if (home.empty())
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "HOME environment variable not set");
    std::string systemd_dir = home + "/.config/systemd/user";
    std::string service_file_path = systemd_dir + "/context-forge.service";

    // Setup the service emplacement
    onBasicVerbose("Creating systemd directory: " << systemd_dir);
    utils::encapsulation::Process proc_mkdir;
    proc_mkdir.spawn("/bin/bash", {"-c", "mkdir -p " + systemd_dir});
    proc_mkdir.wait();

    // Extract arguments values (need to be updated later using: contains)
    std::string rules, ip, model, system_prompt;
    std::uint16_t port_value = 0;
    try {rules         = (std::string)this->_settings.at("rules");        } catch (...) {}
    try {ip            = (std::string)this->_settings.at("ip");           } catch (...) {}
    try {port_value    = (std::uint16_t)this->_settings.at("port");       } catch (...) {}
    try {model         = (std::string)this->_settings.at("model");        } catch (...) {}
    try {system_prompt = (std::string)this->_settings.at("system-prompt");} catch (...) {}

    // Build the service content
    std::ostringstream service_content;
    service_content << "[Unit]\n";
    service_content << "Description=context-forge daemon\n";
    service_content << "After=network.target\n\n";
    service_content << "[Service]\n";
    service_content << "Type=simple\n";
    service_content << "Restart=on-failure\n";
    service_content << "RestartSec=2\n";
    service_content << "ExecStart=" << home << "/.local/bin/context-forge server";

    if (!rules.empty()) service_content << " --rules " << rules;
    if (!ip.empty()) service_content << " --ip " << ip;
    if (port_value != 0) service_content << " --port " << port_value;
    if (!model.empty()) service_content << " --model " << model;
    if (!system_prompt.empty()) service_content << " --system-prompt " << system_prompt;

    service_content << "\n\n";
    service_content << "[Install]\n";
    service_content << "WantedBy=default.target\n";

    // Write the service file
    std::ofstream service_file(service_file_path);
    if (!service_file.is_open())
        throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "Failed to create service file: " + service_file_path);
    service_file << service_content.str();
    service_file.close();
    onBasicVerbose("Service file created at: " << service_file_path);

    // Relead daemon
    onBasicVerbose("Reloading systemd user daemon config...");
    utils::encapsulation::Process daemon_reload;
    daemon_reload.spawn("/bin/bash", {"-c", "systemctl --user daemon-reload"});
    daemon_reload.wait();

    // Enable it to be able to start with the session
    onBasicVerbose("Enabling context-forge service...");
    utils::encapsulation::Process enable_proc;
    enable_proc.spawn("/bin/bash", {"-c", "systemctl --user enable context-forge.service"});
    enable_proc.wait();

    // Start for the current session
    onBasicVerbose("Starting context-forge service...");
    utils::encapsulation::Process start_proc;
    start_proc.spawn("/bin/bash", {"-c", "systemctl --user start context-forge.service"});
    const utils::encapsulation::Status& start_status = start_proc.wait();

    // Check the starting status
    if (!start_status.exited || start_status.code != 0) {
        onAdvancedVerbose("Warning: Service may not have started properly (exit code: " << start_status.code << ")");
        onBasicVerbose("Use 'systemctl --user status context-forge.service' to check status");
    }

    onBasicVerbose("Setup completed successfully!");
    onBasicVerbose("Daemon will start on login and auto-restart on failure");
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
    stop_proc.spawn("/bin/bash", {"-c", "systemctl --user stop context-forge.service"});
    stop_proc.wait();

    // Disable the service (won't restart with the session)
    onBasicVerbose("Disabling context-forge service...");
    utils::encapsulation::Process disable_proc;
    disable_proc.spawn("/bin/bash", {"-c", "systemctl --user disable context-forge.service"});
    disable_proc.wait();

    // Remove the service file
    onBasicVerbose("Removing service file: " << service_file_path);
    utils::encapsulation::Process rm_proc;
    rm_proc.spawn("/bin/bash", {"-c", "rm -f " + service_file_path});
    rm_proc.wait();

    // Relead daemon
    onBasicVerbose("Reloading systemd user daemon config...");
    utils::encapsulation::Process daemon_reload;
    daemon_reload.spawn("/bin/bash", {"-c", "systemctl --user daemon-reload"});
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
