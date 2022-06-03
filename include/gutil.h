#pragma once
#include <vector>
#include <string>

#if defined(_MSC_VER)

#define DISABLE_WARNING(warningNumber)    __pragma(warning( disable : warningNumber ))
#define DISABLE_WARNING_PUSH __pragma(warning( push ))
#define DISABLE_WARNING_POP __pragma(warning( pop ))

#elif defined(__GNUC__) || defined(__clang__)

#define DO_PRAGMA(X) _Pragma(#X)
#define DISABLE_WARNING(warningName) \
    DO_PRAGMA(GCC diagnostic ignored #warningName)
#define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
#define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)

#else
#define DISABLE_WARNING_PUSH
#define DISABLE_WARNING_POP
#endif

std::vector<std::string> split(const char* str, const char* delim);