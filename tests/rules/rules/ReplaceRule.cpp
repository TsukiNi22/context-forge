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
##  @file ReplaceRule.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/rules/ReplaceRule.hpp"
#include "tools/Config.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <type_traits>
#include <memory>
#include <string>

//----------------------------------------------------------------//
/* TRAITS */

TEST(ReplaceRule, Traits) {
    // an instruction should never be duplicated
    static_assert(!std::is_copy_constructible_v<forge::rules::ReplaceRule>);
    static_assert(!std::is_copy_assignable_v<forge::rules::ReplaceRule>);
    static_assert(!std::is_move_constructible_v<forge::rules::ReplaceRule>);
    static_assert(!std::is_move_assignable_v<forge::rules::ReplaceRule>);
    static_assert(std::is_base_of_v<forge::rules::IRule, forge::rules::ReplaceRule>);
}

TEST(ReplaceRule, Name) {
    forge::rules::ReplaceRule rule;
    std::unique_ptr<forge::rules::IRule> ptr = std::make_unique<forge::rules::ReplaceRule>();

    // name used inside the rules (.cfg)
    ASSERT_EQ(rule.name(), "replace");
    ASSERT_EQ(ptr->name(), "replace");
}

//----------------------------------------------------------------//
/* LOAD */

struct ReplaceRuleLoadCase {
    std::string name;
    std::string config; // libconfig content with a `replace` group
    bool valid;
};
std::ostream& operator<<(std::ostream& os, const ReplaceRuleLoadCase& c) {return os << c.name;}

class ReplaceRuleLoadTest : public ::testing::TestWithParam<ReplaceRuleLoadCase> {};

