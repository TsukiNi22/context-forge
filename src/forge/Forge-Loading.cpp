/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/09/2026 by @author Tsukini

File Name:
##  @file Forge-Formating.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Manip
#define _Network
#define _Attribute
#define _Exception
#define _Verbose
#define _Encapsulation
#include <utils/utils.hpp>
#include "forge/Forge.hpp"
#include <libconfig.h++>
#include <filesystem>
#include <iostream>
#include <cstdint>
#include <string>

void forge::Forge::load(void)
{
    // load rules
    if (this->_settings.contains("rules")) {
        try {
            const std::string dir = this->_settings.at("rules");
            const bool rec = this->_settings.contains("recursive");

            // check the rules path
            if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) _unlikely {
                throw utils::exception::ErrorException(utils::exception::ExternalCode::InvalidDirectory, "I/O error while gettings rules (the rules directory was propably removed)");
            }

            // for each files or sub-files
            if (rec) {
                for (const auto& entry: std::filesystem::recursive_directory_iterator(dir)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".cfg")
                        this->loadCFG(entry.path().string());
                }
            } else {
                for (const auto& entry: std::filesystem::directory_iterator(dir)) {
                    if (entry.is_regular_file() && entry.path().extension() == ".cfg")
                        this->loadCFG(entry.path().string());
                }
            }
        } catch (const utils::exception::IException& e) {
            onBasicVerbose(
                utils::iomanip::color_rgb(205, 0, 0) << utils::smanip::format("<strong>[FAILED]<>")
                << utils::smanip::format("<strong> loadCFG: the rules formating part will be ignored until a valid restart<>")
            );
            onDebugVerboseC(std::cerr, e.formated());
        }
    }

    // load llm
    try {this->loadLLM();}
    catch (const utils::exception::IException& e) {
        onBasicVerbose(
            utils::iomanip::color_rgb(205, 0, 0) << utils::smanip::format("<strong>[FAILED]<>")
            << utils::smanip::format("<strong> loadLLM: the llm formating part will be ignored until a valid restart<>")
        );
        onDebugVerboseC(std::cerr, e.formated());
    }
}

void forge::Forge::loadCFG(const std::string& path)
{
    libconfig::Config cfg;

    // Open & Read the rule file
    try {
        cfg.readFile(path);
    } catch (const libconfig::FileIOException& e) {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, "I/O error while reading file (the rules file was propably removed)");
    } catch (const libconfig::ParseException& e) {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, "Parse error at " + std::string(e.getFile()) + ":" + std::to_string(e.getLine()) + " - " + std::string(e.getError()));
    }

    /*
    // Reading rules
    std::string name;
    if (!cfg.lookupValue("name", name)) {
        name = "default";
    }

    int port = 8080;
    (void)cfg.lookupValue("server.port", port); // supports nested groups: server = { port = 8080; };
    */
}

#define quick_fallback(t, s, fallback) (this->_settings.contains(s) ? (t)this->_settings.at(s) : fallback)
void forge::Forge::loadLLM(void)
{
    const std::string model = quick_fallback(std::string, "model", "qwen2.5-coder:1.5b");
    const std::string syprompt = quick_fallback(std::string, "system-prompt", "");

    // resolve ollama address
    utils::network::Address addr;
    addr.ip.first = quick_fallback(std::string, "ip", "localhost");
    addr.port = quick_fallback(std::uint16_t, "port", 11434);
    utils::network::socket::resolve_address(addr);

    /*
     * open socket with ollama
     * check if model exists
     * up it and keep it alive using schedule (10min~ & 0s when exit)
    */
}
