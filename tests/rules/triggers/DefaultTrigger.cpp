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
##  @file DefaultTrigger.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/triggers/DefaultTrigger.hpp"
#include "tools/Config.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <type_traits>
#include <memory>
#include <string>

//----------------------------------------------------------------//
/* TRAITS */

TEST(DefaultTrigger, Traits) {
    // an instruction should never be duplicated
    static_assert(!std::is_copy_constructible_v<forge::rules::DefaultTrigger>);
    static_assert(!std::is_copy_assignable_v<forge::rules::DefaultTrigger>);
    static_assert(!std::is_move_constructible_v<forge::rules::DefaultTrigger>);
    static_assert(!std::is_move_assignable_v<forge::rules::DefaultTrigger>);
    static_assert(std::is_base_of_v<forge::rules::ITrigger, forge::rules::DefaultTrigger>);
}

TEST(DefaultTrigger, Name) {
    forge::rules::DefaultTrigger trigger;
    std::unique_ptr<forge::rules::ITrigger> ptr = std::make_unique<forge::rules::DefaultTrigger>();

    // name used inside the rules (.cfg)
    ASSERT_EQ(trigger.name(), "trigger");
    ASSERT_EQ(ptr->name(), "trigger");
}

//----------------------------------------------------------------//
/* LOAD */

struct DefaultTriggerLoadCase {
    std::string name;
    std::string config; // libconfig content with a `trigger` group
    bool valid;
};
std::ostream& operator<<(std::ostream& os, const DefaultTriggerLoadCase& c) {return os << c.name;}

class DefaultTriggerLoadTest : public ::testing::TestWithParam<DefaultTriggerLoadCase> {};

TEST_P(DefaultTriggerLoadTest, ValidateConfig) {
    const DefaultTriggerLoadCase& testCase = GetParam();
    forge::rules::DefaultTrigger trigger;
    tests::tools::Config cfg;

    const libconfig::Setting& s = cfg.parse(testCase.config, "trigger");
    if (testCase.valid) {
        ASSERT_NO_THROW(trigger.load(s));
        return;
    }
    try {
        trigger.load(s);
        FAIL() << "Expected load to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), static_cast<utils::exception::InternalCode>(utils::exception::ExternalCode::Rules));
    }
}

INSTANTIATE_TEST_SUITE_P(ConfigCases, DefaultTriggerLoadTest,
    ::testing::Values(
        // valid
        DefaultTriggerLoadCase{"Empty", "trigger: {};", true},
        DefaultTriggerLoadCase{"Bin", "trigger: {bin = [\"ls\"];};", true},
        DefaultTriggerLoadCase{"BinMultiple", "trigger: {bin = [\"ls\", \"cat\", \"echo\"];};", true},
        DefaultTriggerLoadCase{"BinEmpty", "trigger: {bin = [];};", true},
        DefaultTriggerLoadCase{"Contains", "trigger: {contains = \"error\";};", true},
        DefaultTriggerLoadCase{"Match", "trigger: {match = \"^ok$\";};", true},
        DefaultTriggerLoadCase{"Full", "trigger: {bin = [\"ls\"]; contains = \"error\"; match = \"^ok$\";};", true},
        DefaultTriggerLoadCase{"UnknownKey", "trigger: {unknown = 2;};", true},

        // invalid
        DefaultTriggerLoadCase{"BinString", "trigger: {bin = \"ls\";};", false},
        DefaultTriggerLoadCase{"BinInt", "trigger: {bin = 2;};", false},
        DefaultTriggerLoadCase{"BinList", "trigger: {bin = (\"ls\");};", false},
        DefaultTriggerLoadCase{"BinGroup", "trigger: {bin = {ls = true;};};", false},
        DefaultTriggerLoadCase{"BinIntValues", "trigger: {bin = [1, 2];};", false},
        DefaultTriggerLoadCase{"BinBoolValues", "trigger: {bin = [true];};", false},
        DefaultTriggerLoadCase{"ContainsInt", "trigger: {contains = 2;};", false},
        DefaultTriggerLoadCase{"ContainsBool", "trigger: {contains = true;};", false},
        DefaultTriggerLoadCase{"ContainsArray", "trigger: {contains = [\"error\"];};", false},
        DefaultTriggerLoadCase{"MatchInt", "trigger: {match = 2;};", false},
        DefaultTriggerLoadCase{"MatchFloat", "trigger: {match = 2.5;};", false},
        DefaultTriggerLoadCase{"MatchArray", "trigger: {match = [\"^ok$\"];};", false}
    ),
    [](const ::testing::TestParamInfo<DefaultTriggerLoadCase>& info) {return info.param.name;}
);

TEST(DefaultTrigger, LoadInvalidRegex) {
    forge::rules::DefaultTrigger trigger;
    tests::tools::Config cfg;

    // an invalid regex should never be silently accepted
    ASSERT_ANY_THROW(trigger.load(cfg.parse("trigger: {contains = \"(\";};", "trigger")));
    ASSERT_ANY_THROW(trigger.load(cfg.parse("trigger: {match = \"[a-\";};", "trigger")));
}

