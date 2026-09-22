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
##  @file Rules.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Verbose
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/Rules.hpp"
#include "tools/MockInstruction.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <optional>
#include <memory>
#include <vector>
#include <string>

//----------------------------------------------------------------//
/* PATH */

TEST(Rules, DefaultPath) {
    forge::rules::Rules rules;

    ASSERT_EQ(rules.path(), "[none]");
}

TEST(Rules, Path) {
    forge::rules::Rules rules;

    rules.path("rules/test.cfg");
    ASSERT_EQ(rules.path(), "rules/test.cfg");
}

TEST(Rules, MoveKeepContent) {
    forge::rules::Rules rules;
    rules.path("rules/test.cfg");
    rules.push(std::make_unique<tests::tools::MockTrigger>(false));
    rules.push(std::make_unique<tests::tools::MockRule>([](std::string& content) {content += "!";}));

    // Move constructor
    forge::rules::Rules moved(std::move(rules));
    std::string content = "Testing";
    ASSERT_EQ(moved.path(), "rules/test.cfg");
    ASSERT_FALSE(moved.trigger("ls", content));
    moved.apply("ls", content);
    ASSERT_EQ(content, "Testing!");

    // Move assignment
    forge::rules::Rules assigned;
    assigned = std::move(moved);
    content = "Testing";
    ASSERT_EQ(assigned.path(), "rules/test.cfg");
    ASSERT_FALSE(assigned.trigger("ls", content));
    assigned.apply("ls", content);
    ASSERT_EQ(content, "Testing!");
}

//----------------------------------------------------------------//
/* TRIGGER */

struct RulesTriggerCase {
    std::string name;
    std::vector<bool> triggers; // result of each trigger pushed (in order)
    bool expected;
};
std::ostream& operator<<(std::ostream& os, const RulesTriggerCase& c) {return os << c.name;}

class RulesTriggerTest : public ::testing::TestWithParam<RulesTriggerCase> {};

