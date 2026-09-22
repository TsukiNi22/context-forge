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
##  @file Plugins.cpp

File Description:
##  Check the shared object (plugins) built by the `plugins` target
##  and the extern "C" interface used by the forge to load them
\**************************************************************/

#define _Exception
#define _Encapsulation
#define _Attribute
#include <utils/utils.hpp>
#include "forge/Forge.hpp"
#include "forge/rules/triggers/DefaultTrigger.hpp"
#include "forge/rules/pre-rules/AnsiPreRule.hpp"
#include "forge/rules/rules/LnRule.hpp"
#include <gtest/gtest.h>
#include <filesystem>
#include <functional>
#include <cstddef>
#include <memory>
#include <vector>
#include <string>
#include <set>

#ifndef TESTS_PLUGINS_DIR
    #error "TESTS_PLUGINS_DIR should be defined by the tests CMakeLists.txt"
#endif

/* extern "C" interface of every plugin */
using TypeFn = int (*)();
using NameFn = const char* (*)();

struct PluginCase {
    std::string name;
    std::string path; // relative to the plugins directory
    int type;
    std::string pluginName; // name used inside the rules (.cfg)
    std::function<void(utils::encapsulation::SharedObject&)> check; // type specific checks on the factory
};
std::ostream& operator<<(std::ostream& os, const PluginCase& c) {return os << c.name;}

class PluginsTest : public ::testing::TestWithParam<PluginCase> {
    public:
        static std::string path(const std::string& relative) {return (std::filesystem::path(TESTS_PLUGINS_DIR) / relative).string();};
};

TEST_P(PluginsTest, Exists) {
    const PluginCase& testCase = GetParam();

    ASSERT_TRUE(std::filesystem::is_regular_file(PluginsTest::path(testCase.path))) << "Missing plugin: " << PluginsTest::path(testCase.path) << " (build the `plugins` target)";
}

TEST_P(PluginsTest, Load) {
    const PluginCase& testCase = GetParam();
    utils::encapsulation::SharedObject plugin(PluginsTest::path(testCase.path));

    ASSERT_TRUE(plugin.isloaded());
    ASSERT_NE(plugin.get(), nullptr);
    ASSERT_EQ(plugin.path(), PluginsTest::path(testCase.path));
}

TEST_P(PluginsTest, Interface) {
    const PluginCase& testCase = GetParam();
    utils::encapsulation::SharedObject plugin(PluginsTest::path(testCase.path));

    // every symbol used by the forge should exist
    ASSERT_NO_THROW((void)plugin.loadFunction<TypeFn>("type"));
    ASSERT_NO_THROW((void)plugin.loadFunction<NameFn>("name"));
    ASSERT_NO_THROW((void)plugin.loadFunction<void (*)()>("factory"));
}

TEST_P(PluginsTest, Type) {
    const PluginCase& testCase = GetParam();
    utils::encapsulation::SharedObject plugin(PluginsTest::path(testCase.path));

    const int type = plugin.loadFunction<TypeFn>("type")();
    ASSERT_EQ(type, testCase.type);

    // should be one of the known type
    ASSERT_TRUE(type == TYPE_TRIGGER || type == TYPE_PRE_RULE || type == TYPE_RULE);
}

TEST_P(PluginsTest, Name) {
    const PluginCase& testCase = GetParam();
    utils::encapsulation::SharedObject plugin(PluginsTest::path(testCase.path));

    const char* name = plugin.loadFunction<NameFn>("name")();
    ASSERT_NE(name, nullptr);
    ASSERT_EQ(std::string(name), testCase.pluginName);

    // reserved names by the rules (.cfg) parsing
    ASSERT_NE(std::string(name), "enable");
    ASSERT_NE(std::string(name), "block");
    ASSERT_NE(std::string(name), "");
}

TEST_P(PluginsTest, Factory) {
    const PluginCase& testCase = GetParam();
    utils::encapsulation::SharedObject plugin(PluginsTest::path(testCase.path));

    testCase.check(plugin);
}

TEST_P(PluginsTest, FactoryDistinctInstances) {
    const PluginCase& testCase = GetParam();
    utils::encapsulation::SharedObject plugin(PluginsTest::path(testCase.path));

    // the factory should always create a new instance
    void* (*factory)() = plugin.loadFunction<void* (*)()>("factory");
    std::unique_ptr<forge::rules::IInstruction> first(static_cast<forge::rules::IInstruction*>(factory()));
    std::unique_ptr<forge::rules::IInstruction> second(static_cast<forge::rules::IInstruction*>(factory()));
    ASSERT_NE(first.get(), nullptr);
    ASSERT_NE(second.get(), nullptr);
    ASSERT_NE(first.get(), second.get());
}

TEST_P(PluginsTest, UnknownSymbol) {
    const PluginCase& testCase = GetParam();
    utils::encapsulation::SharedObject plugin(PluginsTest::path(testCase.path));

    try {
        (void)plugin.loadFunction<void (*)()>("unknown_symbol");
        FAIL() << "Expected loadFunction to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Dlsym);
    }
}

