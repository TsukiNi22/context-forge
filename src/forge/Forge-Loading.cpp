/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗ 
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 20/09/2026 by @author Tsukini

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
#include <variant>
#include <cstdint>
#include <string>

#define quick_fallback(t, s, fallback) (this->_settings.contains(s) ? (t)this->_settings.at(s) : fallback)
void forge::Forge::load(void)
{
    // load rules
    if (this->_settings.contains("rules")) {
        const std::string dirP = quick_fallback(std::string, "plugins", "");
        const std::string dirR = this->_settings.at("rules");
        const bool rec = this->_settings.contains("recursive");

        onDebugVerbose("loading: plugins");
        // load plugins
        if (dirP.empty()) goto rules;

        // check the rules path
        if (!std::filesystem::exists(dirP) || !std::filesystem::is_directory(dirP)) _unlikely {
            onDebugVerboseC(std::cerr, utils::exception::ErrorException(utils::exception::ExternalCode::InvalidDirectory, "I/O error while gettings plugins (the plugins directory was propably removed)").formated());
            goto rules;
        }

        // for each files or sub-files
        if (rec) {
            for (const auto& entry: std::filesystem::recursive_directory_iterator(dirP)) {
                if (entry.is_regular_file() && entry.path().extension() == ".so") {
                    try {this->loadPlugin(entry.path().string());}
                    catch (const utils::exception::IException& e) {onDebugVerboseC(std::cerr, e.formated());}
                }
            }
        } else {
            for (const auto& entry: std::filesystem::directory_iterator(dirP)) {
                if (entry.is_regular_file() && entry.path().extension() == ".so") {
                    try {this->loadPlugin(entry.path().string());}
                    catch (const utils::exception::IException& e) {onDebugVerboseC(std::cerr, e.formated());}
                }
            }
        }

        rules:
        // check if there is rules that where loaded
        if (this->_plugins.size() == 0) {
            onBasicVerbose(
                utils::iomanip::color_rgb(175, 0, 175) << utils::smanip::format("<strong>[WARNING]<>")
                << utils::smanip::format("<strong> loadPlugin: no valid plugins where found, there is a high chance for the rules formating part to be ignored (see logs above or enable debug verbose)<>")
            );
        } else {
            onDebugVerbose("loaded plugins: " << this->_rules.size());
        }

        onDebugVerbose("loading: rules");

        // check the rules path
        if (!std::filesystem::exists(dirR) || !std::filesystem::is_directory(dirR)) _unlikely {
            onDebugVerboseC(std::cerr, utils::exception::ErrorException(utils::exception::ExternalCode::InvalidDirectory, "I/O error while gettings rules (the rules directory was propably removed)").formated());
            goto skip;
        }

        // for each files or sub-files
        if (rec) {
            for (const auto& entry: std::filesystem::recursive_directory_iterator(dirR)) {
                if (entry.is_regular_file() && entry.path().extension() == ".cfg") {
                    try {this->loadCFG(entry.path().string());}
                    catch (const utils::exception::IException& e) {onDebugVerboseC(std::cerr, e.formated());}
                }
            }
        } else {
            for (const auto& entry: std::filesystem::directory_iterator(dirR)) {
                if (entry.is_regular_file() && entry.path().extension() == ".cfg") {
                    try {this->loadCFG(entry.path().string());}
                    catch (const utils::exception::IException& e) {onDebugVerboseC(std::cerr, e.formated());}
                }
            }
        }

        skip:
        // check if there is rules that where loaded
        if (this->_rules.size() == 0) {
            onBasicVerbose(
                utils::iomanip::color_rgb(175, 0, 175) << utils::smanip::format("<strong>[WARNING]<>")
                << utils::smanip::format("<strong> loadCFG: the rules formating part will be ignored, no valid rules where found (see logs above or enable debug verbose)<>")
            );
        } else {
            onDebugVerbose("loaded rules: " << this->_rules.size());
        }
    } else {
        onBasicVerbose(
            utils::iomanip::color_rgb(175, 0, 175) << utils::smanip::format("<strong>[WARNING]<>")
            << utils::smanip::format("<strong> loadCFG: the rules formating part will be ignored, no path given<>")
        );
    }

    // load llm
    onDebugVerbose("loading: llm");
    try {this->loadLLM();}
    catch (const utils::exception::IException& e) {
        onBasicVerbose(
            utils::iomanip::color_rgb(205, 0, 0) << utils::smanip::format("<strong>[FAILED]<>")
            << utils::smanip::format("<strong> loadLLM: the llm formating part will be ignored until a valid restart<>")
        );
        onDebugVerboseC(std::cerr, e.formated());
    }
}

