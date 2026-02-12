#pragma once

#include "core/geometry/Shape.h"
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <map>
#include <string>
#include <vector>


namespace opencad {
namespace io {

/**
 * @brief Experimental Native Parasolid (.x_t) Reader
 *
 * This is a partial implementation that attempts to read
 * geometric primitives from the text-based XT format.
 * It is NOT a full B-Rep parser (which is extremely complex).
 */
class ParasolidReader {
public:
  ParasolidReader();
  ~ParasolidReader();

  /**
   * @brief Reads a Parasolid text file (.x_t)
   */
  bool read(const std::string &filename);

  /**
   * @brief Reads from a string buffer (useful for embedded data in SolidWorks)
   */
  bool readFromBuffer(const std::string &buffer);

  std::vector<core::Shape> getAllShapes() const;
  std::string errorMessage() const;

private:
  void parse(const std::string &content);

  // Helper to parse specific entities
  void parsePoint(const std::string &line);
  void parseCircle(const std::string &line);
  // ... add more primitive parsers

  std::vector<core::Shape> m_shapes;
  std::string m_error;

  // Temporary storage for parsed geometry before assembly
  std::vector<gp_Pnt> m_points;
};

} // namespace io
} // namespace opencad
