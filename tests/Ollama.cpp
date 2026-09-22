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
##  @file Ollama.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Verbose
#define _Network
#define _Attribute
#include <utils/utils.hpp>
#include "forge/Ollama.hpp"
#include "tools/MockOllama.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <functional>
#include <cstddef>
#include <thread>
#include <chrono>
#include <string>

// allow use of 10ms, 2s, ...
using namespace std::chrono_literals;

#define MODEL "qwen2.5-coder:1.5b"
#define SYSPROMPT "You are a formater, only awnser with the formated output"

/* wait until the condition is valid or the timeout is reached */
static bool wait_for(const std::function<bool()>& condition, std::chrono::milliseconds timeout = 2s)
{
    const auto end = std::chrono::steady_clock::now() + timeout;

    while (std::chrono::steady_clock::now() < end) {
        if (condition()) return true;
        std::this_thread::sleep_for(10ms);
    }
    return condition();
}

class OllamaTest: public ::testing::Test
{
    protected:
        void SetUp(void) final {
            this->_verbose = utils::verbose::verbose;
            this->_server.addModel(MODEL);
            this->_server.start();
        };
        void TearDown(void) final {
            this->_server.stop();
            utils::verbose::verbose = this->_verbose;
        };

    public:
        tests::tools::MockOllama _server;
        utils::verbose::Verbose _verbose;
};

//----------------------------------------------------------------//
/* NO INIT */

class OllamaNoInitTest : public ::testing::TestWithParam<std::string> {};

TEST_P(OllamaNoInitTest, PromptUntouched) {
    forge::Ollama ollama;

    // without init the prompt should be returned as it is
    ASSERT_EQ(ollama.prompt(GetParam()), GetParam());
}

INSTANTIATE_TEST_SUITE_P(InputCases, OllamaNoInitTest,
    ::testing::Values(
        "Testing",
        "S.O.S",
        "Please need help, fuck the unit_tests...",
        "<binary>ls</binary>\n<output>\nmulti\nline\n</output>\n",
        ""
    )
);

TEST(Ollama, NoInitUpDown) {
    forge::Ollama ollama;

    ASSERT_NO_THROW(ollama.up());
    ASSERT_NO_THROW(ollama.down());
    ASSERT_NO_THROW(ollama.down());
}

TEST_F(OllamaTest, NoInitNoRequest) {
    {
        forge::Ollama ollama;
        ollama.up();
        (void)ollama.prompt("Testing");
        ollama.down();
    }

    // nothing should have reached the server
    std::this_thread::sleep_for(50ms);
    ASSERT_EQ(this->_server.shows().size(), 0);
    ASSERT_EQ(this->_server.generates().size(), 0);
}

//----------------------------------------------------------------//
/* INIT */

TEST_F(OllamaTest, Init) {
    forge::Ollama ollama;

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));

    // the model existance should have been checked
    std::vector<nlohmann::json> shows = this->_server.shows();
    ASSERT_EQ(shows.size(), 1);
    ASSERT_EQ(shows[0].at("model").get<std::string>(), MODEL);
    ASSERT_EQ(this->_server.generates().size(), 0);
}

TEST_F(OllamaTest, InitUnknownModel) {
    forge::Ollama ollama;

    try {
        ollama.init(this->_server.address(), "unknown-model", SYSPROMPT);
        FAIL() << "Expected init to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), static_cast<utils::exception::InternalCode>(utils::exception::ExternalCode::Ollama));
    }

    // the model existance should have been checked
    std::vector<nlohmann::json> shows = this->_server.shows();
    ASSERT_EQ(shows.size(), 1);
    ASSERT_EQ(shows[0].at("model").get<std::string>(), "unknown-model");
}

TEST_F(OllamaTest, InitUnknownModelStayUninitialized) {
    forge::Ollama ollama;

    ASSERT_ANY_THROW(ollama.init(this->_server.address(), "unknown-model", SYSPROMPT));

    // should still act like a non initialized instance
    ASSERT_EQ(ollama.prompt("Testing"), "Testing");
    ASSERT_NO_THROW(ollama.up());
    ASSERT_NO_THROW(ollama.down());
    std::this_thread::sleep_for(50ms);
    ASSERT_EQ(this->_server.generates().size(), 0);
}

TEST_F(OllamaTest, InitUnreachable) {
    forge::Ollama ollama;

    // stop the server to get a closed port
    utils::network::Address addr = this->_server.address();
    this->_server.stop();

    try {
        ollama.init(addr, MODEL, SYSPROMPT);
        FAIL() << "Expected init to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), static_cast<utils::exception::InternalCode>(utils::exception::ExternalCode::Ollama));
    }
    ASSERT_EQ(ollama.prompt("Testing"), "Testing");
}

//----------------------------------------------------------------//
/* PROMPT */

class OllamaPromptTest: public OllamaTest, public ::testing::WithParamInterface<std::string> {};

TEST_P(OllamaPromptTest, ProducesExpectedOutput) {
    forge::Ollama ollama;
    const std::string& input = GetParam();

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));
    ASSERT_EQ(ollama.prompt(input), "forged: " + input);
}

