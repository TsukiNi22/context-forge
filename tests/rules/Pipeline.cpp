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
##  @file Pipeline.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Exception
#define _Attribute
#include <utils/utils.hpp>
#include "forge/rules/Rules.hpp"
#include "forge/rules/triggers/DefaultTrigger.hpp"
#include "forge/rules/pre-rules/AnsiPreRule.hpp"
#include "forge/rules/rules/LnRule.hpp"
#include "forge/rules/rules/DropRule.hpp"
#include "forge/rules/rules/DupRule.hpp"
#include "forge/rules/rules/InsertRule.hpp"
#include "forge/rules/rules/ReplaceRule.hpp"
#include <gtest/gtest.h>
#include <libconfig.h++>
#include <memory>
#include <string>

/* same dispatch as the forge (loadCFG) but using the plugins class directly instead of the shared objects */
template<typename T>
static void push(forge::rules::Rules& rules, const libconfig::Setting& s)
{
    std::unique_ptr<T> ptr = std::make_unique<T>();
    ptr->load(s);
    rules.push(std::move(ptr));
}

static void build(forge::rules::Rules& rules, const std::string& config)
{
    libconfig::Config cfg;
    cfg.readString(config);

    for (const libconfig::Setting& s: cfg.getRoot()) {
        const std::string name = s.getName();
        if (name == "block") rules.loadBlock(s);
        else if (name == "trigger") push<forge::rules::DefaultTrigger>(rules, s);
        else if (name == "ansi") push<forge::rules::AnsiPreRule>(rules, s);
        else if (name == "ln") push<forge::rules::LnRule>(rules, s);
        else if (name == "drop") push<forge::rules::DropRule>(rules, s);
        else if (name == "dup") push<forge::rules::DupRule>(rules, s);
        else if (name == "insert") push<forge::rules::InsertRule>(rules, s);
        else if (name == "replace") push<forge::rules::ReplaceRule>(rules, s);
        else FAIL() << "Unknown plugin used: " << name;
    }
}

//----------------------------------------------------------------//
/* PIPELINE */

struct PipelineCase {
    std::string name;
    std::string config; // content of a rules file (.cfg)
    std::string bin;
    std::string content;
    bool trigger;
    std::string expected; // only checked when triggered
};
std::ostream& operator<<(std::ostream& os, const PipelineCase& c) {return os << c.name;}

class PipelineTest : public ::testing::TestWithParam<PipelineCase> {};

TEST_P(PipelineTest, ProducesExpectedOutput) {
    const PipelineCase& testCase = GetParam();
    forge::rules::Rules rules;
    std::string content = testCase.content;

    ASSERT_NO_FATAL_FAILURE(build(rules, testCase.config));
    ASSERT_EQ(rules.trigger(testCase.bin, content), testCase.trigger);
    if (!testCase.trigger) return;

    rules.apply(testCase.bin, content);
    ASSERT_EQ(content, testCase.expected);
}

/* based on .info/rules-t2.cfg */
#define FULL_CHAIN \
    "trigger = {bin = [\"grep\", \"rg\"]; contains = \"match\";};\n" \
    "ansi = false;\n" \
    "drop = [\"^\\\\s*$\"];\n" \
    "dup = {match = [\"^Binary file .* matches$\"]; keep = 1; invert = false;};\n" \
    "replace = {match = [\"\\\\bTODO\\\\b\"]; by = \"[TODO -> <INSERT>]\";};\n" \
    "ln = {tail = 50;};\n" \
    "insert = {before = \"----- grep output -----\\n\"; after = \"\\n----- end -----\\n\";};\n"

#define GREP_OUTPUT \
    "\x1b[35msrc/a.cpp\x1b[0m: // TODO match\n" \
    "\n" \
    "Binary file x matches\n" \
    "Binary file y matches\n" \
    "  \n" \
    "src/b.cpp: match TODO2\n"

INSTANTIATE_TEST_SUITE_P(RulesFiles, PipelineTest,
    ::testing::Values(
        // based on .info/rules-t1.cfg
        PipelineCase{
            "LsTotal",
            "trigger = {bin = [\"ls\"];};\ndrop = [\"^total \\\\d+\"];\n",
            "ls",
            "total 12\nMakefile\nREADME.md",
            true,
            "Makefile\nREADME.md"
        },
        PipelineCase{
            "LsTotalOtherBin",
            "trigger = {bin = [\"ls\"];};\ndrop = [\"^total \\\\d+\"];\n",
            "cat",
            "total 12\nMakefile\nREADME.md",
            false,
            ""
        },

        // based on .info/rules-t2.cfg
        PipelineCase{
            "FullChain",
            FULL_CHAIN,
            "grep",
            GREP_OUTPUT,
            true,
            "----- grep output -----\n"
            "src/a.cpp: // [TODO -> TODO] match\n"
            "Binary file x matches\n"
            "src/b.cpp: match TODO2\n"
            "----- end -----\n"
        },
        PipelineCase{
            "FullChainContains",
            FULL_CHAIN,
            "cat",
            "no color here, just a match",
            true,
            "----- grep output -----\n"
            "no color here, just a match\n"
            "----- end -----\n"
        },
        PipelineCase{
            "FullChainNoTrigger",
            FULL_CHAIN,
            "cat",
            "nothing to see",
            false,
            ""
        },

        // block + rules applied on each block
        PipelineCase{
            "BlockLn",
            "block = {ln = 1; sep = \"\\n\";};\ninsert = {before = \"- \";};\n",
            "ls",
            "a\nb\nc",
            true,
            "- a\n- b\n- c"
        },
        PipelineCase{
            "BlockShow",
            "block = {ln = 2; show = 1; sep = \"|\";};\nln = {head = 1;};\n",
            "ls",
            "a\nb\nc\nd",
            true,
            "a"
        },
        PipelineCase{
            "BlockChar",
            "block = {char = 3; sep = \"|\";};\nreplace = {match = [\"^.\"]; by = \"_\";};\n",
            "ls",
            "abcdefgh",
            true,
            "_bc|_ef|_h"
        },

        // no trigger: apply on all
        PipelineCase{
            "NoTrigger",
            "ansi = false;\ndup = {eq = [\"x\"];};\n",
            "echo",
            "\x1b[1mx\x1b[0m\nx\ny",
            true,
            "x\ny"
        },
        PipelineCase{
            "Empty",
            "",
            "echo",
            "Testing\n",
            true,
            "Testing\n"
        }
    ),
    [](const ::testing::TestParamInfo<PipelineCase>& info) {return info.param.name;}
);

TEST(Pipeline, InvalidRuleStopBuild) {
    forge::rules::Rules rules;

    // the first invalid plugin config should stop the whole rules file
    try {
        build(rules, "ln = {head = \"2\";};\n");
        FAIL() << "Expected build to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), static_cast<utils::exception::InternalCode>(utils::exception::ExternalCode::Rules));
    }
}
