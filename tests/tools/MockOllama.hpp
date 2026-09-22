/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 22/09/2026 by @author Tsukini

File Name:
##  @file MockOllama.hpp

File Description:
##  Tools used to mock a minimal ollama http server (/api/show, /api/generate)
\**************************************************************/

#ifndef MOCKOLLAMA_H
    #define MOCKOLLAMA_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #define _Network
    #include <utils/utils.hpp>  // utils::network::Address
    #include <nlohmann/json.hpp>// nlohmann::json
    #include <httplib.h>        // httplib::Server
    #include <functional>       // std::function
    #include <cstdint>          // std::uint16_t
    #include <thread>           // std::thread
    #include <vector>           // std::vector
    #include <string>           // std::string
    #include <mutex>            // std::mutex, std::lock_guard
    #include <set>              // std::set

namespace tests::tools { // namespace start
//----------------------------------------------------------------//
/* CLASS */

class MockOllama {
    private:
        httplib::Server _server;
        std::thread _thread;
        int _port = 0;

        /* behaviour */
        std::set<std::string> _models; // models known by the server
        int _generateStatus = 200; // status returned by /api/generate
        std::function<std::string(const std::string&)> _responder = [](const std::string& prompt) {return "forged: " + prompt;};

        /* journal */
        mutable std::mutex _lock;
        std::vector<nlohmann::json> _shows; // bodies received on /api/show
        std::vector<nlohmann::json> _generates; // bodies received on /api/generate

        // ---------- Pre-Function -------- //
        void setup(void) {
            this->_server.Post("/api/show", [this](const httplib::Request& req, httplib::Response& res) {
                nlohmann::json body = nlohmann::json::parse(req.body, nullptr, false);
                {
                    std::lock_guard<std::mutex> lock(this->_lock);
                    this->_shows.push_back(body);
                }

                // model existance
                if (body.is_object() && body.contains("model") && this->_models.contains(body.at("model").get<std::string>())) {
                    res.status = 200;
                    res.set_content(nlohmann::json{{"modelfile", ""}}.dump(), "application/json");
                } else {
                    res.status = 404;
                    res.set_content(nlohmann::json{{"error", "model not found"}}.dump(), "application/json");
                }
            });

            this->_server.Post("/api/generate", [this](const httplib::Request& req, httplib::Response& res) {
                nlohmann::json body = nlohmann::json::parse(req.body, nullptr, false);
                {
                    std::lock_guard<std::mutex> lock(this->_lock);
                    this->_generates.push_back(body);
                }

                // forced failure
                if (this->_generateStatus != 200) {
                    res.status = this->_generateStatus;
                    res.set_content(nlohmann::json{{"error", "forced failure"}}.dump(), "application/json");
                    return;
                }

                // keep alive request (no prompt) or a real prompt
                std::string response;
                if (body.is_object() && body.contains("prompt")) response = this->_responder(body.at("prompt").get<std::string>());
                res.status = 200;
                res.set_content(nlohmann::json{{"response", response}, {"done", true}}.dump(), "application/json");
            });
        };

    public:
        // ---------- Pre-Function -------- //
        void start(void) {
            this->setup();
            this->_port = this->_server.bind_to_any_port("127.0.0.1");
            this->_thread = std::thread([this] {this->_server.listen_after_bind();});
            this->_server.wait_until_ready();
        };
        void stop(void) {
            this->_server.stop();
            if (this->_thread.joinable()) this->_thread.join();
        };

        // ------------ Function ---------- //
        /* behaviour */
        inline void addModel(const std::string& model) {this->_models.insert(model);};
        inline void generateStatus(int status) {this->_generateStatus = status;};
        inline void responder(const std::function<std::string(const std::string&)>& responder) {this->_responder = responder;};

        /* getter */
        inline int port(void) const {return this->_port;};
        inline utils::network::Address address(void) const {
            utils::network::Address addr;
            addr.ip.first = "127.0.0.1";
            addr.port = static_cast<std::uint16_t>(this->_port);
            return addr;
        };
        inline std::vector<nlohmann::json> shows(void) const {std::lock_guard<std::mutex> lock(this->_lock); return this->_shows;};
        inline std::vector<nlohmann::json> generates(void) const {std::lock_guard<std::mutex> lock(this->_lock); return this->_generates;};

        // ---------- Constructor --------- //
        MockOllama() = default;
        MockOllama(const MockOllama& other) = delete;
        MockOllama(MockOllama&& other) = delete;

        // ----------- Destructor --------- //
        ~MockOllama() {this->stop();};
};

} // namespace end
#endif /* MOCKOLLAMA_H */
