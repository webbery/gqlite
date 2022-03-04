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
  , _statement(nullptr)
{}

GQLiteImpl::~GQLiteImpl()
{
  this->close();
  if (_statement) {
    delete _statement;
  }
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

void GQLiteImpl::set(GStatement* pStatement)
{
  if (_statement) {
    delete _statement;
  }
  _statement = pStatement;
}

int GQLiteImpl::create(const char* filename, gqlite_open_mode mode)
{
  return GSinglecton::get<GStorageEngine>()->create(filename);
}

void GQLiteImpl::exec(GStatement& stm)
{
  _pVirtualEngine->exec_once(stm);
  set(&stm);
}