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
##  @file Forge-Formating.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#define _Attribute
#define _Exception
#define _Verbose
#define _Encapsulation
#include <utils/utils.hpp>
#include "forge/Forge.hpp"
#include <string>

void forge::Forge::formatCFG(const std::string& bin, std::string& input) const
{
    // apply each rules
    onDebugVerbose("----- [RULES] -----");
    for (const forge::rules::Rules& rules: this->_rules) {
        if (rules.trigger(bin, input)) {
            onAdvancedVerbose("apply: " << rules.path());
            rules.apply(bin, input);
        }
    }
}

void forge::Forge::formatLLM(const std::string& bin, std::string& input) const
{
    if (!this->_llm) return;
    onDebugVerbose("----- [LLM] -----");

    // setup prompt
    std::string prompt;
    prompt += "<binary>" + bin + "</binary>\n";
    prompt += "<output>\n" + input + "\n</output>\n";

    // call llm
    input = this->_ollama.prompt(prompt);
}