TEST_P(RulesTriggerTest, ProducesExpectedResult) {
    const RulesTriggerCase& testCase = GetParam();
    forge::rules::Rules rules;

    for (const bool result: testCase.triggers)
        rules.push(std::make_unique<tests::tools::MockTrigger>(result));

    ASSERT_EQ(rules.trigger("ls", "Testing"), testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(TriggerCases, RulesTriggerTest,
    ::testing::Values(
        RulesTriggerCase{"NoTrigger", {}, true},
        RulesTriggerCase{"SingleTrue", {true}, true},
        RulesTriggerCase{"SingleFalse", {false}, false},
        RulesTriggerCase{"AllTrue", {true, true, true}, true},
        RulesTriggerCase{"AllFalse", {false, false, false}, false},
        RulesTriggerCase{"FirstTrue", {true, false, false}, true},
        RulesTriggerCase{"MiddleTrue", {false, true, false}, true},
        RulesTriggerCase{"LastTrue", {false, false, true}, true}
    ),
    [](const ::testing::TestParamInfo<RulesTriggerCase>& info) {return info.param.name;}
);

TEST(Rules, TriggerForwardArguments) {
    tests::tools::Journal journal;
    forge::rules::Rules rules;

    rules.push(std::make_unique<tests::tools::MockTrigger>(true, &journal));
    ASSERT_TRUE(rules.trigger("ls", "Testing"));
    ASSERT_EQ(journal, (tests::tools::Journal{"trigger:ls:Testing"}));
}

TEST(Rules, TriggerStopAtFirstValid) {
    tests::tools::Journal journal;
    forge::rules::Rules rules;

    // Only the two first should be called
    rules.push(std::make_unique<tests::tools::MockTrigger>(false, &journal));
    rules.push(std::make_unique<tests::tools::MockTrigger>(true, &journal));
    rules.push(std::make_unique<tests::tools::MockTrigger>(false, &journal));

    ASSERT_TRUE(rules.trigger("ls", "Testing"));
    ASSERT_EQ(journal.size(), 2);
}

TEST(Rules, TriggerCalledEveryTime) {
    tests::tools::Journal journal;
    forge::rules::Rules rules;

    // No cache, the trigger should be call for each content
    rules.push(std::make_unique<tests::tools::MockTrigger>(false, &journal));
    ASSERT_FALSE(rules.trigger("ls", "Testing"));
    ASSERT_FALSE(rules.trigger("cat", "S.O.S"));
    ASSERT_EQ(journal, (tests::tools::Journal{"trigger:ls:Testing", "trigger:cat:S.O.S"}));
}

//----------------------------------------------------------------//
/* LOAD BLOCK */

struct RulesLoadBlockCase {
    std::string name;
    std::string config; // libconfig content with a `block` group
    bool valid;
};
std::ostream& operator<<(std::ostream& os, const RulesLoadBlockCase& c) {return os << c.name;}

class RulesLoadBlockTest : public ::testing::TestWithParam<RulesLoadBlockCase> {};

TEST_P(RulesLoadBlockTest, ValidateConfig) {
    const RulesLoadBlockCase& testCase = GetParam();
    forge::rules::Rules rules;
    libconfig::Config cfg;

    ASSERT_NO_THROW(cfg.readString(testCase.config));
    const libconfig::Setting& s = cfg.getRoot()["block"];

    if (testCase.valid) {
        ASSERT_NO_THROW(rules.loadBlock(s));
        return;
    }
    try {
        rules.loadBlock(s);
        FAIL() << "Expected loadBlock to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), static_cast<utils::exception::InternalCode>(utils::exception::ExternalCode::Rules));
    }
}

INSTANTIATE_TEST_SUITE_P(ConfigCases, RulesLoadBlockTest,
    ::testing::Values(
        // valid
        RulesLoadBlockCase{"Empty", "block: {};", true},
        RulesLoadBlockCase{"Ln", "block: {ln = 2;};", true},
        RulesLoadBlockCase{"Char", "block: {char = 2;};", true},
        RulesLoadBlockCase{"Show", "block: {show = 2;};", true},
        RulesLoadBlockCase{"Sep", "block: {sep = \"--\";};", true},
        RulesLoadBlockCase{"SepEmpty", "block: {sep = \"\";};", true},
        RulesLoadBlockCase{"LnShowSep", "block: {ln = 2; show = 1; sep = \"--\";};", true},
        RulesLoadBlockCase{"CharShowSep", "block: {char = 2; show = 1; sep = \"--\";};", true},
        RulesLoadBlockCase{"UnknownKey", "block: {unknown = 2;};", true},

        // invalid
        RulesLoadBlockCase{"LnAndChar", "block: {ln = 2; char = 2;};", false},
        RulesLoadBlockCase{"LnZero", "block: {ln = 0;};", false},
        RulesLoadBlockCase{"LnNegative", "block: {ln = -2;};", false},
        RulesLoadBlockCase{"LnString", "block: {ln = \"2\";};", false},
        RulesLoadBlockCase{"LnFloat", "block: {ln = 2.5;};", false},
        RulesLoadBlockCase{"LnBool", "block: {ln = true;};", false},
        RulesLoadBlockCase{"CharZero", "block: {char = 0;};", false},
        RulesLoadBlockCase{"CharNegative", "block: {char = -2;};", false},
        RulesLoadBlockCase{"CharString", "block: {char = \"2\";};", false},
        RulesLoadBlockCase{"ShowZero", "block: {show = 0;};", false},
        RulesLoadBlockCase{"ShowNegative", "block: {show = -2;};", false},
        RulesLoadBlockCase{"ShowString", "block: {show = \"2\";};", false},
        RulesLoadBlockCase{"SepInt", "block: {sep = 2;};", false},
        RulesLoadBlockCase{"SepBool", "block: {sep = true;};", false}
    ),
    [](const ::testing::TestParamInfo<RulesLoadBlockCase>& info) {return info.param.name;}
);

//----------------------------------------------------------------//
/* APPLY */

/* every case use one pre-rule (append "+") and one rule (wrap into "[]") */
struct RulesApplyCase {
    std::string name;
    std::optional<std::string> config; // libconfig content with a `block` group (nullopt: block disable)
    std::string content;
    std::string expected;
};
std::ostream& operator<<(std::ostream& os, const RulesApplyCase& c) {return os << c.name;}

class RulesApplyTest : public ::testing::TestWithParam<RulesApplyCase> {};

TEST_P(RulesApplyTest, ProducesExpectedOutput) {
    const RulesApplyCase& testCase = GetParam();
    forge::rules::Rules rules;
    libconfig::Config cfg;

    // setup the block
    if (testCase.config.has_value()) {
        ASSERT_NO_THROW(cfg.readString(*testCase.config));
        ASSERT_NO_THROW(rules.loadBlock(cfg.getRoot()["block"]));
    }

    // setup the instructions
    rules.push(std::make_unique<tests::tools::MockPreRule>([](std::string& content) {content += "+";}));
    rules.push(std::make_unique<tests::tools::MockRule>([](std::string& content) {content = "[" + content + "]";}));

    std::string content = testCase.content;
    rules.apply("ls", content);
    ASSERT_EQ(content, testCase.expected);
}

INSTANTIATE_TEST_SUITE_P(ApplyCases, RulesApplyTest,
    ::testing::Values(
        // block disable
        RulesApplyCase{"NoBlock", std::nullopt, "abc\ndef", "[abc\ndef+]"},
        RulesApplyCase{"NoBlockEmpty", std::nullopt, "", "[+]"},
        RulesApplyCase{"NoBlockSingleNewLine", std::nullopt, "\n", "[\n+]"},

        // block enable without split
        RulesApplyCase{"BlockEmpty", "block: {};", "abc\ndef", "[abc\ndef+]"},
        RulesApplyCase{"BlockOnlyShow", "block: {show = 1;};", "abc\ndef", "[abc\ndef+]"},
        RulesApplyCase{"BlockOnlySep", "block: {sep = \"|\";};", "abc\ndef", "[abc\ndef+]"},

        // block enable: ln
        RulesApplyCase{"Ln1", "block: {ln = 1; sep = \"|\";};", "a\nb\nc", "[a+]|[b+]|[c+]"},
        RulesApplyCase{"Ln2", "block: {ln = 2; sep = \"|\";};", "a\nb\nc\nd\ne", "[a\nb+]|[c\nd+]|[e+]"},
        RulesApplyCase{"Ln2Exact", "block: {ln = 2; sep = \"|\";};", "a\nb\nc\nd\n", "[a\nb+]|[c\nd+]"},
        RulesApplyCase{"LnTrailingNewLine", "block: {ln = 1; sep = \"|\";};", "a\nb\n", "[a+]|[b+]"},
        RulesApplyCase{"LnEmptyLines", "block: {ln = 1; sep = \"|\";};", "\n\n", "[+]|[+]"},
        RulesApplyCase{"LnNoNewLine", "block: {ln = 2; sep = \"|\";};", "abc", "[abc+]"},
        RulesApplyCase{"LnOverContent", "block: {ln = 10; sep = \"|\";};", "a\nb\nc", "[a\nb\nc+]"},
        RulesApplyCase{"LnEmpty", "block: {ln = 1; sep = \"|\";};", "", ""},
        RulesApplyCase{"LnDefaultSep", "block: {ln = 1;};", "a\nb\nc", "[a+][b+][c+]"},
        RulesApplyCase{"LnMultiCharSep", "block: {ln = 1; sep = \" -- \";};", "a\nb", "[a+] -- [b+]"},

        // block enable: char
        RulesApplyCase{"Char1", "block: {char = 1; sep = \"|\";};", "abc", "[a+]|[b+]|[c+]"},
        RulesApplyCase{"Char3", "block: {char = 3; sep = \"|\";};", "abcdefgh", "[abc+]|[def+]|[gh+]"},
        RulesApplyCase{"Char3Exact", "block: {char = 3; sep = \"|\";};", "abcdef", "[abc+]|[def+]"},
        RulesApplyCase{"CharWithNewLine", "block: {char = 2; sep = \"|\";};", "a\nb\n", "[a\n+]|[b\n+]"},
        RulesApplyCase{"CharOverContent", "block: {char = 10; sep = \"|\";};", "abc", "[abc+]"},
        RulesApplyCase{"CharEmpty", "block: {char = 1; sep = \"|\";};", "", ""},
        RulesApplyCase{"CharDefaultSep", "block: {char = 1;};", "abc", "[a+][b+][c+]"},

        // block enable: show
        RulesApplyCase{"LnShow", "block: {ln = 1; show = 2; sep = \"|\";};", "a\nb\nc\nd", "[a+]|[b+]"},
        RulesApplyCase{"LnShowOne", "block: {ln = 1; show = 1; sep = \"|\";};", "a\nb\nc\nd", "[a+]"},
        RulesApplyCase{"LnShowOverBlocks", "block: {ln = 1; show = 10; sep = \"|\";};", "a\nb", "[a+]|[b+]"},
        RulesApplyCase{"CharShow", "block: {char = 1; show = 2; sep = \"|\";};", "abcd", "[a+]|[b+]"},
        RulesApplyCase{"CharShowOverBlocks", "block: {char = 1; show = 10; sep = \"|\";};", "ab", "[a+]|[b+]"}
    ),
    [](const ::testing::TestParamInfo<RulesApplyCase>& info) {return info.param.name;}
);

TEST(Rules, ApplyWithoutInstruction) {
    forge::rules::Rules rules;
    std::string content = "abc\ndef";

    // nothing to apply, the content should be untouched
    rules.apply("ls", content);
    ASSERT_EQ(content, "abc\ndef");
}

TEST(Rules, ApplyBlockWithoutInstruction) {
    forge::rules::Rules rules;
    libconfig::Config cfg;
    std::string content = "a\nb\nc";

    // only the block split/join should be applied
    ASSERT_NO_THROW(cfg.readString("block: {ln = 1; sep = \"|\";};"));
    ASSERT_NO_THROW(rules.loadBlock(cfg.getRoot()["block"]));
    rules.apply("ls", content);
    ASSERT_EQ(content, "a|b|c");
}

TEST(Rules, ApplyForwardArguments) {
    tests::tools::Journal journal;
    forge::rules::Rules rules;
    std::string content = "Testing";

    rules.push(std::make_unique<tests::tools::MockPreRule>(nullptr, &journal));
    rules.push(std::make_unique<tests::tools::MockRule>(nullptr, &journal));
    rules.apply("ls", content);

    ASSERT_EQ(content, "Testing");
    ASSERT_EQ(journal, (tests::tools::Journal{"pre-rule:ls:Testing", "rule:ls:Testing"}));
}

TEST(Rules, ApplyOrder) {
    tests::tools::Journal journal;
    forge::rules::Rules rules;
    libconfig::Config cfg;
    std::string content = "x\ny";

    // split each line
    ASSERT_NO_THROW(cfg.readString("block: {ln = 1; sep = \"|\";};"));
    ASSERT_NO_THROW(rules.loadBlock(cfg.getRoot()["block"]));

    // push in a mixed order, the pre-rules should always be applied before the rules
    rules.push(std::make_unique<tests::tools::MockRule>([](std::string& s) {s += "c";}, &journal));
    rules.push(std::make_unique<tests::tools::MockPreRule>([](std::string& s) {s += "a";}, &journal));
    rules.push(std::make_unique<tests::tools::MockRule>([](std::string& s) {s += "d";}, &journal));
    rules.push(std::make_unique<tests::tools::MockPreRule>([](std::string& s) {s += "b";}, &journal));

    rules.apply("ls", content);
    ASSERT_EQ(content, "xabcd|yabcd");

    /* Order:
     * block x: pre-rule (a), pre-rule (b), rule (c), rule (d)
     * block y: pre-rule (a), pre-rule (b), rule (c), rule (d)
    */
    ASSERT_EQ(journal, (tests::tools::Journal{
        "pre-rule:ls:x", "pre-rule:ls:xa", "rule:ls:xab", "rule:ls:xabc",
        "pre-rule:ls:y", "pre-rule:ls:ya", "rule:ls:yab", "rule:ls:yabc"
    }));
}

TEST(Rules, ApplyTriggerIndependent) {
    forge::rules::Rules rules;
    std::string content = "Testing";

    // apply doesn't check the triggers, this is the caller responsibility
    rules.push(std::make_unique<tests::tools::MockTrigger>(false));
    rules.push(std::make_unique<tests::tools::MockRule>([](std::string& s) {s += "!";}));

    ASSERT_FALSE(rules.trigger("ls", content));
    rules.apply("ls", content);
    ASSERT_EQ(content, "Testing!");
}
