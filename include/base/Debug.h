#pragma once

#if defined(__linux__)
#include <dlfcn.h>
#define MAX_TRACE_SIZE  100
#ifdef __cplusplus
extern "C" {
#endif
  typedef void* info_t;
  struct thread_info_t {
    info_t info_[MAX_TRACE_SIZE];
    int index_;
  };

  void __attribute__((no_instrument_function)) __cyg_profile_func_enter(void* func, void* caller);
  void __attribute__((no_instrument_function)) __cyg_profile_func_exit(void* func, void* caller);
  int __attribute__((no_instrument_function)) get_tls_backtrace(info_t* backtrace, int maxinum);
  void __attribute__((no_instrument_function)) print_tls_backtrace(void* addr);
  void __attribute__((no_instrument_function)) init_coredump_capture();
#ifdef __cplusplus
}
#endif //__cplusplus
#endif // defined(__linux__)
