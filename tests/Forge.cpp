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
##  @file Forge.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Verbose
#define _Arguments
#define _Attribute
#include <utils/utils.hpp>
#include "forge/Forge.hpp"
#include <gtest/gtest.h>
#include <type_traits>
#include <filesystem>
#include <optional>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <string>

//----------------------------------------------------------------//
/* TRAITS */

TEST(Forge, Traits) {
    // the forge should never be duplicated
    static_assert(!std::is_copy_constructible_v<forge::Forge>);
    static_assert(!std::is_copy_assignable_v<forge::Forge>);
    static_assert(!std::is_move_constructible_v<forge::Forge>);
    static_assert(!std::is_move_assignable_v<forge::Forge>);
    static_assert(std::is_default_constructible_v<forge::Forge>);
}

//----------------------------------------------------------------//
/* EXIT / RUN */

TEST(Forge, ExitWithoutExecution) {
    forge::Forge forge;

    // no process where executed (server/setup/... modes), the default status is a success
    ASSERT_EQ(forge.exit(), OK);
}

TEST(Forge, RunWithoutInit) {
    forge::Forge forge;

    try {
        forge.run();
        FAIL() << "Expected run to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Process);
    }
}

//----------------------------------------------------------------//
/* INIT */

/* placeholder replaced by the fixture: {dir} temporary directory, {file} temporary regular file */
struct ForgeInitCase {
    std::string name;
    std::vector<std::string> argv; // without the binary name
    std::optional<utils::exception::InternalCode> code; // nullopt: should not throw
    utils::exception::Type type = utils::exception::Type::Error;
};
std::ostream& operator<<(std::ostream& os, const ForgeInitCase& c) {return os << c.name;}

class ForgeInitTest: public ::testing::Test, public ::testing::WithParamInterface<ForgeInitCase>
{
    protected:
        void SetUp(void) final {
            this->_verbose = utils::verbose::verbose;

            // the environement should never interfere
            for (const char* env: {"CONTEXT_FORGE_HOST_IPV4", "CONTEXT_FORGE_HOST_PORT", "CONTEXT_FORGE_MODEL", "CONTEXT_FORGE_SYSTEM_PROMPT", "VERBOSE"})
                ::unsetenv(env);

            // temporary tree
            std::string tmp = (std::filesystem::temp_directory_path() / "context-forge-tests-XXXXXX").string();
            ASSERT_NE(::mkdtemp(tmp.data()), nullptr);
            this->_dir = tmp;
            this->_file = this->_dir / "system-prompt";
            std::filesystem::create_directories(this->_dir / "plugins");
            std::filesystem::create_directories(this->_dir / "rules");
            std::ofstream(this->_file) << "You are a formater" << std::endl;
        };
        void TearDown(void) final {
            std::error_code ec;
            std::filesystem::remove_all(this->_dir, ec);
            utils::verbose::verbose = this->_verbose;
        };

    public:
        // build the argv with the binary name & the placeholders replaced
        std::vector<std::string> argv(const std::vector<std::string>& args) const {
            std::vector<std::string> out = {"context-forge"};
            for (std::string arg: args) {
                for (const auto& [key, value]: {std::pair{"{dir}", this->_dir.string()}, std::pair{"{file}", this->_file.string()}})
                    for (std::size_t pos = arg.find(key); pos != std::string::npos; pos = arg.find(key, pos + value.size()))
                        arg.replace(pos, std::string(key).size(), value);
                out.push_back(arg);
            }
            return out;
        };

        std::filesystem::path _dir;
        std::filesystem::path _file;
        utils::verbose::Verbose _verbose;
};

TEST_P(ForgeInitTest, ParseArguments) {
    const ForgeInitCase& testCase = GetParam();
    forge::Forge forge;

    // build the raw argv
    std::vector<std::string> args = this->argv(testCase.argv);
    std::vector<const char*> argv;
    for (const std::string& arg: args) argv.push_back(arg.c_str());

    // ignore any parser output (help, failsafe warnings)
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();

    if (!testCase.code.has_value()) {
        EXPECT_NO_THROW(forge.init(static_cast<int>(argv.size()), argv.data()));
    } else {
        try {
            forge.init(static_cast<int>(argv.size()), argv.data());
            ADD_FAILURE() << "Expected init to throw";
        } catch (const utils::exception::IException& e) {
            EXPECT_EQ(e.getType(), testCase.type);
            EXPECT_EQ(e.getCode(), *testCase.code);
        }
    }

    (void)testing::internal::GetCapturedStdout();
    (void)testing::internal::GetCapturedStderr();
}

