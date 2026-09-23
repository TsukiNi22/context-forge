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
##  @file AnsiPreRule.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/pre-rules/AnsiPreRule.hpp"
#include "tools/Config.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <type_traits>
#include <memory>
#include <string>

#define ESC "\x1b"
#define BEL "\x07"

//----------------------------------------------------------------//
/* TRAITS */

TEST(AnsiPreRule, Traits) {
    // an instruction should never be duplicated
    static_assert(!std::is_copy_constructible_v<forge::rules::AnsiPreRule>);
    static_assert(!std::is_copy_assignable_v<forge::rules::AnsiPreRule>);
    static_assert(!std::is_move_constructible_v<forge::rules::AnsiPreRule>);
    static_assert(!std::is_move_assignable_v<forge::rules::AnsiPreRule>);
    static_assert(std::is_base_of_v<forge::rules::IPreRule, forge::rules::AnsiPreRule>);
}

TEST(AnsiPreRule, Name) {
    forge::rules::AnsiPreRule pre;
    std::unique_ptr<forge::rules::IPreRule> ptr = std::make_unique<forge::rules::AnsiPreRule>();

    // name used inside the rules (.cfg)
    ASSERT_EQ(pre.name(), "ansi");
    ASSERT_EQ(ptr->name(), "ansi");
}

//----------------------------------------------------------------//
/* LOAD */

struct AnsiPreRuleLoadCase {
    std::string name;
    std::string config; // libconfig content with a `ansi` setting
    bool valid;
};
std::ostream& operator<<(std::ostream& os, const AnsiPreRuleLoadCase& c) {return os << c.name;}

class AnsiPreRuleLoadTest : public ::testing::TestWithParam<AnsiPreRuleLoadCase> {};

TEST_P(AnsiPreRuleLoadTest, ValidateConfig) {
    const AnsiPreRuleLoadCase& testCase = GetParam();
    forge::rules::AnsiPreRule pre;
    tests::tools::Config cfg;

    const libconfig::Setting& s = cfg.parse(testCase.config, "ansi");
    if (testCase.valid) {
        ASSERT_NO_THROW(pre.load(s));
        return;
    }
    try {
        pre.load(s);
        FAIL() << "Expected load to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), static_cast<utils::exception::InternalCode>(utils::exception::ExternalCode::Rules));
    }
}

INSTANTIATE_TEST_SUITE_P(ConfigCases, AnsiPreRuleLoadTest,
    ::testing::Values(
        // valid
        AnsiPreRuleLoadCase{"True", "ansi = true;", true},
        AnsiPreRuleLoadCase{"False", "ansi = false;", true},

        // invalid
        AnsiPreRuleLoadCase{"Int", "ansi = 1;", false},
        AnsiPreRuleLoadCase{"Zero", "ansi = 0;", false},
        AnsiPreRuleLoadCase{"Float", "ansi = 1.0;", false},
        AnsiPreRuleLoadCase{"String", "ansi = \"true\";", false},
        AnsiPreRuleLoadCase{"Array", "ansi = [true];", false},
        AnsiPreRuleLoadCase{"Group", "ansi = {enable = true;};", false}
    ),
    [](const ::testing::TestParamInfo<AnsiPreRuleLoadCase>& info) {return info.param.name;}
);

//----------------------------------------------------------------//
/* FORMAT */

/* ansi = true -> keep the sequences, the content should always be untouched */
class AnsiPreRuleKeepTest : public ::testing::TestWithParam<std::string> {};

TEST_P(AnsiPreRuleKeepTest, WithoutLoadUntouched) {
    forge::rules::AnsiPreRule pre;
    std::string content = GetParam();

    // default: keep the ansi sequences
    pre.format("ls", content);
    ASSERT_EQ(content, GetParam());
}

TEST_P(AnsiPreRuleKeepTest, EnableUntouched) {
    forge::rules::AnsiPreRule pre;
    tests::tools::Config cfg;
    std::string content = GetParam();

    ASSERT_NO_THROW(pre.load(cfg.parse("ansi = true;", "ansi")));
    pre.format("ls", content);
    ASSERT_EQ(content, GetParam());
}

INSTANTIATE_TEST_SUITE_P(InputCases, AnsiPreRuleKeepTest,
    ::testing::Values(
        "Testing",
        "S.O.S",
        "Please need help, fuck the unit_tests...",
        ESC "[31mred" ESC "[0m",
        ESC "]8;;https://github.com" BEL "link" ESC "]8;;" BEL,
        ""
    )
);

