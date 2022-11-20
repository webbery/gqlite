#include "ngql.h"

std::regex g_regElements(R"((\w+))");

namespace {
  std::vector<std::string> extractor(const std::string& input, const std::regex& reg) {
    std::vector<std::string> vec;
    std::sregex_iterator iter(input.begin(), input.end(), reg);
    std::sregex_iterator end;
    while (iter != end)
    {
      for (int index = 1; index < iter->size(); ++index) {
        vec.push_back((*iter)[index]);
      }
      ++iter;
    }
    return vec;
  }
}

IConverter::Result NGQLConverter::Parse(const std::string& line, std::string& out) {
  std::cmatch m;

  std::regex regDrop(R"(drop space (\w+);)");
  if (std::regex_match(line.c_str(), m, regDrop)) {
    Drop(m, out);
    return IConverter::Result::CS;
  }

  std::regex regCreateGraph(R"(create space (\w+)\([\w\W]+\);)");
  if (std::regex_match(line.c_str(), m, regCreateGraph)) {
    _creation = m[1];
    return IConverter::Result::RN;
  }

  std::regex regCreateTag(R"(create tag (\w+)\((\w+) \w+(,[ ]*(\w+) \w+)*\);)");
  if (std::regex_match(line.c_str(), m, regCreateTag)) {
    Group g;
    g._name = m[1];
    for (int index = 2; index < m.size(); index+= 2) {
      std::string prop(m[index]);
      if (prop.empty()) continue;

      g._props.push_back(prop);
    }
    _groups.push_back(g);
    return IConverter::Result::RN;
  }

  std::regex regCreateEdge(R"(create edge (\w+)\((\w+) \w+(,[ ]*(\w+) \w+)*\);)");
  if (std::regex_match(line.c_str(), m, regCreateEdge)) {
    Group g;
    g._name = m[1];
    for (int index = 2; index < m.size(); index+= 2) {
      std::string prop(m[index]);
      if (prop.empty()) continue;

      g._props.push_back(prop);
    }
    _groups.push_back(g);
    return IConverter::Result::RN;
  }

  std::regex regCreateIndex(R"(create tag index \w+ on (\w+)\(([\w+\(\d+\)]*)\);)");
  if (std::regex_match(line.c_str(), m, regCreateIndex)) {
    std::string group = m[1];
    std::vector<std::string> indexes;
    std::regex regIndex(R"((\w+)\(\d+\))");
    for (int index = 2; index < m.size(); ++index) {
      std::string sIndex(m[index]);
      std::cmatch mIndx;
      if (std::regex_match(sIndex.c_str(), mIndx, regIndex)) {
        std::string name = mIndx[1];
        indexes.push_back(name);
      }
    }

    for (auto& g: _groups) {
      if (g._name == group) {
        g._indexes = indexes;
      }
    }
    return IConverter::Result::RN;
  }

  if (line.find(":sleep") != std::string::npos || line.find("use") != std::string::npos) {
    return IConverter::Result::CS;
  }

  if (!_creation.empty()) {
    Create(out);
    _creation.clear();
    _groups.clear();
  }

  std::regex regUpsetVertex(R"(insert vertex (\w+)\(([\w\W]+)\) values \"(\w+)\":\(([\w\W]+)\);)");
  if (std::regex_match(line.c_str(), m, regUpsetVertex)) {
    UpsetVertex(m, out);
    return IConverter::Result::CS;
  }

  std::regex regUpsetEdge(R"(insert edge (\w+)\(([\w, ]+)\) values \"(\w+)\"([-><]+)\"(\w+)\"[@\w]*:\(([\w, ]+)\);)");
  if (std::regex_match(line.c_str(), m, regUpsetEdge)) {
    UpsetEdge(m, out);
    return IConverter::Result::CS;
  }

  return IConverter::Result::CS;
}

bool NGQLConverter::Drop(std::cmatch& match, std::string& out) {
  out += std::string("{drop: '") + std::string(match[1]) + "'};";
  return true;
}
bool NGQLConverter::Create(std::string& out) {
  out += "{create: '" + _creation + "', group: [";
  for (auto g: _groups) {
    if (g._props.size()) {
      out += "{" + g._name + ": [";
      for (auto prop: g._props) {
        out += "'" + prop + "',";
      }
      out.pop_back();

      // indexes
      if (g._indexes.size()) {
        out += "], index: [";
        for (auto index: g._indexes) {
          out += "'" + index + "',";
        }
        out.pop_back();
      }
      
      out += "]},";
    } else {

    }
  }
  out.pop_back();
  out += "]};";
  return true;
}
bool NGQLConverter::Skip(const std::string& line, std::string& out) {
  return true;
}

bool NGQLConverter::UpsetVertex(std::cmatch& match, std::string& out) {
  std::string sProps(match[2]);
  std::vector<std::string> props = split(sProps.c_str(), ",");

  out += "{upset: '" + std::string(match[1]) + "', vertex: [";
  //[42, {class: 'a/c', location: [131.24194, 37.12532], keyword: ['a', 'b'], create_time: 1}]
  out += "['" + std::string(match[3]) + "', {";

  std::string value(match[4]);
  std::vector<std::string> values = split(value.c_str(), ",");

  for (int index = 0; index < props.size(); ++index) {
    std::string v(values[index]);
    v.erase(0, v.find_first_not_of(' '));
    if (v.size() && v[0] == '"') {
      v.erase(v.begin());
      v.erase(v.end() - 1);

      std::regex patternQuote("'");
      v = std::regex_replace(v, patternQuote, "\\'");

      v = ("'" + v + "'");
    }
    out += props[index] + ": " + v + ", ";
  }
  out.pop_back();
  out.pop_back();
  
  out += "}]]};";
  return true;
}

bool NGQLConverter::UpsetEdge(std::cmatch& match, std::string& out) {
  out += "{upset: '" + std::string(match[1]) + "', edge: [";
  out += "['" + std::string(match[3]) + "', " + std::string(match[4]);

  std::string props(match[2]), values(match[6]);

  std::vector<std::string> vProps = extractor(props, g_regElements);
  std::vector<std::string> vValues = extractor(values, g_regElements);
  
  if (vProps.size()) {
    out += ": {";
    for (int index = 0; index < vProps.size(); ++index) {
      out += vProps[index] + ": " + vValues[index] + ", ";
    }
    out.erase(out.end() - 2);
    out += "}";
  }
  
  out += ", '" + std::string(match[5]) + "']";
  out += "]};";
  return true;
}