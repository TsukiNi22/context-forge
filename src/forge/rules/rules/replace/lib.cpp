/**************************************************************\
Edition:
##  @date 23/09/2026 by @author Tsukini

File Name:
##  @file lib.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "forge/rules/rules/IRule.hpp"
#include "forge/rules/rules/ReplaceRule.hpp"

// Return the type of the lib
extern "C" {
    const char* name(void)
    {
        return "replace";
    }

    forge::rules::IRule* factory(void)
    {
        return new forge::rules::ReplaceRule;
    }
}
