#pragma once
#include <cstdarg>
#include <string>
#include <exception>

#define MAX_BUFFER_SIZE 256

class GRuntimeException : public std::exception {
public:
  GRuntimeException(const char* fmt, ...){
    va_list arg_ptr;
    va_start(arg_ptr, fmt);
    vsnprintf(_msg, MAX_BUFFER_SIZE, fmt, arg_ptr);
    va_end(arg_ptr);
  }

  const char* what() const { return _msg; }
private:
  char _msg[MAX_BUFFER_SIZE];
};

#undef MAX_BUFFER_SIZE
