#pragma once

#include "core/geometry/Shape.h"
#include <string>
#include <vector>


namespace opencad {
namespace io {

/**
 * @brief Experimental SolidWorks Reader (.sldprt, .sldasm)
 *
 * Attempts to extract embedded body data (Parasolid) or
 * metadata from the binary OLE compound file.
 */
class SolidWorksReader {
public:
  SolidWorksReader();
  ~SolidWorksReader();

  /**
   * @brief Reads a SolidWorks file
   */
  bool read(const std::string &filename);

  std::vector<core::Shape> getAllShapes() const;
  std::string errorMessage() const;

private:
  std::vector<core::Shape> m_shapes;
  std::string m_error;
};

} // namespace io
} // namespace opencad
