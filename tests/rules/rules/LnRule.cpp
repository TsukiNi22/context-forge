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
##  @file LnRule.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/rules/LnRule.hpp"
#include "tools/Config.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <type_traits>
#include <optional>
#include <memory>
#include <string>

#define CONTENT "a\nb\nc\nd\ne"

//----------------------------------------------------------------//
/* TRAITS */

TEST(LnRule, Traits) {
    // an instruction should never be duplicated
    static_assert(!std::is_copy_constructible_v<forge::rules::LnRule>);
    static_assert(!std::is_copy_assignable_v<forge::rules::LnRule>);
    static_assert(!std::is_move_constructible_v<forge::rules::LnRule>);
    static_assert(!std::is_move_assignable_v<forge::rules::LnRule>);
    static_assert(std::is_base_of_v<forge::rules::IRule, forge::rules::LnRule>);
}

TEST(LnRule, Name) {
    forge::rules::LnRule rule;
    std::unique_ptr<forge::rules::IRule> ptr = std::make_unique<forge::rules::LnRule>();

    // name used inside the rules (.cfg)
    ASSERT_EQ(rule.name(), "ln");
    ASSERT_EQ(ptr->name(), "ln");
}

//----------------------------------------------------------------//
/* LOAD */

struct LnRuleLoadCase {
    std::string name;
    std::string config; // libconfig content with a `ln` group
    bool valid;
};
std::ostream& operator<<(std::ostream& os, const LnRuleLoadCase& c) {return os << c.name;}

class LnRuleLoadTest : public ::testing::TestWithParam<LnRuleLoadCase> {};

