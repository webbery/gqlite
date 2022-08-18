#include "base/system/Platform.h"
#include <sys/stat.h>
#include <errno.h>
#include <iostream>
#include <fmt/printf.h>
#ifdef _WIN32
#else
#include <execinfo.h>
#include <signal.h>
#endif

#ifdef __linux
void cpuid(int info[4], int InfoType)
{
  __cpuid_count(InfoType, 0, info[0], info[1], info[2], info[3]);
}
#endif

void isSSESupport(bool& sse2, bool& avx, bool& avx2)
{
  int info[4];
  cpuid(info, 0);
  int nIds = info[0];
  cpuid(info, 0x80000000);
  unsigned nExIds = info[0];
  if (nIds >= 0x00000001) {
    cpuid(info, 0x00000001);
    sse2 = (info[3] & ((int)1 << 26)) != 0;
    avx = (info[2] & ((int)1 << 28)) != 0;
    avx2 = false;
  }
  else if (nIds >= 0x00000007) {
    cpuid(info, 0x00000007);
    sse2 = true;
    avx = (info[1] & ((int)1 << 5)) != 0;
    avx2 = false;
  }
  else if (nIds >= 0x80000001) {
    cpuid(info, 0x80000001);
    sse2 = true;
    avx = false;
    avx2 = false;
  }
}

bool isFileExist(const char* file)
{
#ifdef _WIN32
  struct _stat buf;
  int result = _stat(file, &buf);
  if (result != 0) {
    if (errno == ENOENT) return false;
  }
#else
  struct stat buf;
  int result = stat(file, &buf);
  if (result != 0) return false;
#endif
  return true;
}

std::string get_file_name(const char* pathname) {
  std::string fullpath(pathname);
  size_t pos = fullpath.find_last_of('/');
  if (pos == std::string::npos) {
    pos = fullpath.find_last_of('\\');
  }
  return fullpath.substr(pos + 1, fullpath.size() - pos - 1);
}

#ifdef _WIN32
#else
void GqliteBackTrace(int signalNum) {
#define MAX_STACK_SIZE  100
  void* msg[MAX_STACK_SIZE];
  int nSize = backtrace(msg, MAX_STACK_SIZE);
  char** symbols = backtrace_symbols(msg, nSize);
  for (int i = 1; i < nSize; i++)
  {
      fmt::print("{}\n", symbols[i]);
  }
  free(symbols);
  exit(1);
}
#endif

bool enableStackTrace(bool enable) {
#ifdef _WIN32
#else
  struct sigaction newAct;
  newAct.sa_handler = GqliteBackTrace;
  sigemptyset( &newAct.sa_mask );

  sigaction(SIGABRT, &newAct, NULL);
  sigaction(SIGSEGV, &newAct, NULL);
  sigaction(SIGBUS, &newAct, NULL);
  sigaction(SIGFPE, &newAct, NULL);
  sigaction(SIGILL, &newAct, NULL);
  sigaction(SIGTRAP, &newAct, NULL);
#endif
  return true;
}