TEST_P(OllamaPromptTest, SendExpectedBody) {
    forge::Ollama ollama;
    const std::string& input = GetParam();

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));
    (void)ollama.prompt(input);

    // check the body sent to the server
    std::vector<nlohmann::json> generates = this->_server.generates();
    ASSERT_EQ(generates.size(), 1);
    const nlohmann::json& body = generates[0];
    EXPECT_EQ(body.at("model").get<std::string>(), MODEL);
    EXPECT_EQ(body.at("prompt").get<std::string>(), input);
    EXPECT_EQ(body.at("system").get<std::string>(), SYSPROMPT);
    EXPECT_EQ(body.at("stream").get<bool>(), false);
    EXPECT_EQ(body.at("keep_alive").get<std::string>(), "10m");
    EXPECT_FALSE(body.contains("context")); // memory wiped each time
}

TEST_P(OllamaPromptTest, FailureUntouched) {
    forge::Ollama ollama;
    const std::string& input = GetParam();

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));

    // on failure the prompt should be returned as it is
    this->_server.generateStatus(500);
    ASSERT_EQ(ollama.prompt(input), input);
    ASSERT_EQ(this->_server.generates().size(), 1);
}

TEST_P(OllamaPromptTest, UnreachableUntouched) {
    forge::Ollama ollama;
    const std::string& input = GetParam();

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));

    // on unreachable server the prompt should be returned as it is
    this->_server.stop();
    ASSERT_EQ(ollama.prompt(input), input);
}

INSTANTIATE_TEST_SUITE_P(InputCases, OllamaPromptTest,
    ::testing::Values(
        "Testing",
        "S.O.S",
        "Please need help, fuck the unit_tests...",
        "<binary>ls</binary>\n<output>\nmulti\nline\n</output>\n",
        ""
    )
);

TEST_F(OllamaTest, PromptMultiple) {
    forge::Ollama ollama;

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));
    for (std::size_t i = 0; i < 5; ++i)
        ASSERT_EQ(ollama.prompt("Testing " + std::to_string(i)), "forged: Testing " + std::to_string(i));
    ASSERT_EQ(this->_server.generates().size(), 5);
}

TEST_F(OllamaTest, PromptCustomResponder) {
    forge::Ollama ollama;

    this->_server.responder([](const std::string& prompt) {return "[" + prompt + "]";});
    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));
    ASSERT_EQ(ollama.prompt("Testing"), "[Testing]");
}

//----------------------------------------------------------------//
/* UP / DOWN */

TEST_F(OllamaTest, UpAwake) {
    forge::Ollama ollama;

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));
    ollama.up();

    // the awake request should be sent right away
    ASSERT_TRUE(wait_for([this] {return this->_server.generates().size() >= 1;}));
    std::vector<nlohmann::json> generates = this->_server.generates();
    ASSERT_EQ(generates.size(), 1);
    const nlohmann::json& body = generates[0];
    EXPECT_EQ(body.at("model").get<std::string>(), MODEL);
    EXPECT_EQ(body.at("keep_alive").get<std::string>(), "10m");
    EXPECT_FALSE(body.contains("prompt"));

    // the next awake is far away (9min), nothing else should come
    std::this_thread::sleep_for(100ms);
    ASSERT_EQ(this->_server.generates().size(), 1);
    ollama.down();
}

TEST_F(OllamaTest, UpPrompt) {
    forge::Ollama ollama;

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));
    ollama.up();
    ASSERT_TRUE(wait_for([this] {return this->_server.generates().size() >= 1;}));

    // the prompt should still work while the awake loop is running
    ASSERT_EQ(ollama.prompt("Testing"), "forged: Testing");
    ASSERT_EQ(this->_server.generates().size(), 2);
    ollama.down();
}

TEST_F(OllamaTest, DownCancelAwake) {
    forge::Ollama ollama;

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));
    ollama.up();
    ASSERT_TRUE(wait_for([this] {return this->_server.generates().size() >= 1;}));
    ollama.down();

    // no more request after the down
    const std::size_t count = this->_server.generates().size();
    std::this_thread::sleep_for(100ms);
    ASSERT_EQ(this->_server.generates().size(), count);
}

TEST_F(OllamaTest, DownWithoutUp) {
    forge::Ollama ollama;

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));
    ASSERT_NO_THROW(ollama.down());
    ASSERT_NO_THROW(ollama.down());
    std::this_thread::sleep_for(50ms);
    ASSERT_EQ(this->_server.generates().size(), 0);
}

TEST_F(OllamaTest, UpDownUp) {
    forge::Ollama ollama;

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));

    // first loop
    ollama.up();
    ASSERT_TRUE(wait_for([this] {return this->_server.generates().size() >= 1;}));
    ollama.down();

    // second loop
    ollama.up();
    ASSERT_TRUE(wait_for([this] {return this->_server.generates().size() >= 2;}));
    ollama.down();
}

TEST_F(OllamaTest, DestructorDown) {
    {
        forge::Ollama ollama;
        ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));
        ollama.up();
        ASSERT_TRUE(wait_for([this] {return this->_server.generates().size() >= 1;}));
    }

    // the destructor should have stopped the awake loop without crash
    const std::size_t count = this->_server.generates().size();
    std::this_thread::sleep_for(100ms);
    ASSERT_EQ(this->_server.generates().size(), count);
}

TEST_F(OllamaTest, UpUnreachable) {
    forge::Ollama ollama;

    ASSERT_NO_THROW(ollama.init(this->_server.address(), MODEL, SYSPROMPT));

    // the awake loop should survive an unreachable server
    this->_server.stop();
    ASSERT_NO_THROW(ollama.up());
    std::this_thread::sleep_for(100ms);
    ASSERT_NO_THROW(ollama.down());
}
