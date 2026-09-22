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
##  @file DefaultTrigger.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/triggers/DefaultTrigger.hpp"
#include "forge/rules/triggers/ITrigger.hpp"
#include "forge/rules/Rules.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <memory>
#include <string>
#include <tuple>

using DefaultTriggerTestParam = std::tuple<std::string, std::string>; // <bin, content>
class DefaultTriggerTest : public ::testing::TestWithParam<DefaultTriggerTestParam> {};

TEST(DefaultTrigger, Name) {
    forge::rules::DefaultTrigger trigger;

    ASSERT_EQ(trigger.name(), "[none]");
}

TEST(DefaultTrigger, Load) {
    forge::rules::DefaultTrigger trigger;
    libconfig::Config cfg;

    // any setting should be accepted
    ASSERT_NO_THROW(cfg.readString("default: {}; custom: {key = \"value\"; number = 2;};"));
    ASSERT_NO_THROW(trigger.load(cfg.getRoot()["default"]));
    ASSERT_NO_THROW(trigger.load(cfg.getRoot()["custom"]));
}

TEST_P(DefaultTriggerTest, NeverTrigger) {
    const auto& [bin, content] = GetParam();
    forge::rules::DefaultTrigger trigger;

    ASSERT_FALSE(trigger.trigger(bin, content));
}

TEST_P(DefaultTriggerTest, NeverTriggerThroughRules) {
    const auto& [bin, content] = GetParam();
    forge::rules::Rules rules;

    rules.push(std::unique_ptr<forge::rules::ITrigger>(new forge::rules::DefaultTrigger));
    ASSERT_FALSE(rules.trigger(bin, content));
}

INSTANTIATE_TEST_SUITE_P(InputCases, DefaultTriggerTest,
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