INSTANTIATE_TEST_SUITE_P(BuiltPlugins, PluginsTest,
    ::testing::Values(
        PluginCase{
            "TriggerDefault",
            "triggers/trigger_default.so",
            TYPE_TRIGGER,
            "trigger",
            [](utils::encapsulation::SharedObject& plugin) {
                forge::TriggerFactory factory = plugin.loadFunction<forge::TriggerFactory>("factory");
                std::unique_ptr<forge::rules::ITrigger> trigger(factory());

                ASSERT_NE(trigger.get(), nullptr);
                ASSERT_NE(dynamic_cast<forge::rules::DefaultTrigger*>(trigger.get()), nullptr);
                ASSERT_EQ(trigger->name(), "[none]");
                ASSERT_FALSE(trigger->trigger("ls", "Testing"));
                ASSERT_FALSE(trigger->trigger("", ""));
            }
        },
        PluginCase{
            "PreRuleAnsi",
            "pre-rules/pre-rule_ansi.so",
            TYPE_PRE_RULE,
            "ansi",
            [](utils::encapsulation::SharedObject& plugin) {
                forge::PreRuleFactory factory = plugin.loadFunction<forge::PreRuleFactory>("factory");
                std::unique_ptr<forge::rules::IPreRule> pre(factory());
                std::string content = "Testing\nS.O.S";

                ASSERT_NE(pre.get(), nullptr);
                ASSERT_NE(dynamic_cast<forge::rules::AnsiPreRule*>(pre.get()), nullptr);
                ASSERT_EQ(pre->name(), "[none]");
                ASSERT_NO_THROW(pre->format("ls", content));
                ASSERT_EQ(content, "Testing\nS.O.S"); // plain content untouched
            }
        },
        PluginCase{
            "RuleLn",
            "rules/rule_ln.so",
            TYPE_RULE,
            "ln",
            [](utils::encapsulation::SharedObject& plugin) {
                forge::RuleFactory factory = plugin.loadFunction<forge::RuleFactory>("factory");
                std::unique_ptr<forge::rules::IRule> rule(factory());
                std::string content = "Testing\nS.O.S";

                ASSERT_NE(rule.get(), nullptr);
                ASSERT_NE(dynamic_cast<forge::rules::LnRule*>(rule.get()), nullptr);
                ASSERT_EQ(rule->name(), "[none]");
                ASSERT_NO_THROW(rule->format("ls", content));
            }
        }
    ),
    [](const ::testing::TestParamInfo<PluginCase>& info) {return info.param.name;}
);

//----------------------------------------------------------------//
/* DIRECTORY */

TEST(Plugins, DirectoryContent) {
    std::set<std::string> found;
    std::set<std::string> names;

    // every .so of the plugins directory should be a valid plugin
    for (const auto& entry: std::filesystem::recursive_directory_iterator(TESTS_PLUGINS_DIR)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".so") continue;
        utils::encapsulation::SharedObject plugin(entry.path().string());

        ASSERT_TRUE(plugin.isloaded()) << entry.path();
        const int type = plugin.loadFunction<TypeFn>("type")();
        ASSERT_TRUE(type == TYPE_TRIGGER || type == TYPE_PRE_RULE || type == TYPE_RULE) << entry.path();

        // the names should be unique (the forge refuse duplicates)
        const std::string name = plugin.loadFunction<NameFn>("name")();
        ASSERT_TRUE(names.insert(name).second) << "Duplicated plugin name: " << name;
        found.insert(std::filesystem::relative(entry.path(), TESTS_PLUGINS_DIR).string());
    }

    ASSERT_EQ(found, (std::set<std::string>{"triggers/trigger_default.so", "pre-rules/pre-rule_ansi.so", "rules/rule_ln.so"}));
}

TEST(Plugins, UnknownSharedObject) {
    try {
        utils::encapsulation::SharedObject plugin(PluginsTest::path("unknown/unknown.so"));
        FAIL() << "Expected SharedObject to throw";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getType(), utils::exception::Type::Error);
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::Dlopen);
    }
}

//----------------------------------------------------------------//
/* FACTORY TYPE */

TEST(Plugins, FactoryVariantIndex) {
    // the variant index should match the type returned by the plugins
    static_assert(std::is_same_v<forge::FactoryType<TYPE_TRIGGER>, forge::TriggerFactory>);
    static_assert(std::is_same_v<forge::FactoryType<TYPE_PRE_RULE>, forge::PreRuleFactory>);
    static_assert(std::is_same_v<forge::FactoryType<TYPE_RULE>, forge::RuleFactory>);
    static_assert(std::variant_size_v<forge::PluginFactory> == 3);

    forge::PluginFactory factory;
    factory = static_cast<forge::TriggerFactory>(nullptr);
    ASSERT_EQ(factory.index(), TYPE_TRIGGER);
    factory = static_cast<forge::PreRuleFactory>(nullptr);
    ASSERT_EQ(factory.index(), TYPE_PRE_RULE);
    factory = static_cast<forge::RuleFactory>(nullptr);
    ASSERT_EQ(factory.index(), TYPE_RULE);
}
