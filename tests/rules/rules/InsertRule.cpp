/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 23/09/2026 by @author Tsukini

File Name:
##  @file InsertRule.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/rules/InsertRule.hpp"
#include "tools/Config.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <type_traits>
#include <optional>
#include <memory>
#include <string>

//----------------------------------------------------------------//
/* TRAITS */

TEST(InsertRule, Traits) {
    // an instruction should never be duplicated
    static_assert(!std::is_copy_constructible_v<forge::rules::InsertRule>);
    static_assert(!std::is_copy_assignable_v<forge::rules::InsertRule>);
    static_assert(!std::is_move_constructible_v<forge::rules::InsertRule>);
    static_assert(!std::is_move_assignable_v<forge::rules::InsertRule>);
    static_assert(std::is_base_of_v<forge::rules::IRule, forge::rules::InsertRule>);
}

TEST(InsertRule, Name) {
    forge::rules::InsertRule rule;
    std::unique_ptr<forge::rules::IRule> ptr = std::make_unique<forge::rules::InsertRule>();

    // name used inside the rules (.cfg)
    ASSERT_EQ(rule.name(), "insert");
    ASSERT_EQ(ptr->name(), "insert");
}

//----------------------------------------------------------------//
/* LOAD */

struct InsertRuleLoadCase {
    std::string name;
    std::string config; // libconfig content with a `insert` group
    bool valid;
};
std::ostream& operator<<(std::ostream& os, const InsertRuleLoadCase& c) {return os << c.name;}

class InsertRuleLoadTest : public ::testing::TestWithParam<InsertRuleLoadCase> {};

TEST_P(InsertRuleLoadTest, ValidateConfig) {
    const InsertRuleLoadCase& testCase = GetParam();
    forge::rules::InsertRule rule;
    tests::tools::Config cfg;

    const libconfig::Setting& s = cfg.parse(testCase.config, "insert");
    if (testCase.valid) {
        ASSERT_NO_THROW(rule.load(s));
        return;
    }
    try {
        rule.load(s);
        FAIL() << "Expected load to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), static_cast<utils::exception::InternalCode>(utils::exception::ExternalCode::Rules));
    }
}

INSTANTIATE_TEST_SUITE_P(ConfigCases, InsertRuleLoadTest,
    ::testing::Values(
        // valid
        InsertRuleLoadCase{"Empty", "insert: {};", true},
        InsertRuleLoadCase{"Before", "insert: {before = \"<\";};", true},
        InsertRuleLoadCase{"After", "insert: {after = \">\";};", true},
        InsertRuleLoadCase{"BeforeAfter", "insert: {before = \"<\"; after = \">\";};", true},
        InsertRuleLoadCase{"EmptyStrings", "insert: {before = \"\"; after = \"\";};", true},
        InsertRuleLoadCase{"UnknownKey", "insert: {unknown = 2;};", true},

        // invalid
        InsertRuleLoadCase{"BeforeInt", "insert: {before = 2;};", false},
        InsertRuleLoadCase{"BeforeBool", "insert: {before = true;};", false},
        InsertRuleLoadCase{"BeforeArray", "insert: {before = [\"<\"];};", false},
        InsertRuleLoadCase{"AfterInt", "insert: {after = 2;};", false},
        InsertRuleLoadCase{"AfterFloat", "insert: {after = 2.5;};", false},
        InsertRuleLoadCase{"AfterGroup", "insert: {after = {value = \">\";};};", false},
        InsertRuleLoadCase{"ValidBeforeInvalidAfter", "insert: {before = \"<\"; after = 2;};", false}
    ),
    [](const ::testing::TestParamInfo<InsertRuleLoadCase>& info) {return info.param.name;}
);

//----------------------------------------------------------------//
/* FORMAT */

struct InsertRuleCase {
    std::string name;
    std::optional<std::string> config; // libconfig content with a `insert` group (nullopt: no load)
    std::string content;
    std::string expected;
};
std::ostream& operator<<(std::ostream& os, const InsertRuleCase& c) {return os << c.name;}

class InsertRuleTest : public ::testing::TestWithParam<InsertRuleCase> {};

TEST_P(InsertRuleTest, ProducesExpectedOutput) {
    const InsertRuleCase& testCase = GetParam();
    forge::rules::InsertRule rule;
    tests::tools::Config cfg;
    std::string content = testCase.content;

    if (testCase.config.has_value())
        ASSERT_NO_THROW(rule.load(cfg.parse(*testCase.config, "insert")));
    rule.format("ls", content);
    ASSERT_EQ(content, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(FormatCases, InsertRuleTest,
    ::testing::Values(
        // nothing to do
        InsertRuleCase{"NoLoad", std::nullopt, "Testing", "Testing"},
        InsertRuleCase{"Empty", "insert: {};", "Testing", "Testing"},
        InsertRuleCase{"EmptyStrings", "insert: {before = \"\"; after = \"\";};", "Testing", "Testing"},

        // insertion
        InsertRuleCase{"Before", "insert: {before = \"<\";};", "Testing", "<Testing"},
        InsertRuleCase{"After", "insert: {after = \">\";};", "Testing", "Testing>"},
        InsertRuleCase{"BeforeAfter", "insert: {before = \"<\"; after = \">\";};", "Testing", "<Testing>"},
        InsertRuleCase{"EmptyContent", "insert: {before = \"<\"; after = \">\";};", "", "<>"},
        InsertRuleCase{"MultiLine", "insert: {before = \"# \";};", "a\nb", "# a\nb"},
        InsertRuleCase{"NewLines", "insert: {before = \"---\\n\"; after = \"\\n---\";};", "a\nb", "---\na\nb\n---"},
        InsertRuleCase{"Tags", "insert: {before = \"<output>\\n\"; after = \"</output>\\n\";};", "S.O.S\n", "<output>\nS.O.S\n</output>\n"},

        // no special sequence for the insert rule
        InsertRuleCase{"NoToken", "insert: {before = \"<INSERT>\";};", "Testing", "<INSERT>Testing"}
    ),
    [](const ::testing::TestParamInfo<InsertRuleCase>& info) {return info.param.name;}
);

TEST(InsertRule, AppliedEachTime) {
    forge::rules::InsertRule rule;
    tests::tools::Config cfg;
    std::string content = "Testing";

    // no state, each format insert again
    ASSERT_NO_THROW(rule.load(cfg.parse("insert: {before = \"<\"; after = \">\";};", "insert")));
    rule.format("ls", content);
    rule.format("ls", content);
    ASSERT_EQ(content, "<<Testing>>");
}

TEST(InsertRule, BinIndependent) {
    forge::rules::InsertRule rule;
    tests::tools::Config cfg;

    // the binary name should never change the result
    ASSERT_NO_THROW(rule.load(cfg.parse("insert: {before = \"<\"; after = \">\";};", "insert")));
    for (const std::string bin: {"ls", "cat", ""}) {
        std::string content = "Testing";
        rule.format(bin, content);
        ASSERT_EQ(content, "<Testing>");
    }
}
