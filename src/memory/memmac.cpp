#include <memory>
#include <mutex>
#include <cassert>
#ifdef __APPLE__
#include "memory/memmac.h"
#include <sys/malloc.h>
#define MAX_MEMORY_SIZE   16*1024
#define MAX_STACK_SIZE    128

#ifdef _DEBUG
void* GDebugMemory::allocate(int size)
{
  return malloc(size);
}

void GDebugMemory::deallocate(void* ptr)
{
  free(ptr);
}
#endif

void* GMemory::allocate(int size)
{
  return malloc(size);
}

void GMemory::deallocate(void* ptr)
{
  free(ptr);
}

#endif // apple
