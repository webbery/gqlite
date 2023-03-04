#pragma once

#include <memory>
#include <functional>
#include <map>
#if defined(WIN32)
#include <windows.h>
#elif (defined(__APPLE__) && defined(__MACH__)) || defined(__ANDROID__)
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif
#include <sys/ucontext.h>
#include <ucontext.h>
#else
#include <ucontext.h>
#endif

#ifndef STACK_SIZE
#define STACK_SIZE (1024*1024)
#endif

class GCoSchedule;
class GCoroutine{
public:
#ifdef WIN32
  friend void __stdcall __win_entry(LPVOID lpParameter);
#else
  friend void __unix_entry(uint32_t l32, uint32_t h32);
  friend void __save_stack(GCoroutine* c, char* top);
#endif
  friend class GCoSchedule;

  enum class Status {
    Uninitialize,
    Ready,
    Suspend,
    Running,
    Finish,
  };

  ~GCoroutine();

  void await();
  void yield();
  void resume();

  Status status() const { return _status; }
  uint8_t id() const { return _id; }
private:
  void init(std::function<void(GCoroutine*)> const& func);

  template <typename F>
  GCoroutine(F func):_status(Status::Uninitialize),_id(0)
#ifndef  WIN32
    , _stack(nullptr), _cap(0)
#endif
  { init(func); }

private:
#if defined(WIN32)
  LPVOID _context;
#else
  ucontext_t _context;
  char* _stack;
  size_t _cap;
  size_t _size;
#endif
  std::function<void(GCoroutine*)> _func;

  GCoSchedule* _schedule;

  Status _status;

  uint8_t _id;
};

class GCoSchedule {
public:
#ifdef WIN32
  friend void __stdcall __win_entry(LPVOID lpParameter);
#else
  friend void __unix_entry(uint32_t l32, uint32_t h32);
#endif
  friend class GCoroutine;

  enum class ScheduleType {
    Main,     /*< bind to main thread */
  };

  GCoSchedule();
  ~GCoSchedule();

  template<typename F>
  GCoroutine* createCoroutine(F f) {
    GCoroutine* c = new GCoroutine(f);
    init(c);
    return c;
  }

  void init(GCoroutine* c);

  void run();

  void join();

  size_t size() const;

private:
#if defined(WIN32)
  LPVOID _main;
#else
  ucontext_t _main;
  char* _stack;
#endif
  uint8_t _current;
  std::map<uint8_t, GCoroutine*> _coroutines;
};