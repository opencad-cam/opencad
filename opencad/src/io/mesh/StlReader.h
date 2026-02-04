#pragma once

#include <QString>
#include <TopoDS_Shape.hxx>

namespace opencad {
namespace io {

class StlReader {
public:
  /**
   * @brief Reads an STL file and returns it as a TopoDS_Shape
   * @param filename Path to the STL file
   * @return TopoDS_Shape containing the mesh as a triangulation, or null shape
   * if failed
   */
  static TopoDS_Shape readFile(const QString &filename);
};

} // namespace io
} // namespace opencad
