#pragma once

#ifdef EMBEDING
typedef uint16_t EdgeID;
typedef uint32_t VertexID;
#else
#include <string>
typedef std::string EdgeID;
typedef std::string VertexID;
#endif
