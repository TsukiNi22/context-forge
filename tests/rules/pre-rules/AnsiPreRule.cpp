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
##  @file AnsiPreRule.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/pre-rules/AnsiPreRule.hpp"
#include "forge/rules/pre-rules/IPreRule.hpp"
#include "forge/rules/Rules.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <memory>
#include <string>
#include <tuple>

using AnsiPreRuleTestParam = std::tuple<std::string, std::string>; // <bin, content>
class AnsiPreRuleTest : public ::testing::TestWithParam<AnsiPreRuleTestParam> {};

TEST(AnsiPreRule, Name) {
    forge::rules::AnsiPreRule pre;

    ASSERT_EQ(pre.name(), "[none]");
}

TEST(AnsiPreRule, Load) {
    forge::rules::AnsiPreRule pre;
    libconfig::Config cfg;

    // any setting should be accepted
    ASSERT_NO_THROW(cfg.readString("ansi: {}; custom: {key = \"value\"; number = 2;};"));
    ASSERT_NO_THROW(pre.load(cfg.getRoot()["ansi"]));
    ASSERT_NO_THROW(pre.load(cfg.getRoot()["custom"]));
}

/* content without any ansi sequence should never be edited */
TEST_P(AnsiPreRuleTest, KeepPlainContent) {
    const auto& [bin, input] = GetParam();
    forge::rules::AnsiPreRule pre;
    std::string content = input;

    ASSERT_NO_THROW(pre.format(bin, content));
    ASSERT_EQ(content, input);
}

TEST_P(AnsiPreRuleTest, Idempotent) {
    const auto& [bin, input] = GetParam();
    forge::rules::AnsiPreRule pre;
    std::string once = input, twice = input;

    // applying the pre-rule twice should give the same result as once
    ASSERT_NO_THROW(pre.format(bin, once));
    ASSERT_NO_THROW(pre.format(bin, twice));
    ASSERT_NO_THROW(pre.format(bin, twice));
    ASSERT_EQ(once, twice);
}

TEST_P(AnsiPreRuleTest, KeepPlainContentThroughRules) {
    const auto& [bin, input] = GetParam();
    forge::rules::Rules rules;
    std::string content = input;

    rules.push(std::unique_ptr<forge::rules::IPreRule>(new forge::rules::AnsiPreRule));
    ASSERT_NO_THROW(rules.apply(bin, content));
    ASSERT_EQ(content, input);
}

INSTANTIATE_TEST_SUITE_P(InputCases, AnsiPreRuleTest,
    ::testing::Combine(
        ::testing::Values(
            "ls",
            "/usr/bin/cat",
            ""
        ),
        ::testing::Values(
            "Testing",
            "S.O.S",
            "Please need help, fuck the unit_tests...",
            "multi\nline\n",
            "tab\tand \\backslash",
            ""
        )
    )
);
