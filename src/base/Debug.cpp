#include "base/Debug.h"

#if defined(__linux__)
#define MAX_TRACE_SIZE  100
 
#ifdef __cplusplus
extern "C" {
#endif
  thread_stack_t& get_tls() {
    thread_local thread_stack_t thread_info;
    return thread_info;
  }
  void __attribute__((no_instrument_function)) __cyg_profile_func_enter(void* func, void* caller) {
    thread_stack_t& info = get_tls();
  }

  void __attribute__((no_instrument_function)) __cyg_profile_func_exit(void* func, void* caller) {

  }

#ifdef __cplusplus
}
#endif //__cplusplus
#endif // defined(__linux__)