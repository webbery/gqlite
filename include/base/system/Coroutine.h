#pragma once

#include <memory>
#include <functional>
#include <map>

class GWorker {
public:
  enum class Status {
    Uninitialize,
    Ready,
    Suspend,
    Running,
    Finish,
  };

  GWorker(): _status(Status::Uninitialize) {}
  virtual ~GWorker() {}

  virtual void yield() = 0;
  virtual void resume() = 0;
  virtual Status status() const { return _status; }

protected:
  Status _status;

};

class GDefaultSchedule;
typedef void*   fcontext_t;
struct transfer_t {
    fcontext_t  fctx;
    void    *   data;
};


class GCoroutine : public GWorker{
public:
  friend void __coroutine_entry(transfer_t t);
  friend class GSchedule;
  friend class GDefaultSchedule;

  ~GCoroutine();

  void yield();
  void resume();

  Status status() const { return _status; }
  uint8_t id() const { return _id; }
private:
  void init(std::function<void(GCoroutine*)> const& func);

  template <typename F>
  GCoroutine(F func):_id(0)
  { init(func); }

private:
  std::function<void(GCoroutine*)> _func;

  fcontext_t _ctx{nullptr};

  void* _stack{nullptr};

  GDefaultSchedule* _schedule;


  uint8_t _id;
};
