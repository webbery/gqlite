#include "base/Debug.h"

#if defined(__linux__)
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <execinfo.h>
#ifdef __cplusplus
#include <cxxabi.h>
#include <memory>
extern "C" {
#endif

  thread_info_t* __attribute__((no_instrument_function)) get_tls() {
    thread_local thread_info_t info;
    return &info;
  }

  int __attribute__((no_instrument_function)) get_tls_backtrace(info_t* backtrace, int maxinum) {
    int cnt = maxinum;
    thread_info_t* tls = get_tls();
    if (tls->index_ < cnt) {
      cnt = tls->index_;
    }
    if (cnt > 0) {      
      for (int i=0; i< cnt;++i) {
        backtrace[cnt - 1 - i] = tls->info_[i];
      }
    }
    return cnt;
  }
  
  void __attribute__((no_instrument_function)) __cyg_profile_func_enter(void* func, void* caller) {
    auto* tls = get_tls();
    if (tls->index_ < MAX_TRACE_SIZE) {
      tls->info_[tls->index_++] = func;
    }
  }

  void __attribute__((no_instrument_function)) __cyg_profile_func_exit(void* func, void* caller) {
    thread_info_t* tls = get_tls();
    //get_tls_dlinfo(func);
    
    //get_tls_dlinfo(tls->info_[tls->index_ - 1]);
    if (--tls->index_ < 0) {
      tls->index_ = 0;
    }
  }
  
  void __attribute__((no_instrument_function)) print_tls_backtrace(void* addr) {
    #if defined(__ANDROID_API__)
        Dl_info info;
    if (dladdr(addr, &info)) {
    unsigned long relative_addr = (char*)addr - (char*)info.dli_fbase;
    #ifdef __cplusplus
    int status;
    char* realname = info.dli_sname?abi::__cxa_demangle(info.dli_sname, 0, 0, &status): nullptr;
    
    printf("%p 0x%lx %s\n", addr, relative_addr, realname?realname:"?");
    free(realname);
    #else
    printf("%p 0x%lx %s\n", addr,relative_addr, info.dli_sname?info.dli_sname:"?");
    #endif
    }
    #else
    char** symb = backtrace_symbols(&addr, 1);
    for (int i = 0; i< 1; ++i) {
      #ifdef __cplusplus
    int status;
    char* realname = symb[i]?abi::__cxa_demangle(symb[i], 0, 0, &status): nullptr;
    printf("%p %s\n", addr, realname?realname:"?");
    free(realname);
    #else
    printf("%p %s\n", addr, info.dli_sname?info.dli_sname:"?");
    #endif
    }
    free(symb);
    #endif // ANDROID_API
  }
  
  void __attribute__((no_instrument_function)) backtrace_output(int signal) {
  }
  
  void __attribute__((no_instrument_function)) init_coredump_capture() {
    struct sigaction action;
    action.sa_handler = backtrace_output;
    sigemptyset(&action.sa_mask);
    
    sigaction(SIGABRT, &action, NULL);
    sigaction(SIGSEGV, &action, NULL);
    sigaction(SIGBUS, &action, NULL);
    sigaction(SIGFPE, &action, NULL);
    sigaction(SIGILL, &action, NULL);
    sigaction(SIGTRAP, &action, NULL);
  }

#ifdef __cplusplus
}
#endif //__cplusplus
#endif // defined(__linux__)
