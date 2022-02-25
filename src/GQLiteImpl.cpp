#include "GQliteImpl.h"
#include "ParserEngine.h"
#include "Platform.h"
#include "Statement.h"
#include "Error.h"
#include "Memory.h"
#include "StorageEngine.h"
#include "Singlecton.h"

GQLiteImpl::GQLiteImpl()
  : _pVirtualEngine(new GVirtualEngine)
{}

GQLiteImpl::~GQLiteImpl()
{
  this->close();
  delete _pVirtualEngine;
}

int GQLiteImpl::open(const char* filename, gqlite_open_mode mode)
{
  return create(filename, mode);
}

int GQLiteImpl::close()
{
  return GSinglecton::get<GStorageEngine>()->closeGraph(nullptr);
}

int GQLiteImpl::create(const char* filename, gqlite_open_mode mode)
{
  return GSinglecton::get<GStorageEngine>()->create(filename);
}

int GQLiteImpl::exec(GStatement& stm)
{
  _pVirtualEngine->test(stm);
  return ECode_Success;
}