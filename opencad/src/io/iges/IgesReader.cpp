#include "io/iges/IgesReader.h"
#include <IGESControl_Reader.hxx>
#include <Interface_Static.hxx>
#include <TopoDS_Shape.hxx>
#include <XSControl_TransferReader.hxx>
#include <XSControl_WorkSession.hxx>


namespace opencad {
namespace io {

IgesReader::IgesReader() {}

IgesReader::~IgesReader() {}

bool IgesReader::read(const std::string &filename) {
  IGESControl_Reader reader;

  // Set read precision mode if needed, but defaults are usually fine

  int status = reader.ReadFile(filename.c_str());
  if (status != IFSelect_RetDone) {
    m_error =
        "Failed to read IGES file (Status: " + std::to_string(status) + ")";
    return false;
  }

  // Transfer roots
  reader.TransferRoots();

  int numShapes = reader.NbShapes();
  if (numShapes == 0) {
    m_error = "No shapes found in IGES file";
    return false;
  }

  m_shapes.clear();
  for (int i = 1; i <= numShapes; ++i) {
    TopoDS_Shape shape = reader.Shape(i);
    if (!shape.IsNull()) {
      m_shapes.push_back(core::Shape(shape));
    }
  }

  if (m_shapes.empty()) {
    m_error = "Failed to transfer shapes";
    return false;
  }

  return true;
}

core::Shape IgesReader::getShape() const {
  if (m_shapes.empty()) {
    return core::Shape();
  }
  return m_shapes[0];
}

std::vector<core::Shape> IgesReader::getAllShapes() const { return m_shapes; }

std::string IgesReader::errorMessage() const { return m_error; }

} // namespace io
} // namespace opencad
