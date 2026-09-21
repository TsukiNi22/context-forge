/**************************************************************\
Edition:
##  @date 21/09/2026 by @author Tsukini

File Name:
##  @file lib.cpp

File Description:
##  You know, I don t think there are good or bad descriptions,
##  for me, life is all about functions...
\**************************************************************/

#include "forge/Forge.hpp"

// Return the type of the lib
extern "C" {
    int type(void)
    {
        return TYPE_RULE;
    }
}
