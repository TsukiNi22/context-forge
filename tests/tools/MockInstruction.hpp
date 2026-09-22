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
##  @file MockInstruction.hpp

File Description:
##  Tools used to mock the different instruction (trigger, pre-rule, rule)
##  and to journal the call made on them
\**************************************************************/

#ifndef MOCKINSTRUCTION_H
    #define MOCKINSTRUCTION_H

    //----------------------------------------------------------------//
    /* INCLUDE */

    /* type */
    #include "forge/rules/triggers/ITrigger.hpp"    // forge::rules::ITrigger
    #include "forge/rules/pre-rules/IPreRule.hpp"   // forge::rules::IPreRule
    #include "forge/rules/rules/IRule.hpp"          // forge::rules::IRule
    #include <libconfig.h++>                        // libconfig::Setting
    #include <functional>                           // std::function
    #include <vector>                               // std::vector
    #include <string>                               // std::string

namespace tests::tools { // namespace start
//----------------------------------------------------------------//
/* TYPEDEF */

using Journal = std::vector<std::string>; // ordered list of every call made on the mocks
using Formater = std::function<void(std::string&)>; // edition applied on the content by a mock

//----------------------------------------------------------------//
/* CLASS */

class MockTrigger: public forge::rules::ITrigger {
    private:
        tests::tools::Journal* _journal = nullptr;
        bool _result;
        bool _loaded = false;

    public:
        // ---------- Pre-Function -------- //
        void load(const libconfig::Setting& s) final {(void)s; this->_loaded = true;};
        bool trigger(const std::string& bin, const std::string& content) final {
            if (this->_journal) this->_journal->push_back("trigger:" + bin + ":" + content);
            return this->_result;
        };

        // ------------ Function ---------- //
        inline bool loaded(void) const {return this->_loaded;};

        // ---------- Constructor --------- //
        MockTrigger(bool result, tests::tools::Journal* journal = nullptr): _journal{journal}, _result{result} {};
};

class MockPreRule: public forge::rules::IPreRule {
    private:
        tests::tools::Journal* _journal = nullptr;
        tests::tools::Formater _formater;
        bool _loaded = false;

    public:
        // ---------- Pre-Function -------- //
        void load(const libconfig::Setting& s) final {(void)s; this->_loaded = true;};
        void format(const std::string& bin, std::string& content) final {
            if (this->_journal) this->_journal->push_back("pre-rule:" + bin + ":" + content);
            if (this->_formater) this->_formater(content);
        };

        // ------------ Function ---------- //
        inline bool loaded(void) const {return this->_loaded;};

        // ---------- Constructor --------- //
        MockPreRule(tests::tools::Formater formater = nullptr, tests::tools::Journal* journal = nullptr): _journal{journal}, _formater{formater} {};
};

class MockRule: public forge::rules::IRule {
    private:
        tests::tools::Journal* _journal = nullptr;
        tests::tools::Formater _formater;
        bool _loaded = false;

    public:
        // ---------- Pre-Function -------- //
        void load(const libconfig::Setting& s) final {(void)s; this->_loaded = true;};
        void format(const std::string& bin, std::string& content) final {
            if (this->_journal) this->_journal->push_back("rule:" + bin + ":" + content);
            if (this->_formater) this->_formater(content);
        };

        // ------------ Function ---------- //
        inline bool loaded(void) const {return this->_loaded;};

        // ---------- Constructor --------- //
        MockRule(tests::tools::Formater formater = nullptr, tests::tools::Journal* journal = nullptr): _journal{journal}, _formater{formater} {};
};

} // namespace end
#endif /* MOCKINSTRUCTION_H */
