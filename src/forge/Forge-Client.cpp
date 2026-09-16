/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 16/09/2026 by @author Tsukini

File Name:
##  @file Forge-Client.cpp

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
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <algorithm>
#include <optional>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cerrno>
#include <thread>
#include <vector>
#include <string>
#include <array>

_hidden _nodiscard static inline std::vector<std::byte> stringToBytes(const std::string& str)
{
    std::vector<std::byte> bytes(str.size());
    std::memcpy(bytes.data(), str.data(), str.size());
    return bytes;
}

void forge::Forge::fallback(void)
{
    onBasicVerbose(
        utils::iomanip::color_rgb(175, 0, 175) << utils::smanip::format("<strong>[FALLBACK]<>")
        << utils::smanip::format("<strong> context-forge: server is unavailable... (attempting to restart the service; logs: journalctl --user -u context-forge)<>")
    );
    utils::encapsulation::Process proc;
    proc.replace(this->_bin, this->_args);
}

_nodiscard pid_t forge::Forge::getServerPid(void)
{
    // Setup redirection
    onDebugVerbose("setup stdout/stderr redirection...");
    utils::encapsulation::Pipe pipe, pipe_err;
    pipe.trigger(); pipe_err.trigger();

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
        onDebugVerboseC(std::cerr, "systemctl failed (exited=" << status.exited << ", code=" << status.code << ", sig=" << status.sig << "), fallback will be triggered");
        this->fallback();
        throw utils::exception::FatalException(utils::exception::InternalCode::Process, "code: " + std::to_string(status.code) + ", sig: " + std::to_string(status.sig));
    }

    // Extract the content
    onDebugVerbose("extract the pid from the redirect buffer...");
    std::array<char, 4096> buffer{};
    ssize_t n = ::read(pipe.getRead(), buffer.data(), buffer.size());
    if (n <= 0) _unlikely {
        onDebugVerboseC(std::cerr, "failed to read pid from buffer (n=" << n << "), fallback will be triggered");
        this->fallback();
        throw utils::exception::FatalException(utils::exception::InternalCode::Read, ::strerror(errno));
    }

    // Convert to pid
    std::string output(buffer.data(), n);
    if (!output.empty() && output.back() == '\n') output.pop_back(); // ignore '\n' of the output
    onDebugVerbose("convert the buffer into a pid_t: '" << output << "'");
    pid_t pid = std::stoi(output);
    if (pid == 0) _unlikely {
        onDebugVerboseC(std::cerr, "server not running (pid=0), fallback will be trigger");
        this->fallback();
        throw utils::exception::FatalException(utils::exception::InternalCode::Process, "No running process associated to context-forge.service, got pid: 0");
    }

    return pid;
}

void forge::Forge::exec(void)
{
    onDebugVerbose("get server pid...");
    pid_t pid = this->getServerPid();
    if (pid == 0) _unlikely { // Should be impossible to return 0
        onDebugVerboseC(std::cerr, "server not running (pid=0), using fallback (Some dark shit is happening here!)");
        this->fallback();
        throw utils::exception::FatalException(utils::exception::InternalCode::Process, "No running process associated to context-forge.service, got pid: 0");
    }

    // Open the shm
    onDebugVerbose("opening shared memory with server pid: " << pid);
    utils::encapsulation::SharedMemory shm;
    std::string shm_name = SHM_NAME + std::to_string(pid);
    shm.init<false, utils::encapsulation::shm::LayoutPolicy::Interleaved>(shm_name);
    onDebugVerbose("shm opened as: " << shm_name);

    // Setup the fd redirection
    onDebugVerbose("setup redirection...");
    utils::encapsulation::Pipe pipe; pipe.trigger();
    int redirectedFd = this->_settings.contains("redirect") ? (int)this->_settings.at("redirect") : STDIN_FILENO;

    // Start the sub-process
    onDebugVerbose("----------------- [Execution] -----------------");
    utils::encapsulation::Process proc;
    proc.dup(pipe.getWrite(), redirectedFd);
    proc.spawn(this->_bin, this->_args);

    // Close the write from parent side
    while (proc.getPid() == -1) // until the process is up
        std::this_thread::yield();
    pipe.closeWrite();

    // Read in parrallele the pipe
    std::string output;
    char buffer[4096];
    ssize_t n;
    while ((n = ::read(pipe.getRead(), buffer, sizeof(buffer))) > 0)
        output.append(buffer, static_cast<std::size_t>(n));
    if (n < 0 && errno != EINTR) _unlikely {
        onBasicVerbose(
            utils::iomanip::color_rgb(205, 0, 0) << utils::smanip::format("<strong>[ERROR]<>")
            << utils::smanip::format("<strong> read on redirected fd failed: ") << ::strerror(errno)
            << utils::iomanip::reset()
        );
    }

    // Wait until the end of the sub-process
    this->_status = proc.wait();
    onDebugVerbose("----------------- [Execution] -----------------");

    // Send the information
    std::vector<std::byte> bytes = stringToBytes(output);
    std::size_t channel_used = (bytes.size() / CHANNEL_SIZE) + (bytes.size() % CHANNEL_SIZE != 0);
    if (channel_used == 0) channel_used = 1; // always send at least one string even empty

    // function to select chunk of the bytes
    constexpr std::size_t headerSize = sizeof(std::int32_t);
    constexpr std::size_t payloadSize = CHANNEL_SIZE - headerSize;
    auto chunkAt = [&bytes, payloadSize](std::size_t index) -> std::vector<std::byte> {
        std::size_t offset = index * payloadSize;
        std::size_t len = std::min(payloadSize, bytes.size() - offset);
        std::vector<std::byte> chunk(headerSize + len);
        std::int32_t chunkIndex = static_cast<std::int32_t>(index);
        std::memcpy(chunk.data(), &chunkIndex, headerSize);
        std::memcpy(chunk.data() + headerSize, bytes.data() + offset, len);
        return chunk;
    };

    // Send all chunk
    utils::encapsulation::shm::Id id = shm.send(chunkAt(0), (channel_used == 1), true);
    for (std::size_t i = 1; i < channel_used; ++i)
        shm.send(chunkAt(i), id, (i == channel_used - 1), true);

    // Get the information
    shm.join(id, true); // wait for the full awnser
    std::optional<std::vector<std::vector<std::byte>>> payloads = shm.read(id);
    std::string formated;

    // Format the return
    if (payloads.has_value()) _likely {
        std::vector<std::pair<std::uint32_t, std::vector<std::byte>>> chunks;
        chunks.reserve(payloads->size());

        // separate the index and content from the payload
        for (const std::vector<std::byte>& chunk : *payloads) {
            std::uint32_t chunkIndex = 0;
            std::memcpy(&chunkIndex, chunk.data(), headerSize);
            std::vector<std::byte> payload(chunk.begin() + headerSize, chunk.end());
            chunks.emplace_back(chunkIndex, std::move(payload));
        }

        // sort the chunks
        std::sort(chunks.begin(), chunks.end(), [](const auto& a, const auto& b) {return a.first < b.first;});

        // reserve the formated output size and reassemble it
        std::size_t size = 0;
        for (const auto& [_, payload]: chunks) size += payload.size();
        formated.reserve(size);
        for (const auto& [_, payload]: chunks)
            formated.append(reinterpret_cast<const char*>(payload.data()), payload.size());
    }

    // Display the formated version
    onDebugVerbose("----------------- [Formated] -----------------");
    ::write(redirectedFd, formated.data(), formated.size());
    onDebugVerbose("----------------- [Formated] -----------------");
}
