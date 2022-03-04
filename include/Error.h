#pragma once
#include <string>

#define EXTERN_BINARY_BIT   1

#define LITE_CRIT       "\033[22;31m"
#define LITE_ERR        "\033[22;35m"
#define LITE_WARN       "\033[22;33m"
#define LITE_NOTICE     "\033[22;34m"
#define LITE_INFO       "\033[22;32m"
#define LITE_DEBUG      "\033[22;36m"

extern std::string get_file_name(const char* pathname);
#ifdef GQL_LEVEL_DEBUG
#define gprint(LEVEL, fmt, ...) \
{\
printf(LEVEL "[%s:%d] [%s] " fmt "\033[22;0m\n",\
    get_file_name(__FILE__).c_str(),\
    __LINE__,\
    __FUNCTION__,\
    __VA_ARGS__);\
}
  
#else
#define gprint(fmt, ...)
#endif
