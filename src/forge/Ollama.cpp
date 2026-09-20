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
##  @file Ollama.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#define _Exception
#define _Verbose
#define _Network
#include <utils/utils.hpp>
#include "forge/Ollama.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <chrono>

// allow use of 9min, 0ms, ...
using namespace std::chrono_literals;

_cold void forge::Ollama::model_(void) const
{
    onDebugVerbose("ollama [" << this->_model << "]: checking model existance...");
    nlohmann::json body = {{"model", this->_model}};
    auto res = this->_cli->Post("/api/show", body.dump(), "application/json");

    // check return
    if (!res || res->status != 200) {
        onDebugVerbose("ollama [" << this->_model << "]: failed to find the model (run: context-forge pull '" << this->_model << "') or the ollama server is not running (run: context-forge status) [status: " << res->status << "]");
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Ollama, "Failed to find the given model: " + this->_model);
    }

    onDebugVerbose("ollama [" << this->_model << "]: model was found");
}

_cold void forge::Ollama::awake(void) const
{
    if (!this->_init) return;
    onDebugVerbose("ollama: awake");

    // awake the ollama server for the next 10min
    nlohmann::json body = {
        {"model", this->_model},
        {"keep_alive", "10m"}
    };
    this->_cli->Post("/api/generate", body.dump(), "application/json");

    // schedule the next awake in 9min
    this->_scheduler.schedule(9min, [this] {this->awake();});
}

_cold void forge::Ollama::init(const utils::network::Address& addr, const std::string& model, const std::string& sysprompt)
{
    // setup connection
    onDebugVerbose("ollama: init the connection");
    try {
        this->_cli.emplace(addr.ip.first, addr.port);
        this->_cli->set_connection_timeout(5);
        this->_cli->set_read_timeout(120);
    } catch (const std::exception& e) {
        throw utils::exception::ErrorException(utils::exception::ExternalCode::Ollama, "Failed to setup the connection: " + std::string(e.what()));
    }

    // setup internal var
    this->_model = model;
    this->_sysprompt = sysprompt;
    this->model_();

    this->_init = true;
}

_hot _nodiscard std::string forge::Ollama::prompt(const std::string& prompt) const
{
    if (!this->_init) return prompt;

    // send the prompt
    onDebugVerbose("ollama: sending prompt...")
    nlohmann::json body = {
        {"model", this->_model},
        {"prompt", prompt},
        {"system", this->_sysprompt},
        {"stream", false}, // await full awnser
        {"keep_alive", "10m"}
        // no context filed (memory wiped each time)
    };
    auto res = this->_cli->Post("/api/generate", body.dump(), "application/json");

    // check the return
    if (!res || res->status != 200) {
        onDebugVerbose("ollama: failed to format the prompt [status: " << res->status << "]");
        return prompt;
    }

    // get the return
    onDebugVerbose("ollama: prompt successfully formated")
    auto json = nlohmann::json::parse(res->body);
    return json.at("response").get<std::string>();
}

_cold void forge::Ollama::up(void)
{
    if (!this->_init) return;

    // start awake loop
    onDebugVerbose("ollama: starting the server");
    (void)this->_scheduler.schedule(0ms, [this] {this->awake();});
}

_cold void forge::Ollama::down(void)
{
    if (!this->_init) return;

    // stop awake loop
    onDebugVerbose("ollama: the server was taken down");
    this->_scheduler.cancel(); // cancel all current tasks scheduled
}