TEST_P(ReplaceRuleLoadTest, ValidateConfig) {
    const ReplaceRuleLoadCase& testCase = GetParam();
    forge::rules::ReplaceRule rule;
    tests::tools::Config cfg;

    const libconfig::Setting& s = cfg.parse(testCase.config, "replace");
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

INSTANTIATE_TEST_SUITE_P(ConfigCases, ReplaceRuleLoadTest,
    ::testing::Values(
        // valid
        ReplaceRuleLoadCase{"OnlyBy", "replace: {by = \"x\";};", true},
        ReplaceRuleLoadCase{"ByEmpty", "replace: {by = \"\";};", true},
        ReplaceRuleLoadCase{"Match", "replace: {match = [\"[0-9]+\"]; by = \"N\";};", true},
        ReplaceRuleLoadCase{"Eq", "replace: {eq = [\"foo\", \"bar\"]; by = \"baz\";};", true},
        ReplaceRuleLoadCase{"Full", "replace: {match = [\"[0-9]+\"]; eq = [\"foo\"]; by = \"<<INSERT>>\";};", true},
        ReplaceRuleLoadCase{"EmptyArrays", "replace: {match = []; eq = []; by = \"x\";};", true},
        ReplaceRuleLoadCase{"UnknownKey", "replace: {unknown = 2; by = \"x\";};", true},

        // invalid
        ReplaceRuleLoadCase{"Empty", "replace: {};", false},
        ReplaceRuleLoadCase{"MissingBy", "replace: {eq = [\"foo\"];};", false},
        ReplaceRuleLoadCase{"ByInt", "replace: {by = 2;};", false},
        ReplaceRuleLoadCase{"ByArray", "replace: {by = [\"x\"];};", false},
        ReplaceRuleLoadCase{"MatchString", "replace: {match = \"[0-9]+\"; by = \"N\";};", false},
        ReplaceRuleLoadCase{"MatchIntValues", "replace: {match = [1]; by = \"N\";};", false},
        ReplaceRuleLoadCase{"EqString", "replace: {eq = \"foo\"; by = \"bar\";};", false},
        ReplaceRuleLoadCase{"EqList", "replace: {eq = (\"foo\"); by = \"bar\";};", false},
        ReplaceRuleLoadCase{"EqBoolValues", "replace: {eq = [true]; by = \"bar\";};", false}
    ),
    [](const ::testing::TestParamInfo<ReplaceRuleLoadCase>& info) {return info.param.name;}
);

TEST(ReplaceRule, LoadInvalidRegex) {
    forge::rules::ReplaceRule rule;
    tests::tools::Config cfg;

    // an invalid regex should never be silently accepted
    ASSERT_ANY_THROW(rule.load(cfg.parse("replace: {match = [\"(\"]; by = \"x\";};", "replace")));
}

//----------------------------------------------------------------//
/* FORMAT */

struct ReplaceRuleCase {
    std::string name;
    std::string config; // libconfig content with a `replace` group
    std::string content;
    std::string expected;
};
std::ostream& operator<<(std::ostream& os, const ReplaceRuleCase& c) {return os << c.name;}

class ReplaceRuleTest : public ::testing::TestWithParam<ReplaceRuleCase> {};

TEST_P(ReplaceRuleTest, ProducesExpectedOutput) {
    const ReplaceRuleCase& testCase = GetParam();
    forge::rules::ReplaceRule rule;
    tests::tools::Config cfg;
    std::string content = testCase.content;

    ASSERT_NO_THROW(rule.load(cfg.parse(testCase.config, "replace")));
    rule.format("ls", content);
    ASSERT_EQ(content, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(FormatCases, ReplaceRuleTest,
    ::testing::Values(
        // nothing to do
        ReplaceRuleCase{"OnlyBy", "replace: {by = \"x\";};", "Testing\n", "Testing\n"},
        ReplaceRuleCase{"NoMatch", "replace: {match = [\"[0-9]+\"]; eq = [\"foo\"]; by = \"x\";};", "Testing", "Testing"},
        ReplaceRuleCase{"EmptyContent", "replace: {match = [\"[0-9]+\"]; eq = [\"foo\"]; by = \"x\";};", "", ""},

        // eq (exact string)
        ReplaceRuleCase{"Eq", "replace: {eq = [\"foo\"]; by = \"bar\";};", "foo foo baz", "bar bar baz"},
        ReplaceRuleCase{"EqMultiple", "replace: {eq = [\"foo\", \"baz\"]; by = \"_\";};", "foo bar baz", "_ bar _"},
        ReplaceRuleCase{"EqNoRegex", "replace: {eq = [\"a.c\"]; by = \"x\";};", "abc a.c", "abc x"},
        ReplaceRuleCase{"EqMultiLine", "replace: {eq = [\"\\n\"]; by = \" \";};", "a\nb\nc", "a b c"},
        ReplaceRuleCase{"EqDelete", "replace: {eq = [\" \"]; by = \"\";};", "a b c", "abc"},
        ReplaceRuleCase{"EqByContainsEq", "replace: {eq = [\"a\"]; by = \"aa\";};", "aXa", "aaXaa"},
        ReplaceRuleCase{"EqToken", "replace: {eq = [\"id\"]; by = \"[<INSERT>]\";};", "id=1 id=2", "[id]=1 [id]=2"},

        // match (regex)
        ReplaceRuleCase{"Match", "replace: {match = [\"[0-9]+\"]; by = \"N\";};", "a1 b22 c333", "aN bN cN"},
        ReplaceRuleCase{"MatchMultiple", "replace: {match = [\"[0-9]+\", \"[A-Z]\"]; by = \"_\";};", "a1 B2 c", "a_ __ c"},
        ReplaceRuleCase{"MatchDelete", "replace: {match = [\"\\\\s+\"]; by = \"\";};", "a  b\t c", "abc"},
        ReplaceRuleCase{"MatchToken", "replace: {match = [\"[0-9]+\"]; by = \"<<INSERT>>\";};", "a1 b22", "a<1> b<22>"},
        ReplaceRuleCase{"MatchTokenOnly", "replace: {match = [\"[a-z]+\"]; by = \"<INSERT>\";};", "abc def", "abc def"},
        ReplaceRuleCase{"MatchEmpty", "replace: {match = [\"x*\"]; by = \"-\";};", "ab", "-a-b-"},
        ReplaceRuleCase{"MatchWholeLine", "replace: {match = [\"^.*$\"]; by = \"[<INSERT>]\";};", "abc", "[abc]"},

        // order: every regex, then every exact string
        ReplaceRuleCase{"MatchBeforeEq", "replace: {match = [\"a\"]; eq = [\"bb\"]; by = \"b\";};", "aa", "b"}
    ),
    [](const ::testing::TestParamInfo<ReplaceRuleCase>& info) {return info.param.name;}
);

TEST(ReplaceRule, BinIndependent) {
    forge::rules::ReplaceRule rule;
    tests::tools::Config cfg;

    // the binary name should never change the result
    ASSERT_NO_THROW(rule.load(cfg.parse("replace: {eq = [\"foo\"]; by = \"bar\";};", "replace")));
    for (const std::string bin: {"ls", "cat", ""}) {
        std::string content = "foo";
        rule.format(bin, content);
        ASSERT_EQ(content, "bar");
    }
}
