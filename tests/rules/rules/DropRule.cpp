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
##  @file DropRule.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/rules/DropRule.hpp"
#include "tools/Config.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <type_traits>
#include <optional>
#include <memory>
#include <string>

#define CONTENT "info: a\nerror: b\ninfo: c\nwarn: d"

//----------------------------------------------------------------//
/* TRAITS */

TEST(DropRule, Traits) {
    // an instruction should never be duplicated
    static_assert(!std::is_copy_constructible_v<forge::rules::DropRule>);
    static_assert(!std::is_copy_assignable_v<forge::rules::DropRule>);
    static_assert(!std::is_move_constructible_v<forge::rules::DropRule>);
    static_assert(!std::is_move_assignable_v<forge::rules::DropRule>);
    static_assert(std::is_base_of_v<forge::rules::IRule, forge::rules::DropRule>);
}

TEST(DropRule, Name) {
    forge::rules::DropRule rule;
    std::unique_ptr<forge::rules::IRule> ptr = std::make_unique<forge::rules::DropRule>();

    // name used inside the rules (.cfg)
    ASSERT_EQ(rule.name(), "drop");
    ASSERT_EQ(ptr->name(), "drop");
}

//----------------------------------------------------------------//
/* LOAD */

struct DropRuleLoadCase {
    std::string name;
    std::string config; // libconfig content with a `drop` array
    bool valid;
};
std::ostream& operator<<(std::ostream& os, const DropRuleLoadCase& c) {return os << c.name;}

class DropRuleLoadTest : public ::testing::TestWithParam<DropRuleLoadCase> {};

TEST_P(DropRuleLoadTest, ValidateConfig) {
    const DropRuleLoadCase& testCase = GetParam();
    forge::rules::DropRule rule;
    tests::tools::Config cfg;

    const libconfig::Setting& s = cfg.parse(testCase.config, "drop");
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

INSTANTIATE_TEST_SUITE_P(ConfigCases, DropRuleLoadTest,
    ::testing::Values(
        // valid
        DropRuleLoadCase{"Empty", "drop = [];", true},
        DropRuleLoadCase{"Single", "drop = [\"^error\"];", true},
        DropRuleLoadCase{"Multiple", "drop = [\"^error\", \"warn\", \"[0-9]+\"];", true},

        // invalid
        DropRuleLoadCase{"String", "drop = \"^error\";", false},
        DropRuleLoadCase{"Int", "drop = 2;", false},
        DropRuleLoadCase{"Bool", "drop = true;", false},
        DropRuleLoadCase{"List", "drop = (\"^error\");", false},
        DropRuleLoadCase{"Group", "drop = {match = \"^error\";};", false},
        DropRuleLoadCase{"IntValues", "drop = [1, 2];", false},
        DropRuleLoadCase{"BoolValues", "drop = [true];", false}
    ),
    [](const ::testing::TestParamInfo<DropRuleLoadCase>& info) {return info.param.name;}
);

TEST(DropRule, LoadInvalidRegex) {
    forge::rules::DropRule rule;
    tests::tools::Config cfg;

    // an invalid regex should never be silently accepted
    ASSERT_ANY_THROW(rule.load(cfg.parse("drop = [\"(\"];", "drop")));
}

//----------------------------------------------------------------//
/* FORMAT */

struct DropRuleCase {
    std::string name;
    std::optional<std::string> config; // libconfig content with a `drop` array (nullopt: no load)
    std::string content;
    std::string expected;
};
std::ostream& operator<<(std::ostream& os, const DropRuleCase& c) {return os << c.name;}

class DropRuleTest : public ::testing::TestWithParam<DropRuleCase> {};

TEST_P(DropRuleTest, ProducesExpectedOutput) {
    const DropRuleCase& testCase = GetParam();
    forge::rules::DropRule rule;
    tests::tools::Config cfg;
    std::string content = testCase.content;

    if (testCase.config.has_value())
        ASSERT_NO_THROW(rule.load(cfg.parse(*testCase.config, "drop")));
    rule.format("ls", content);
    ASSERT_EQ(content, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(FormatCases, DropRuleTest,
    ::testing::Values(
        // nothing to do (content untouched, even the trailing new line)
        DropRuleCase{"NoLoad", std::nullopt, CONTENT, CONTENT},
        DropRuleCase{"Empty", "drop = [];", CONTENT, CONTENT},
        DropRuleCase{"EmptyTrailingNewLine", "drop = [];", "a\nb\n", "a\nb\n"},
        DropRuleCase{"NoMatch", "drop = [\"^debug\"];", CONTENT, CONTENT},

        // drop matching lines
        DropRuleCase{"Single", "drop = [\"^error\"];", CONTENT, "info: a\ninfo: c\nwarn: d"},
        DropRuleCase{"SingleMultipleLines", "drop = [\"^info\"];", CONTENT, "error: b\nwarn: d"},
        DropRuleCase{"Multiple", "drop = [\"^error\", \"^warn\"];", CONTENT, "info: a\ninfo: c"},
        DropRuleCase{"Search", "drop = [\"b\"];", CONTENT, "info: a\ninfo: c\nwarn: d"},
        DropRuleCase{"Regex", "drop = [\"^[a-z]+: [ab]$\"];", CONTENT, "info: c\nwarn: d"},
        DropRuleCase{"First", "drop = [\"a$\"];", CONTENT, "error: b\ninfo: c\nwarn: d"},
        DropRuleCase{"Last", "drop = [\"d$\"];", CONTENT, "info: a\nerror: b\ninfo: c"},
        DropRuleCase{"All", "drop = [\".*\"];", CONTENT, ""},
        DropRuleCase{"DuplicatePattern", "drop = [\"^error\", \"^error\"];", CONTENT, "info: a\ninfo: c\nwarn: d"},

        // empty lines
        DropRuleCase{"EmptyLines", "drop = [\"^$\"];", "a\n\nb\n", "a\nb"},
        DropRuleCase{"EmptyContent", "drop = [\"a\"];", "", ""},
        DropRuleCase{"EmptyContentMatchEmpty", "drop = [\"^$\"];", "", ""},
        DropRuleCase{"KeepEmptyLines", "drop = [\"a\"];", "a\n\nb", "\nb"}
    ),
    [](const ::testing::TestParamInfo<DropRuleCase>& info) {return info.param.name;}
);

TEST(DropRule, BinIndependent) {
    forge::rules::DropRule rule;
    tests::tools::Config cfg;

    // the binary name should never change the result
    ASSERT_NO_THROW(rule.load(cfg.parse("drop = [\"^error\"];", "drop")));
    for (const std::string bin: {"ls", "cat", ""}) {
        std::string content = CONTENT;
        rule.format(bin, content);
        ASSERT_EQ(content, "info: a\ninfo: c\nwarn: d");
    }
}

TEST(DropRule, LoadAppend) {
    forge::rules::DropRule rule;
    tests::tools::Config cfg1, cfg2;
    std::string content = CONTENT;

    // each load add its patterns to the previous one
    ASSERT_NO_THROW(rule.load(cfg1.parse("drop = [\"^error\"];", "drop")));
    ASSERT_NO_THROW(rule.load(cfg2.parse("drop = [\"^warn\"];", "drop")));
    rule.format("ls", content);
    ASSERT_EQ(content, "info: a\ninfo: c");
}
