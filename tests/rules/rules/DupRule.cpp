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
##  @file DupRule.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/rules/DupRule.hpp"
#include "tools/Config.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <type_traits>
#include <optional>
#include <memory>
#include <string>

#define CONTENT "x\na\nx\nb\nx"

//----------------------------------------------------------------//
/* TRAITS */

TEST(DupRule, Traits) {
    // an instruction should never be duplicated
    static_assert(!std::is_copy_constructible_v<forge::rules::DupRule>);
    static_assert(!std::is_copy_assignable_v<forge::rules::DupRule>);
    static_assert(!std::is_move_constructible_v<forge::rules::DupRule>);
    static_assert(!std::is_move_assignable_v<forge::rules::DupRule>);
    static_assert(std::is_base_of_v<forge::rules::IRule, forge::rules::DupRule>);
}

TEST(DupRule, Name) {
    forge::rules::DupRule rule;
    std::unique_ptr<forge::rules::IRule> ptr = std::make_unique<forge::rules::DupRule>();

    // name used inside the rules (.cfg)
    ASSERT_EQ(rule.name(), "dup");
    ASSERT_EQ(ptr->name(), "dup");
}

//----------------------------------------------------------------//
/* LOAD */

struct DupRuleLoadCase {
    std::string name;
    std::string config; // libconfig content with a `dup` group
    bool valid;
};
std::ostream& operator<<(std::ostream& os, const DupRuleLoadCase& c) {return os << c.name;}

class DupRuleLoadTest : public ::testing::TestWithParam<DupRuleLoadCase> {};