TEST_P(LnRuleLoadTest, ValidateConfig) {
    const LnRuleLoadCase& testCase = GetParam();
    forge::rules::LnRule rule;
    tests::tools::Config cfg;

    const libconfig::Setting& s = cfg.parse(testCase.config, "ln");
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

INSTANTIATE_TEST_SUITE_P(ConfigCases, LnRuleLoadTest,
    ::testing::Values(
        // valid
        LnRuleLoadCase{"Empty", "ln: {};", true},
        LnRuleLoadCase{"Head", "ln: {head = 2;};", true},
        LnRuleLoadCase{"HeadNegative", "ln: {head = -2;};", true},
        LnRuleLoadCase{"HeadZero", "ln: {head = 0;};", true},
        LnRuleLoadCase{"Tail", "ln: {tail = 2;};", true},
        LnRuleLoadCase{"TailNegative", "ln: {tail = -2;};", true},
        LnRuleLoadCase{"HeadTail", "ln: {head = 2; tail = 1;};", true},
        LnRuleLoadCase{"UnknownKey", "ln: {unknown = 2;};", true},

        // invalid
        LnRuleLoadCase{"HeadString", "ln: {head = \"2\";};", false},
        LnRuleLoadCase{"HeadFloat", "ln: {head = 2.5;};", false},
        LnRuleLoadCase{"HeadBool", "ln: {head = true;};", false},
        LnRuleLoadCase{"HeadArray", "ln: {head = [2];};", false},
        LnRuleLoadCase{"HeadInt64", "ln: {head = 2L;};", false},
        LnRuleLoadCase{"TailString", "ln: {tail = \"2\";};", false},
        LnRuleLoadCase{"TailFloat", "ln: {tail = 2.5;};", false},
        LnRuleLoadCase{"TailBool", "ln: {tail = false;};", false},
        LnRuleLoadCase{"ValidHeadInvalidTail", "ln: {head = 2; tail = \"1\";};", false}
    ),
    [](const ::testing::TestParamInfo<LnRuleLoadCase>& info) {return info.param.name;}
);

//----------------------------------------------------------------//
/* FORMAT */

struct LnRuleCase {
    std::string name;
    std::optional<std::string> config; // libconfig content with a `ln` group (nullopt: no load)
    std::string content;
    std::string expected;
};
std::ostream& operator<<(std::ostream& os, const LnRuleCase& c) {return os << c.name;}

class LnRuleTest : public ::testing::TestWithParam<LnRuleCase> {};

TEST_P(LnRuleTest, ProducesExpectedOutput) {
    const LnRuleCase& testCase = GetParam();
    forge::rules::LnRule rule;
    tests::tools::Config cfg;
    std::string content = testCase.content;

    if (testCase.config.has_value())
        ASSERT_NO_THROW(rule.load(cfg.parse(*testCase.config, "ln")));
    rule.format("ls", content);
    ASSERT_EQ(content, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(FormatCases, LnRuleTest,
    ::testing::Values(
        // nothing to do (content untouched, even the trailing new line)
        LnRuleCase{"NoLoad", std::nullopt, CONTENT, CONTENT},
        LnRuleCase{"Empty", "ln: {};", CONTENT, CONTENT},
        LnRuleCase{"EmptyTrailingNewLine", "ln: {};", "a\nb\n", "a\nb\n"},

        // head
        LnRuleCase{"Head", "ln: {head = 2;};", CONTENT, "a\nb"},
        LnRuleCase{"HeadOne", "ln: {head = 1;};", CONTENT, "a"},
        LnRuleCase{"HeadZero", "ln: {head = 0;};", CONTENT, ""},
        LnRuleCase{"HeadExact", "ln: {head = 5;};", CONTENT, CONTENT},
        LnRuleCase{"HeadOverContent", "ln: {head = 10;};", CONTENT, CONTENT},
        LnRuleCase{"HeadNegative", "ln: {head = -2;};", CONTENT, "c\nd\ne"},
        LnRuleCase{"HeadNegativeExact", "ln: {head = -5;};", CONTENT, ""},
        LnRuleCase{"HeadNegativeOverContent", "ln: {head = -10;};", CONTENT, ""},
        LnRuleCase{"HeadTrailingNewLine", "ln: {head = 1;};", "a\nb\n", "a"},
        LnRuleCase{"HeadNoNewLine", "ln: {head = 1;};", "abc", "abc"},
        LnRuleCase{"HeadEmptyContent", "ln: {head = 2;};", "", ""},

        // tail
        LnRuleCase{"Tail", "ln: {tail = 2;};", CONTENT, "d\ne"},
        LnRuleCase{"TailOne", "ln: {tail = 1;};", CONTENT, "e"},
        LnRuleCase{"TailZero", "ln: {tail = 0;};", CONTENT, ""},
        LnRuleCase{"TailExact", "ln: {tail = 5;};", CONTENT, CONTENT},
        LnRuleCase{"TailOverContent", "ln: {tail = 10;};", CONTENT, CONTENT},
        LnRuleCase{"TailNegative", "ln: {tail = -2;};", CONTENT, "a\nb\nc"},
        LnRuleCase{"TailNegativeExact", "ln: {tail = -5;};", CONTENT, ""},
        LnRuleCase{"TailNegativeOverContent", "ln: {tail = -10;};", CONTENT, ""},
        LnRuleCase{"TailNoNewLine", "ln: {tail = 1;};", "abc", "abc"},
        LnRuleCase{"TailEmptyContent", "ln: {tail = 2;};", "", ""},

        // head applied before tail
        LnRuleCase{"HeadTail", "ln: {head = 4; tail = 2;};", CONTENT, "c\nd"},
        LnRuleCase{"HeadNegativeTailNegative", "ln: {head = -1; tail = -1;};", CONTENT, "b\nc\nd"},
        LnRuleCase{"HeadTailNegative", "ln: {head = 3; tail = -1;};", CONTENT, "a\nb"},
        LnRuleCase{"HeadNegativeTail", "ln: {head = -3; tail = 1;};", CONTENT, "e"},
        LnRuleCase{"HeadTailEmpty", "ln: {head = 1; tail = -1;};", CONTENT, ""},

        // empty lines are lines too
        LnRuleCase{"EmptyLines", "ln: {head = 2;};", "\n\n\n", "\n"},
        LnRuleCase{"EmptyLinesTail", "ln: {tail = -1;};", "a\n\nb", "a\n"}
    ),
    [](const ::testing::TestParamInfo<LnRuleCase>& info) {return info.param.name;}
);

TEST(LnRule, BinIndependent) {
    forge::rules::LnRule rule;
    tests::tools::Config cfg;

    // the binary name should never change the result
    ASSERT_NO_THROW(rule.load(cfg.parse("ln: {head = 2;};", "ln")));
    for (const std::string bin: {"ls", "cat", ""}) {
        std::string content = CONTENT;
        rule.format(bin, content);
        ASSERT_EQ(content, "a\nb");
    }
}

TEST(LnRule, Reusable) {
    forge::rules::LnRule rule;
    tests::tools::Config cfg;

    // no state should be kept between two format
    ASSERT_NO_THROW(rule.load(cfg.parse("ln: {head = -1;};", "ln")));
    std::string first = CONTENT, second = CONTENT;
    rule.format("ls", first);
    rule.format("ls", second);
    ASSERT_EQ(first, "b\nc\nd\ne");
    ASSERT_EQ(second, "b\nc\nd\ne");
}
