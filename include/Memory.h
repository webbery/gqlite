#pragma once
/************************************************************************/
/* gqlite memory in order to solve these problems:
/* 1. give memory interface to plugin for managing.
/* 2. support byte code address params
/* 3. minimal usage for embedding device
/************************************************************************/
#ifdef WIN32
#include "base/system/memory/memwin.h"
#elif defined(__APPLE__)
#include "base/system/memory/memmac.h"
#else
#include "base/system/memory/memlin.h"
#endif

