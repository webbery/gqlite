#pragma once

#ifdef _WIN32
#define _cpuid(info, x)    __cpuidex(info, x, 0)
#else
void _cpuid(int info[4], int InfoType);
#endif

void isSSESupport(bool& sse2, bool& avx, bool& avx2);

bool isFileExist(const char* file);

bool enableStackTrace(bool enable);