//----------------------------------------------------------------//
/* TRIGGER */

struct DefaultTriggerCase {
    std::string name;
    std::string config; // libconfig content with a `trigger` group
    std::string bin;
    std::string content;
    bool expected;
};
std::ostream& operator<<(std::ostream& os, const DefaultTriggerCase& c) {return os << c.name;}

class DefaultTriggerTest : public ::testing::TestWithParam<DefaultTriggerCase> {};

TEST_P(DefaultTriggerTest, ProducesExpectedResult) {
    const DefaultTriggerCase& testCase = GetParam();
    forge::rules::DefaultTrigger trigger;
    tests::tools::Config cfg;

    ASSERT_NO_THROW(trigger.load(cfg.parse(testCase.config, "trigger")));
    ASSERT_EQ(trigger.trigger(testCase.bin, testCase.content), testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(TriggerCases, DefaultTriggerTest,
    ::testing::Values(
        // nothing given: trigger on all
        DefaultTriggerCase{"EmptyAll", "trigger: {};", "ls", "Testing", true},
        DefaultTriggerCase{"EmptyAllEmptyInput", "trigger: {};", "", "", true},
        DefaultTriggerCase{"BinEmptyAll", "trigger: {bin = [];};", "ls", "Testing", true},

        // bin
        DefaultTriggerCase{"BinFound", "trigger: {bin = [\"ls\"];};", "ls", "Testing", true},
        DefaultTriggerCase{"BinFoundMultiple", "trigger: {bin = [\"ls\", \"cat\"];};", "cat", "Testing", true},
        DefaultTriggerCase{"BinNotFound", "trigger: {bin = [\"ls\", \"cat\"];};", "echo", "Testing", false},
        DefaultTriggerCase{"BinExact", "trigger: {bin = [\"ls\"];};", "lsblk", "Testing", false},
        DefaultTriggerCase{"BinCaseSensitive", "trigger: {bin = [\"ls\"];};", "LS", "Testing", false},
        DefaultTriggerCase{"BinEmptyName", "trigger: {bin = [\"ls\"];};", "", "Testing", false},

        // contains (regex_search)
        DefaultTriggerCase{"ContainsFound", "trigger: {contains = \"error\";};", "make", "an error occured", true},
        DefaultTriggerCase{"ContainsNotFound", "trigger: {contains = \"error\";};", "make", "all good", false},
        DefaultTriggerCase{"ContainsRegex", "trigger: {contains = \"[0-9]+ failed\";};", "ctest", "12 passed, 3 failed", true},
        DefaultTriggerCase{"ContainsMultiLine", "trigger: {contains = \"warning\";};", "make", "line 1\nwarning: unused\nline 3", true},
        DefaultTriggerCase{"ContainsEmptyContent", "trigger: {contains = \"error\";};", "make", "", false},

        // match (regex_match, whole content)
        DefaultTriggerCase{"MatchFound", "trigger: {match = \"ok\";};", "echo", "ok", true},
        DefaultTriggerCase{"MatchPartial", "trigger: {match = \"ok\";};", "echo", "not ok", false},
        DefaultTriggerCase{"MatchTrailingNewLine", "trigger: {match = \"ok\";};", "echo", "ok\n", false},
        DefaultTriggerCase{"MatchRegex", "trigger: {match = \"[a-z]+ [0-9]+\";};", "echo", "abc 123", true},

        // combination: any of them is enough
        DefaultTriggerCase{"CombinedBin", "trigger: {bin = [\"ls\"]; contains = \"error\";};", "ls", "all good", true},
        DefaultTriggerCase{"CombinedContains", "trigger: {bin = [\"ls\"]; contains = \"error\";};", "cat", "an error", true},
        DefaultTriggerCase{"CombinedMatch", "trigger: {bin = [\"ls\"]; match = \"ok\";};", "cat", "ok", true},
        DefaultTriggerCase{"CombinedNone", "trigger: {bin = [\"ls\"]; contains = \"error\"; match = \"ok\";};", "cat", "all good", false},
        DefaultTriggerCase{"BinEmptyWithContains", "trigger: {bin = []; contains = \"error\";};", "ls", "all good", false}
    ),
    [](const ::testing::TestParamInfo<DefaultTriggerCase>& info) {return info.param.name;}
);

TEST(DefaultTrigger, WithoutLoad) {
    forge::rules::DefaultTrigger trigger;

    // nothing configured: trigger on all
    ASSERT_TRUE(trigger.trigger("ls", "Testing"));
    ASSERT_TRUE(trigger.trigger("", ""));
}

TEST(DefaultTrigger, NoCache) {
    forge::rules::DefaultTrigger trigger;
    tests::tools::Config cfg;

    // the result only depend on the given input
    ASSERT_NO_THROW(trigger.load(cfg.parse("trigger: {contains = \"error\";};", "trigger")));
    ASSERT_TRUE(trigger.trigger("make", "error"));
    ASSERT_FALSE(trigger.trigger("make", "fine"));
    ASSERT_TRUE(trigger.trigger("make", "error"));
}
