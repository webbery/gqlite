#include "ScanObserver.h"

GScanObserver::GScanObserver(GLoader* loader)
  :_loader(loader)
{

}

GScanObserver::~GScanObserver()
{

}

void GScanObserver::update(KeyType type, const std::string& key, const nlohmann::json& value) {

}