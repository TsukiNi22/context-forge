/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 18/09/2026 by @author Tsukini

File Name:
##  @file Forge-Server.cpp

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
#include <unistd.h>
#include <algorithm>
#include <optional>
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

_hidden _nodiscard static inline std::vector<std::byte> stringToBytes(const std::string& str)
{
    std::vector<std::byte> bytes(str.size());
    std::memcpy(bytes.data(), str.data(), str.size());
    return bytes;
}

void forge::Forge::server(void)
{
    onAdvancedVerbose("load ressources (rules, system-prompt, ect)...");
    this->load();

    onAdvancedVerbose("create the shm...");
    utils::encapsulation::SharedMemory shm;
    std::string shm_name = SHM_NAME + std::to_string(shm.ownership());
    shm.init<true, utils::encapsulation::shm::LayoutPolicy::Interleaved>(shm_name, CHANNEL_SIZE, CHANNEL_NUMBER);
    onDebugVerbose("shm created as: " << shm_name);

    // inf loop
    constexpr std::size_t headerSize = sizeof(std::uint32_t);
    constexpr std::size_t payloadSize = CHANNEL_SIZE - headerSize;
    while (true) {
        // Get the information
        onAdvancedVerbose("waiting for transmition...");
        shm.join(true); // wait for any full awnser
        onAdvancedVerbose("new transmition!");
        std::optional<std::unordered_map<utils::encapsulation::shm::Id, std::vector<std::vector<std::byte>>>> payloads = shm.read(utils::encapsulation::shm::ReadFilter::LastOnly);
        std::unordered_map<utils::encapsulation::shm::Id, std::string> inputs;

        // Format the return
        onAdvancedVerbose("reading...");
        if (payloads.has_value()) _likely {
            for (const auto& [id, sub_payloads]: *payloads) {
                std::vector<std::pair<std::uint32_t, std::vector<std::byte>>> chunks;
                chunks.reserve(sub_payloads.size());
                std::string& input = inputs[id];

                // separate the index and content from the payload
                for (const std::vector<std::byte>& chunk: sub_payloads) {
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
                input.reserve(size);
                for (const auto& [_, payload]: chunks)
                    input.append(reinterpret_cast<const char*>(payload.data()), payload.size());
            }
        }

        // display for debug purpose transmition
        onDebugVerboseFn(
            std::cout << "--------- [transmitions] ---------" << std::endl;
            for (const auto& [_, input]: inputs) {
                std::cout
                << "----- start -----" << std::endl
                << input
                << "------ end ------" << std::endl
                ;
            }
            std::cout << "--------- [transmitions] ---------" << std::endl;
        );

        // Pass throught internal formating
        onAdvancedVerbose("formating...");
        for (auto& [_, input]: inputs) {
            this->formatCFG(input); // rules files (*.cfg)
            this->formatLLM(input); // llm (with ollama)
        }

        // Send the information
        onAdvancedVerbose("sending...");
        for (const auto& [_, input]: inputs) {
            std::vector<std::byte> bytes = stringToBytes(input);
            std::size_t channel_used = (bytes.size() / CHANNEL_SIZE) + (bytes.size() % CHANNEL_SIZE != 0);
            if (channel_used == 0) channel_used = 1; // always send at least one string even empty

            // function to select chunk of the bytes
            auto chunkAt = [&bytes, payloadSize](std::size_t index) -> std::vector<std::byte> {
                std::size_t offset = index * payloadSize;
                std::size_t len = std::min(payloadSize, bytes.size() - offset);
                std::vector<std::byte> chunk(headerSize + len);
                std::uint32_t chunkIndex = static_cast<std::uint32_t>(index);
                std::memcpy(chunk.data(), &chunkIndex, headerSize);
                std::memcpy(chunk.data() + headerSize, bytes.data() + offset, len);
                return chunk;
            };

            // Send all chunk
            utils::encapsulation::shm::Id id = shm.send(chunkAt(0), (channel_used == 1), true);
            for (std::size_t i = 1; i < channel_used; ++i)
                shm.send(chunkAt(i), id, (i == channel_used - 1), true);
        }
    }
}
