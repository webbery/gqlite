#pragma once

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
#define __X86_ARCH__
#elif defined(__ARM_ARCH_2__) || defined(__aarch64__) || defined(_M_ARM64)
#define __ARM_ARCH__
#elif defined(mips) || defined(__mips__) || defined(__mips)
#endif

#ifdef _WIN32
#define _cpuid(info, x)    __cpuidex(info, x, 0)
#else
void _cpuid(int info[4], int InfoType);
#endif

void isSSESupport(bool& sse2, bool& avx, bool& avx2);

bool isFileExist(const char* file);

bool enableStackTrace(bool enable);
