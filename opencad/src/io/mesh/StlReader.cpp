#include "io/mesh/StlReader.h"
#include <BRepBuilderAPI_MakeFace.hxx>
#include <Poly_Triangulation.hxx>
#include <QDebug>
#include <StlAPI_Reader.hxx>
#include <TCollection_AsciiString.hxx>
#include <TopoDS_Shape.hxx>


namespace opencad {
namespace io {

TopoDS_Shape StlReader::readFile(const QString &filename) {
  qDebug() << "Reading STL file:" << filename;

  // Create reader
  StlAPI_Reader reader;
  TopoDS_Shape shape;

  // Convert QString to OCCT string
  TCollection_AsciiString path(filename.toUtf8().constData());

  // Read the file
  // StlAPI_Reader reads into a specific shape if provided, or creates a
  // compound of faces
  bool result = reader.Read(shape, path.ToCString());

  if (!result) {
    qDebug() << "Failed to read STL file";
    return TopoDS_Shape();
  }

  if (shape.IsNull()) {
    qDebug() << "STL read resulted in null shape";
    return TopoDS_Shape();
  }

  qDebug() << "STL read successfully";
  return shape;
}

} // namespace io
} // namespace opencad
