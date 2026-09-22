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
##  @file AInstruction.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#define _Observer
#include <utils/utils.hpp>
#include <utils/security/observer/INotifier.hpp>
#include "forge/rules/AInstruction.hpp"
#include "forge/rules/IInstruction.hpp"
#include "tools/MockInstruction.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <memory>
#include <string>
#include <regex>

/* minimal concrete instruction */
class NamedInstruction: public forge::rules::AInstruction {
    public:
        void load(const libconfig::Setting& s) final {(void)s;};

        NamedInstruction() = default;
        NamedInstruction(const std::string& name): forge::rules::AInstruction(name) {};
};

//----------------------------------------------------------------//
/* NAME */

TEST(AInstruction, DefaultName) {
    NamedInstruction instruction;

    ASSERT_EQ(instruction.name(), "[none]");
}

class AInstructionNameTest : public ::testing::TestWithParam<std::string> {};

TEST_P(AInstructionNameTest, KeepGivenName) {
    NamedInstruction instruction(GetParam());

    ASSERT_EQ(instruction.name(), GetParam());
}

TEST_P(AInstructionNameTest, KeepGivenNameThroughInterface) {
    std::unique_ptr<forge::rules::IInstruction> instruction = std::make_unique<NamedInstruction>(GetParam());

    ASSERT_EQ(instruction->name(), GetParam());
}

INSTANTIATE_TEST_SUITE_P(InputCases, AInstructionNameTest,
    ::testing::Values(
        "Testing",
        "S.O.S",
        "Please need help, fuck the unit_tests...",
        ""
    )
);

TEST(AInstruction, InterfacesDefaultName) {
    tests::tools::MockTrigger trigger(true);
    tests::tools::MockPreRule pre;
    tests::tools::MockRule rule;

    // the interfaces can't forward a name, they should all use the default one
    ASSERT_EQ(trigger.name(), "[none]");
    ASSERT_EQ(pre.name(), "[none]");
    ASSERT_EQ(rule.name(), "[none]");
}

//----------------------------------------------------------------//
/* OBSERVER */

/* every instruction is observed (potential leak from shared object) */
struct AInstructionLeakCase {
    std::string name;
    std::function<void()> generate;
    bool leak;
};
std::ostream& operator<<(std::ostream& os, const AInstructionLeakCase& c) {return os << c.name;}

class AInstructionLeakTest : public ::testing::TestWithParam<AInstructionLeakCase> {};

TEST_P(AInstructionLeakTest, DetectsLeak) {
    const AInstructionLeakCase& testCase = GetParam();

    ASSERT_GT(utils::security::observer::instances::Notifiers.size(), 0);
    testing::internal::CaptureStderr();

    // generate the things to observe
    testCase.generate();

    // Trigger & Reset MemoryLeakNotifer instances
    std::unique_ptr<utils::security::observer::INotifier>& notifier = utils::security::observer::instances::Notifiers[0];
    notifier->trigger();
    notifier->clear(true);

    // the instance ids depend on the whole run, only check the observed name
    std::string output = testing::internal::GetCapturedStderr();
    static const std::regex leakRegex(R"(^\[WARNING\] Memory leak detected \(origin: [^)]*\)\n-- At least one instance wasn't properly closed --\n(  [0-9]+ - IInstruction\n)+$)");
    if (testCase.leak) ASSERT_TRUE(std::regex_match(output, leakRegex)) << output;
    else ASSERT_EQ(output, "");
}

INSTANTIATE_TEST_SUITE_P(LeakCases, AInstructionLeakTest,
    ::testing::Values(
        AInstructionLeakCase{
            "Stack",
            [] {
                NamedInstruction instruction("Testing");
                (void)instruction;
            },
            false
        },
        AInstructionLeakCase{
            "UniquePtr",
            [] {
                std::unique_ptr<forge::rules::IInstruction> instruction = std::make_unique<NamedInstruction>("Testing");
                (void)instruction;
            },
            false
        },
        AInstructionLeakCase{
            "Interfaces",
            [] {
                std::unique_ptr<forge::rules::ITrigger> trigger = std::make_unique<tests::tools::MockTrigger>(true);
                std::unique_ptr<forge::rules::IPreRule> pre = std::make_unique<tests::tools::MockPreRule>();
                std::unique_ptr<forge::rules::IRule> rule = std::make_unique<tests::tools::MockRule>();
                (void)trigger;
                (void)pre;
                (void)rule;
            },
            false
        },
        AInstructionLeakCase{
            "Leak",
            [] {
                forge::rules::IInstruction* ptr = new NamedInstruction("Testing");
                (void)ptr;
            },
            true
        },
        AInstructionLeakCase{
            "LeakInterfaces",
            [] {
                forge::rules::ITrigger* trigger = new tests::tools::MockTrigger(true);
                forge::rules::IPreRule* pre = new tests::tools::MockPreRule();
                forge::rules::IRule* rule = new tests::tools::MockRule();
                (void)trigger;
                (void)pre;
                (void)rule;
            },
            true
        }
    ),
    [](const ::testing::TestParamInfo<AInstructionLeakCase>& info) {return info.param.name;}
);
