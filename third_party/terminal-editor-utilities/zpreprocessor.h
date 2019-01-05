// Distributed under MIT License, see LICENSE file
// (c) 2018 Zbigniew Skowron, zbychs@gmail.com

#pragma once

/// ZTOKEN_STRINGIZE expands to string containing argument.
#define ZTOKEN_STRINGIZE(x) ZTOKEN_STRINGIZE2(x)
#define ZTOKEN_STRINGIZE2(x) #x

/// ZVA_OPT_SUPPORTED expands to 'true' if __VA_OPT__ is supported by the compiler. To 'false' otherwise.
#define ZPP_THIRD_ARG(a,b,c,...) c
#define ZVA_OPT_SUPPORTED_I(...) ZPP_THIRD_ARG(__VA_OPT__(,),true,false,)
#define ZVA_OPT_SUPPORTED ZVA_OPT_SUPPORTED_I(?)
