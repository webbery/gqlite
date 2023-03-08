#pragma once
#include <cstddef>

#ifdef __ANDROID__
struct gcontext_t
{
  unsigned long int uc_flags;
  struct gcontext_t *uc_link;
  stack_t uc_stack;
  void* __args;
  // mcontext_t uc_mcontext;
  // sigset_t uc_sigmask;
  // struct _libc_fpstate __fpregs_mem;
};

typedef void*   fcontext_t;

struct transfer_t {
    fcontext_t  fctx;
    void    *   data;
};
#elif WIN32
#else
#include <ucontext.h>
typedef ucontext_t gcontext_t;
#endif