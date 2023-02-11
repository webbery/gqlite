#pragma once
#include <string>
#include <exception>
#include <cstdarg>

#define MAX_BUFFER_SIZE 256
class GCompileException : public std::exception {
public:
  GCompileException(const char* fmt, ...){
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    vsnprintf(_msg, MAX_BUFFER_SIZE, fmt, arg_ptr);
    va_end(arg_ptr);
  }

  const char* what() const { return _msg; }
private:
  char _msg[MAX_BUFFER_SIZE];
};