/* ansi = false -> remove the sequences */
struct AnsiPreRuleStripCase {
    std::string name;
    std::string content;
    std::string expected;
};
std::ostream& operator<<(std::ostream& os, const AnsiPreRuleStripCase& c) {return os << c.name;}

class AnsiPreRuleStripTest : public ::testing::TestWithParam<AnsiPreRuleStripCase> {};

TEST_P(AnsiPreRuleStripTest, ProducesExpectedOutput) {
    const AnsiPreRuleStripCase& testCase = GetParam();
    forge::rules::AnsiPreRule pre;
    tests::tools::Config cfg;
    std::string content = testCase.content;

    ASSERT_NO_THROW(pre.load(cfg.parse("ansi = false;", "ansi")));
    pre.format("ls", content);
    ASSERT_EQ(content, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(StripCases, AnsiPreRuleStripTest,
    ::testing::Values(
        // without sequence
        AnsiPreRuleStripCase{"Plain", "Testing", "Testing"},
        AnsiPreRuleStripCase{"PlainMultiLine", "Testing\nS.O.S\n", "Testing\nS.O.S\n"},
        AnsiPreRuleStripCase{"Empty", "", ""},

        // CSI (ESC [ ... final byte)
        AnsiPreRuleStripCase{"Color", ESC "[31mred" ESC "[0m", "red"},
        AnsiPreRuleStripCase{"ColorMultipleParams", ESC "[1;38;2;46;204;113mbold" ESC "[m text", "bold text"},
        AnsiPreRuleStripCase{"ColorMultiLine", ESC "[32mok" ESC "[0m\n" ESC "[31mko" ESC "[0m", "ok\nko"},
        AnsiPreRuleStripCase{"Cursor", ESC "[2K" ESC "[1Gline", "line"},
        AnsiPreRuleStripCase{"ColorOnly", ESC "[0m", ""},
        AnsiPreRuleStripCase{"ColorMiddle", "a" ESC "[4mb" ESC "[24mc", "abc"},

        // OSC (ESC ] ... BEL | ESC \)
        AnsiPreRuleStripCase{"HyperlinkBel", ESC "]8;;https://github.com" BEL "link" ESC "]8;;" BEL, "link"},
        AnsiPreRuleStripCase{"HyperlinkSt", ESC "]8;;https://github.com" ESC "\\link" ESC "]8;;" ESC "\\", "link"},
        AnsiPreRuleStripCase{"Title", ESC "]0;title" BEL "after", "after"},

        // two chars sequence (ESC x)
        AnsiPreRuleStripCase{"SaveRestore", ESC "7saved" ESC "8", "saved"},

        // truncated sequence
        AnsiPreRuleStripCase{"LoneEsc", "abc" ESC, "abc"},
        AnsiPreRuleStripCase{"UnterminatedCsi", "abc" ESC "[31", "abc"},
        AnsiPreRuleStripCase{"UnterminatedOsc", "abc" ESC "]8;;link", "abc"},

        // other control char should be kept
        AnsiPreRuleStripCase{"KeepBel", "a" BEL "b", "a" BEL "b"},
        AnsiPreRuleStripCase{"KeepTab", "a\tb\r\n", "a\tb\r\n"}
    ),
    [](const ::testing::TestParamInfo<AnsiPreRuleStripCase>& info) {return info.param.name;}
);

TEST(AnsiPreRule, LoadLastValue) {
    forge::rules::AnsiPreRule pre;
    tests::tools::Config cfg1, cfg2;
    std::string content = ESC "[31mred" ESC "[0m";

    // the last load should override the previous one
    ASSERT_NO_THROW(pre.load(cfg1.parse("ansi = false;", "ansi")));
    ASSERT_NO_THROW(pre.load(cfg2.parse("ansi = true;", "ansi")));
    pre.format("ls", content);
    ASSERT_EQ(content, ESC "[31mred" ESC "[0m");
}

TEST(AnsiPreRule, BinIndependent) {
    forge::rules::AnsiPreRule pre;
    tests::tools::Config cfg;

    // the binary name should never change the result
    ASSERT_NO_THROW(pre.load(cfg.parse("ansi = false;", "ansi")));
    for (const std::string bin: {"ls", "cat", ""}) {
        std::string content = ESC "[31mred" ESC "[0m";
        pre.format(bin, content);
        ASSERT_EQ(content, "red");
    }
}
