#pragma once

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
#define __X86_ARCH__
#elif defined(__ARM_ARCH_2__) || defined(__aarch64__) || defined(_M_ARM64)
#define __ARM_ARCH__
#elif defined(mips) || defined(__mips__) || defined(__mips)
#endif

#ifdef _WIN32
#include <intrin.h>
#define _cpuid(info, x)    __cpuidex(info, x, 0)
#else
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
#include <x86intrin.h>
#elif defined(__ARM_ARCH__)
#include <arm_neon.h>
#endif
void _cpuid(int info[4], int InfoType);
#endif

void isSSESupport(bool& sse2, bool& avx, bool& avx2);

bool enableStackTrace(bool enable);
