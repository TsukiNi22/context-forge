/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 15/09/2026 by @author Tsukini

File Name:
##  @file Forge-Client.cpp

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
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <cstddef>
#include <cstring>
#include <cerrno>
#include <string>
#include <array>
    
void forge::Forge::fallback(void)
{
    onDebugVerbose("fallback!!!");
    utils::encapsulation::Process proc;
    proc.replace(this->_bin, this->_args);
}

_nodiscard pid_t forge::Forge::getServerPid(void)
{
    // Setup redirection
    onDebugVerbose("setup stdout/stderr redirection...");
    utils::encapsulation::Pipe pipe, pipe_err;
    pipe.trigger(); pipe_err.trigger();
    int fd = pipe.getRead();

    // Get the service pid (0 == not running)
    onDebugVerbose("using systemctl (sub-process), get the service main pid...");
    utils::encapsulation::Process proc;
    proc.dup(pipe.getWrite(), STDOUT_FILENO);
    proc.dup(pipe_err.getWrite(), STDERR_FILENO); // No output displayed, could use an fd on /dev/null but not auto handled
    proc.spawn("systemctl", {"--user", "show", "context-forge.service", "--value", "-p", "MainPID"});
    const utils::encapsulation::Status& status = proc.wait();
    onDebugVerbose("systemctl ended!");
    pipe_err.close(); // prematured close (shouldn't be used in the future)

    // Check process and service status
    if (!status.exited || status.code != 0) _unlikely {
        this->fallback();
        //throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "code: " + std::to_string(status.code) + ", sig: " + std::to_string(status.sig));
    }

    // Extract the content
    onDebugVerbose("extract the pid from the redirect buffer...");
    std::array<char, 4096> buffer{};
    ssize_t n = ::read(fd, buffer.data(), buffer.size());
    if (n <= 0) _unlikely {
        this->fallback();
        //throw utils::exception::ErrorException(utils::exception::InternalCode::Read, ::strerror(errno));
    }

    // Convert to pid
    std::string output(buffer.data(), n);
    if (!output.empty() && output.back() == '\n') output.pop_back(); // ignore '\n' of the output
    onDebugVerbose("convert the buffer into a pid_t: '" << output << "'");
    pid_t pid = std::stoi(output);
    if (pid == 0) _unlikely {
        this->fallback();
        //throw utils::exception::ErrorException(utils::exception::InternalCode::Process, "No running process associated to context-forge.service, got pid: 0");
    }

    return pid;
}

void forge::Forge::exec(void)
{
    pid_t pid = this->getServerPid();

    // check if a server is running
    // in this case get it's pid and open the shm
    // otherwhise just replace this proc by the process (replace)
    // init connection
    // exec bin in sub-process (spawn)
    // redirect given stream
    // send it to the server
    // recv the formated version and display it
    // close connection
}