INSTANTIATE_TEST_SUITE_P(ArgumentsCases, ForgeInitTest,
    ::testing::Values(
        // exec
        ForgeInitCase{"Exec", {"exec", "-c", "ls"}, std::nullopt},
        ForgeInitCase{"ExecArguments", {"exec", "-c", "ls", "-la", "/tmp"}, std::nullopt},
        ForgeInitCase{"ExecShortFlags", {"exec", "-n", "-d", "1", "-c", "ls"}, std::nullopt},
        ForgeInitCase{"ExecLongFlags", {"exec", "--no-nl", "--redirect", "2", "--command", "ls"}, std::nullopt},
        ForgeInitCase{"ExecVerbose", {"exec", "-v", "debug", "-c", "ls"}, std::nullopt},
        ForgeInitCase{"ExecMissingCommand", {"exec"}, utils::exception::InternalCode::NoCompliantUsage},
        // no case with a flag missing its mandatory option (ex: "exec -c"): libutils restriction on FlagOptionsNumber
        // doesn't allow the Warning type used by the failsafe parsing (FatalException -> abort)
        ForgeInitCase{"ExecInvalidFd", {"exec", "-d", "abc", "-c", "ls"}, utils::exception::InternalCode::NoCompliantUsage},
        ForgeInitCase{"ExecNegativeFd", {"exec", "-d", "-1", "-c", "ls"}, utils::exception::InternalCode::NoCompliantUsage},
        ForgeInitCase{"ExecClosedFd", {"exec", "-d", "999", "-c", "ls"}, utils::exception::InternalCode::NoCompliantUsage},

        // server
        ForgeInitCase{"Server", {"server"}, std::nullopt},
        ForgeInitCase{"ServerPlugins", {"server", "--plugins", "{dir}/plugins"}, std::nullopt},
        ForgeInitCase{"ServerRules", {"server", "--rules", "{dir}/rules", "--recursive"}, std::nullopt},
        ForgeInitCase{"ServerOllama", {"server", "--ip", "127.0.0.1", "--port", "11434", "--model", "qwen2.5-coder:1.5b", "--system-prompt", "{file}"}, std::nullopt},
        ForgeInitCase{"ServerFull", {"server", "-P", "{dir}/plugins", "-r", "{dir}/rules", "-R", "-a", "127.0.0.1", "-p", "11434", "-m", "qwen2.5-coder:1.5b", "-s", "{file}", "-v", "advanced"}, std::nullopt},
        ForgeInitCase{"ServerInvalidPlugins", {"server", "--plugins", "{dir}/unknown"}, utils::exception::InternalCode::NoCompliantUsage},
        ForgeInitCase{"ServerInvalidRules", {"server", "--rules", "{file}"}, utils::exception::InternalCode::NoCompliantUsage},
        ForgeInitCase{"ServerInvalidSystemPrompt", {"server", "--system-prompt", "{dir}"}, utils::exception::InternalCode::NoCompliantUsage},
        ForgeInitCase{"ServerInvalidPort", {"server", "--port", "abc"}, utils::exception::InternalCode::NoCompliantUsage},
        ForgeInitCase{"ServerInvalidVerbose", {"server", "--verbose", "loud"}, utils::exception::InternalCode::NoCompliantUsage},

        // check
        ForgeInitCase{"CheckLocal", {"check", "local"}, std::nullopt},
        ForgeInitCase{"CheckLocalFull", {"check", "local", "--plugins", "{dir}/plugins", "--rules", "{dir}/rules", "--system-prompt", "{file}"}, std::nullopt},
        ForgeInitCase{"CheckDist", {"check", "dist"}, std::nullopt},
        ForgeInitCase{"CheckDistOllama", {"check", "dist", "--ip", "127.0.0.1", "--port", "11434", "--model", "qwen2.5-coder:1.5b"}, std::nullopt},
        ForgeInitCase{"CheckMissingMode", {"check"}, utils::exception::InternalCode::NoCompliantUsage},
        ForgeInitCase{"CheckInvalidMode", {"check", "remote"}, utils::exception::InternalCode::NoCompliantUsage},

        // setup / remove
        ForgeInitCase{"Setup", {"setup"}, std::nullopt},
        ForgeInitCase{"SetupCopy", {"setup", "--copy", "--plugins", "{dir}/plugins", "--rules", "{dir}/rules", "--system-prompt", "{file}"}, std::nullopt},
        ForgeInitCase{"SetupShortCopy", {"setup", "-C", "-R"}, std::nullopt},
        ForgeInitCase{"Remove", {"remove"}, std::nullopt},
        ForgeInitCase{"RemoveVerbose", {"remove", "--verbose", "none"}, std::nullopt},

        // service
        ForgeInitCase{"Stop", {"stop"}, std::nullopt},
        ForgeInitCase{"Start", {"start"}, std::nullopt},
        ForgeInitCase{"Restart", {"restart"}, std::nullopt},
        ForgeInitCase{"Status", {"status"}, std::nullopt},
        ForgeInitCase{"StatusOllama", {"status", "--ip", "127.0.0.1", "--port", "11434"}, std::nullopt},
        ForgeInitCase{"StatusInvalidFlag", {"status", "--verbose", "basic"}, utils::exception::InternalCode::NoCompliantUsage},

        // ollama
        ForgeInitCase{"InstallOllama", {"install-ollama"}, std::nullopt},
        ForgeInitCase{"Pull", {"pull", "qwen2.5-coder:1.5b"}, std::nullopt},
        ForgeInitCase{"PullMissingModel", {"pull"}, utils::exception::InternalCode::NoCompliantUsage},

        // global
        ForgeInitCase{"NoArguments", {}, utils::exception::InternalCode::NoCompliantUsage},
        ForgeInitCase{"UnknownMode", {"unknown"}, utils::exception::InternalCode::NoCompliantUsage},
        ForgeInitCase{"UnknownFlag", {"server", "--unknown"}, utils::exception::InternalCode::NoCompliantUsage},
        ForgeInitCase{"MultipleModes", {"start", "stop"}, utils::exception::InternalCode::NoCompliantUsage},
        ForgeInitCase{"Help", {"-h"}, utils::exception::InternalCode::Exit, utils::exception::Type::None},
        ForgeInitCase{"HelpLong", {"--help"}, utils::exception::InternalCode::Exit, utils::exception::Type::None},
        ForgeInitCase{"HelpWithMode", {"server", "--help"}, utils::exception::InternalCode::Exit, utils::exception::Type::None}
    ),
    [](const ::testing::TestParamInfo<ForgeInitCase>& info) {return info.param.name;}
);

