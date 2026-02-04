#pragma once

#include "core/geometry/Shape.h"
#include <string>
#include <vector>


namespace opencad {
namespace io {

class IgesReader {
public:
  IgesReader();
  ~IgesReader();

  /**
   * @brief Reads an IGES file
   * @param filename Path to the IGES file
   * @return True if successful
   */
  bool read(const std::string &filename);

  /**
   * @brief Get the loaded shape (if single)
   */
  core::Shape getShape() const;

  /**
   * @brief Get all loaded shapes
   */
  std::vector<core::Shape> getAllShapes() const;

  /**
   * @brief Get error message
   */
  std::string errorMessage() const;

private:
  std::vector<core::Shape> m_shapes;
  std::string m_error;
};

} // namespace io
} // namespace opencad
