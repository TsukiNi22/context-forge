/**************************************************************\
Edition:
##  @date 22/09/2026 by @author Tsukini

File Name:
##  @file lib.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "forge/rules/rules/IRule.hpp"
#include "forge/rules/rules/LnRule.hpp"

// Return the type of the lib
extern "C" {
    const char* name(void)
    {
        return "ln";
    }

    forge::rules::IRule* factory(void)
    {
        return new forge::rules::LnRule;
    }
}