//----------------------------------------------------------------//
/* VERBOSE */

struct ForgeVerboseCase {
    std::string name;
    std::optional<std::string> level; // nullopt: flag not given
    utils::verbose::Verbose expected;
};
std::ostream& operator<<(std::ostream& os, const ForgeVerboseCase& c) {return os << c.name;}

class ForgeVerboseTest : public ::testing::TestWithParam<ForgeVerboseCase> {};

TEST_P(ForgeVerboseTest, SetGlobalVerbose) {
    const ForgeVerboseCase& testCase = GetParam();
    const utils::verbose::Verbose save = utils::verbose::verbose;
    forge::Forge forge;

    // start from a known state
    ::unsetenv("VERBOSE");
    utils::verbose::verbose = utils::verbose::Verbose::Basic;

    // the verbose flag should be before the command one (unlimited flag, consume everything after it)
    std::vector<std::string> args = {"context-forge", "exec"};
    if (testCase.level.has_value()) {
        args.push_back("--verbose");
        args.push_back(*testCase.level);
    }
    args.push_back("-c");
    args.push_back("ls");
    std::vector<const char*> argv;
    for (const std::string& arg: args) argv.push_back(arg.c_str());

    testing::internal::CaptureStdout();
    EXPECT_NO_THROW(forge.init(static_cast<int>(argv.size()), argv.data()));
    (void)testing::internal::GetCapturedStdout();

    // copy of the volatile global to be printable on failure
    const utils::verbose::Verbose verbose = utils::verbose::verbose;
    EXPECT_EQ(verbose, testCase.expected);
    utils::verbose::verbose = save;
}

INSTANTIATE_TEST_SUITE_P(LevelCases, ForgeVerboseTest,
    ::testing::Values(
        ForgeVerboseCase{"Default", std::nullopt, utils::verbose::Verbose::Basic},
        ForgeVerboseCase{"None", "none", utils::verbose::Verbose::None},
        ForgeVerboseCase{"Basic", "basic", utils::verbose::Verbose::Basic},
        ForgeVerboseCase{"Advanced", "advanced", utils::verbose::Verbose::Advanced},
        ForgeVerboseCase{"Debug", "debug", utils::verbose::Verbose::Debug}
    ),
    [](const ::testing::TestParamInfo<ForgeVerboseCase>& info) {return info.param.name;}
);
