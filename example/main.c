#include "gqlite.h"
#include <assert.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif
#ifdef __APPLE__
#include <stdio.h>
#include <mach-o/dyld.h>
#endif

int main() {
  char path[256] = {0};
#ifndef __APPLE__
  getcwd(path, 256);
#else
  unsigned size = 256;
  _NSGetExecutablePath(path, &size);
  int idx = strlen(path);
  while (path[--idx] != '/');
  path[idx] = 0;
#endif
  strcat(path, "/example.db");
  gqlite* handle;
  assert(ECode_Success == gqlite_open(&handle, path));
  assert(ECode_Success == gqlite_close(handle));
  return 0;
}