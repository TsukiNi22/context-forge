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
##  @file LnRule.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/rules/LnRule.hpp"
#include "forge/rules/rules/IRule.hpp"
#include "forge/rules/Rules.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <memory>
#include <string>
#include <tuple>

using LnRuleTestParam = std::tuple<std::string, std::string>; // <bin, content>
class LnRuleTest : public ::testing::TestWithParam<LnRuleTestParam> {};

TEST(LnRule, Name) {
    forge::rules::LnRule rule;

    ASSERT_EQ(rule.name(), "[none]");
}

TEST(LnRule, Load) {
    forge::rules::LnRule rule;
    libconfig::Config cfg;

    // any setting should be accepted
    ASSERT_NO_THROW(cfg.readString("ln: {}; custom: {key = \"value\"; number = 2;};"));
    ASSERT_NO_THROW(rule.load(cfg.getRoot()["ln"]));
    ASSERT_NO_THROW(rule.load(cfg.getRoot()["custom"]));
}

TEST_P(LnRuleTest, Format) {
    const auto& [bin, input] = GetParam();
    forge::rules::LnRule rule;
    std::string content = input;

    ASSERT_NO_THROW(rule.format(bin, content));
}

TEST_P(LnRuleTest, Deterministic) {
    const auto& [bin, input] = GetParam();
    forge::rules::LnRule rule1, rule2;
    std::string content1 = input, content2 = input;

    // the same input should always produce the same output
    ASSERT_NO_THROW(rule1.format(bin, content1));
    ASSERT_NO_THROW(rule2.format(bin, content2));
    ASSERT_EQ(content1, content2);
}

TEST_P(LnRuleTest, SameResultThroughRules) {
    const auto& [bin, input] = GetParam();
    forge::rules::LnRule rule;
    forge::rules::Rules rules;
    std::string direct = input, through = input;

    // direct call
    ASSERT_NO_THROW(rule.format(bin, direct));

    // call through the rules
    rules.push(std::unique_ptr<forge::rules::IRule>(new forge::rules::LnRule));
    ASSERT_NO_THROW(rules.apply(bin, through));

    ASSERT_EQ(direct, through);
}

INSTANTIATE_TEST_SUITE_P(InputCases, LnRuleTest,
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
            ""
        )
    )
);
