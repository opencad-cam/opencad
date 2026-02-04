#include "io/brep/BRepReader.h"
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>

namespace opencad {
namespace io {

core::Shape BRepReader::readFile(const std::string &filename) {
  TopoDS_Shape shape;
  BRep_Builder builder;

  if (BRepTools::Read(shape, filename.c_str(), builder)) {
    return core::Shape(shape);
  }

  return core::Shape();
}

} // namespace io
} // namespace opencad