TEST_P(DupRuleLoadTest, ValidateConfig) {
    const DupRuleLoadCase& testCase = GetParam();
    forge::rules::DupRule rule;
    tests::tools::Config cfg;

    const libconfig::Setting& s = cfg.parse(testCase.config, "dup");
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

INSTANTIATE_TEST_SUITE_P(ConfigCases, DupRuleLoadTest,
    ::testing::Values(
        // valid
        DupRuleLoadCase{"Empty", "dup: {};", true},
        DupRuleLoadCase{"Match", "dup: {match = [\"^warn\"];};", true},
        DupRuleLoadCase{"MatchEmpty", "dup: {match = [];};", true},
        DupRuleLoadCase{"Eq", "dup: {eq = [\"x\", \"y\"];};", true},
        DupRuleLoadCase{"Keep", "dup: {eq = [\"x\"]; keep = 2;};", true},
        DupRuleLoadCase{"Invert", "dup: {eq = [\"x\"]; invert = true;};", true},
        DupRuleLoadCase{"Full", "dup: {match = [\"^warn\"]; eq = [\"x\"]; keep = 3; invert = false;};", true},
        DupRuleLoadCase{"UnknownKey", "dup: {unknown = 2;};", true},

        // invalid
        DupRuleLoadCase{"MatchString", "dup: {match = \"^warn\";};", false},
        DupRuleLoadCase{"MatchList", "dup: {match = (\"^warn\");};", false},
        DupRuleLoadCase{"MatchIntValues", "dup: {match = [1];};", false},
        DupRuleLoadCase{"EqString", "dup: {eq = \"x\";};", false},
        DupRuleLoadCase{"EqBoolValues", "dup: {eq = [true];};", false},
        DupRuleLoadCase{"KeepString", "dup: {keep = \"1\";};", false},
        DupRuleLoadCase{"KeepFloat", "dup: {keep = 1.5;};", false},
        DupRuleLoadCase{"KeepBool", "dup: {keep = true;};", false},
        DupRuleLoadCase{"InvertInt", "dup: {invert = 1;};", false},
        DupRuleLoadCase{"InvertString", "dup: {invert = \"true\";};", false}
    ),
    [](const ::testing::TestParamInfo<DupRuleLoadCase>& info) {return info.param.name;}
);

TEST(DupRule, LoadInvalidRegex) {
    forge::rules::DupRule rule;
    tests::tools::Config cfg;

    // an invalid regex should never be silently accepted
    ASSERT_ANY_THROW(rule.load(cfg.parse("dup: {match = [\"(\"];};", "dup")));
}

//----------------------------------------------------------------//
/* FORMAT */

struct DupRuleCase {
    std::string name;
    std::optional<std::string> config; // libconfig content with a `dup` group (nullopt: no load)
    std::string content;
    std::string expected;
};
std::ostream& operator<<(std::ostream& os, const DupRuleCase& c) {return os << c.name;}

class DupRuleTest : public ::testing::TestWithParam<DupRuleCase> {};

TEST_P(DupRuleTest, ProducesExpectedOutput) {
    const DupRuleCase& testCase = GetParam();
    forge::rules::DupRule rule;
    tests::tools::Config cfg;
    std::string content = testCase.content;

    if (testCase.config.has_value())
        ASSERT_NO_THROW(rule.load(cfg.parse(*testCase.config, "dup")));
    rule.format("ls", content);
    ASSERT_EQ(content, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(FormatCases, DupRuleTest,
    ::testing::Values(
        // nothing to do (content untouched, even the trailing new line)
        DupRuleCase{"NoLoad", std::nullopt, CONTENT, CONTENT},
        DupRuleCase{"Empty", "dup: {};", CONTENT, CONTENT},
        DupRuleCase{"OnlyKeep", "dup: {keep = 1; invert = true;};", CONTENT, CONTENT},
        DupRuleCase{"EmptyTrailingNewLine", "dup: {};", "x\nx\n", "x\nx\n"},
        DupRuleCase{"NoDuplicate", "dup: {eq = [\"x\"];};", "x\na\nb", "x\na\nb"},
        DupRuleCase{"NoMatch", "dup: {eq = [\"z\"];};", CONTENT, CONTENT},

        // eq (exact line)
        DupRuleCase{"Eq", "dup: {eq = [\"x\"];};", CONTENT, "x\na\nb"},
        DupRuleCase{"EqExact", "dup: {eq = [\"x\"];};", "x\nxx\nx \nx", "x\nxx\nx "},
        DupRuleCase{"EqMultipleGroups", "dup: {eq = [\"x\", \"y\"];};", "x\ny\nx\ny\nz", "x\ny\nz"},

        // match (regex_search)
        DupRuleCase{"Match", "dup: {match = [\"^warn\"];};", "warn: 1\ninfo\nwarn: 2\nwarn: 3", "warn: 1\ninfo"},
        DupRuleCase{"MatchMultipleGroups", "dup: {match = [\"^a\", \"^b\"];};", "a1\nb1\na2\nb2", "a1\nb1"},
        DupRuleCase{"MatchPriority", "dup: {match = [\"x\"]; eq = [\"x\"];};", "x\nax\nx", "x"},
        DupRuleCase{"MatchAndEq", "dup: {match = [\"^a\"]; eq = [\"b\"];};", "a1\nb\na2\nb\nc", "a1\nb\nc"},

        // keep
        DupRuleCase{"Keep2", "dup: {eq = [\"x\"]; keep = 2;};", CONTENT, "x\na\nx\nb"},
        DupRuleCase{"KeepExact", "dup: {eq = [\"x\"]; keep = 3;};", CONTENT, CONTENT},
        DupRuleCase{"KeepOverMatches", "dup: {eq = [\"x\"]; keep = 10;};", CONTENT, CONTENT},
        DupRuleCase{"KeepZero", "dup: {eq = [\"x\"]; keep = 0;};", CONTENT, "a\nb"},

        // invert (keep the last ones)
        DupRuleCase{"Invert", "dup: {eq = [\"x\"]; invert = true;};", CONTENT, "a\nb\nx"},
        DupRuleCase{"InvertFalse", "dup: {eq = [\"x\"]; invert = false;};", CONTENT, "x\na\nb"},
        DupRuleCase{"InvertKeep2", "dup: {eq = [\"x\"]; keep = 2; invert = true;};", CONTENT, "a\nx\nb\nx"},
        DupRuleCase{"InvertKeepOverMatches", "dup: {eq = [\"x\"]; keep = 10; invert = true;};", CONTENT, CONTENT},
        DupRuleCase{"InvertKeepZero", "dup: {eq = [\"x\"]; keep = 0; invert = true;};", CONTENT, "a\nb"},
        DupRuleCase{"InvertMultipleGroups", "dup: {eq = [\"x\", \"y\"]; invert = true;};", "x\ny\nx\ny\nz", "x\ny\nz"},

        // empty lines
        DupRuleCase{"EmptyLines", "dup: {eq = [\"\"];};", "a\n\n\nb\n", "a\n\nb"},
        DupRuleCase{"EmptyContent", "dup: {eq = [\"x\"];};", "", ""}
    ),
    [](const ::testing::TestParamInfo<DupRuleCase>& info) {return info.param.name;}
);

TEST(DupRule, BinIndependent) {
    forge::rules::DupRule rule;
    tests::tools::Config cfg;

    // the binary name should never change the result
    ASSERT_NO_THROW(rule.load(cfg.parse("dup: {eq = [\"x\"];};", "dup")));
    for (const std::string bin: {"ls", "cat", ""}) {
        std::string content = CONTENT;
        rule.format(bin, content);
        ASSERT_EQ(content, "x\na\nb");
    }
}

TEST(DupRule, Reusable) {
    forge::rules::DupRule rule;
    tests::tools::Config cfg;

    // the matches count should be reset between two format
    ASSERT_NO_THROW(rule.load(cfg.parse("dup: {eq = [\"x\"];};", "dup")));
    std::string first = CONTENT, second = CONTENT;
    rule.format("ls", first);
    rule.format("ls", second);
    ASSERT_EQ(first, "x\na\nb");
    ASSERT_EQ(second, "x\na\nb");
}
