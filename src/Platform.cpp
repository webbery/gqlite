#include "Platform.h"
#include <sys/stat.h>
#include <errno.h>
#include <iostream>
#ifdef _WIN32
#else
#endif

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