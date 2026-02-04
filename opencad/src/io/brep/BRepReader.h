#pragma once

#include "core/geometry/Shape.h"
#include <string>


namespace opencad {
namespace io {

class BRepReader {
public:
  /**
   * @brief Reads a native OpenCASCADE BRep file
   * @param filename Path to the file
   * @return TopoDS_Shape (check IsNull() for failure)
   */
  static core::Shape readFile(const std::string &filename);
};

} // namespace io
} // namespace opencad
