#pragma once
/************************************************************************/
/* gqlite memory in order to solve these problems:
/* 1. give memory interface to plugin for managing.
/* 2. support byte code address params
/* 3. minimal usage for embedding device
/************************************************************************/
#ifdef WIN32
#include "memory/memwin.h"
#elif defined(__APPLE__)
#include "memory/memmac.h"
#else
#include "memory/memlin.h"
#endif

