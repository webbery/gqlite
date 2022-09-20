#pragma once

#if defined(__linux__)
#define MAX_TRACE_SIZE  100
#ifdef __cplusplus
extern "C" {
#endif
  struct thread_stack_t {
    void* info_[MAX_TRACE_SIZE];
    char index_;
  };
  void __attribute__((no_instrument_function)) __cyg_profile_func_enter(void* func, void* caller);
  void __attribute__((no_instrument_function)) __cyg_profile_func_exit(void* func, void* caller);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif // defined(__linux__)