void forge::Forge::loadPlugin(const std::string& path)
{
    utils::encapsulation::SharedObject plugin(path);

    // check if it's was loaded
    if (!plugin.isloaded()) _unlikely {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Plugins, "Wasn't able to load the plugins: " + path);
    }

    // get name/type
    int type = plugin.loadFunction<int (*)()>("type")();
    std::string name = plugin.loadFunction<const char* (*)()>("name")();

    // check name/type
    switch (type) {
        case TYPE_TRIGGER: _fallthrough;
        case TYPE_PRE_RULE: _fallthrough;
        case TYPE_RULE: break;
        default: throw utils::exception::ErrorException(utils::exception::ExternalCode::Plugins, "Invalid type, only (Trigger:0, Pre-Rule:1, Rule:2) are allowed, but got: " + std::to_string(type));
    }
    if (this->_plugins.contains(name)) _unlikely {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Plugins, "Multiple plugins can't have the same name: " + name);
    }

    // store the plugin
    forge::PluginFactory factory;
    switch (type) {
        case TYPE_TRIGGER:  factory = plugin.loadFunction<forge::TriggerFactory>("factory"); break;
        case TYPE_PRE_RULE: factory = plugin.loadFunction<forge::PreRuleFactory>("factory"); break;
        case TYPE_RULE:     factory = plugin.loadFunction<forge::RuleFactory>("factory"); break;
        default: throw utils::exception::FatalException(utils::exception::ExternalCode::Plugins, "Invalid type???? (Some dark shit is happening here!)");
    }

    onDebugVerbose(utils::iomanip::strong() << name << utils::iomanip::reset() << ": plugin successfully loaded");
    //this->_plugins.emplace(name, std::pair{factory, std::move(plugin)});
this->_plugins.emplace(
    name,
    std::pair<PluginFactory, utils::encapsulation::SharedObject>{
        factory,
        std::move(plugin)
    }
);
}

void forge::Forge::loadCFG(const std::string& path)
{
    libconfig::Config cfg;

    // Open & Read the rule file
    try {
        cfg.readFile(path);
    } catch (const libconfig::FileIOException& e) {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, "I/O error while reading file");
    } catch (const libconfig::ParseException& e) {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Rules, "Parse error at " + std::string(e.getFile()) + ":" + std::to_string(e.getLine()) + " - " + std::string(e.getError()));
    }

    // for each setting try to dispatch to a know one
    /*for () {
    }*/

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

void forge::Forge::loadLLM(void)
{
    const std::string model = quick_fallback(std::string, "model", OLLAMA_DEFAULT_MODEL);
    const std::string syprompt = quick_fallback(std::string, "system-prompt", "");

    // resolve ollama address
    utils::network::Address addr;
    addr.ip.first = quick_fallback(std::string, "ip", OLLAMA_DEFAULT_IP);
    addr.port = quick_fallback(std::uint16_t, "port", OLLAMA_DEFAULT_PORT);
    utils::network::socket::resolve_address(addr);

    /*
     * open socket with ollama
     * check if model exists
     * up it and keep it alive using schedule (10min~ & 0s when exit)
    */
}
