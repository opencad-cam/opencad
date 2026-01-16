#pragma once
/**
 * @file StepReader.h
 * @brief STEP file import
 * 
 * OpenCAD - Modular CAD/CAE Platform
 * I/O Module
 */

#include "core/geometry/Shape.h"
#include <string>
#include <vector>

namespace opencad {
namespace io {

/**
 * @class StepReader
 * @brief Reads STEP (ISO 10303) files
 */
class StepReader {
public:
    StepReader();
    ~StepReader();

    /**
     * @brief Read STEP file
     * @param filename Path to STEP file
     * @return True if successful
     */
    bool read(const std::string& filename);

    /**
     * @brief Get the loaded shape
     */
    core::Shape getShape() const;

    /**
     * @brief Get all shapes (for multi-shape files)
     */
    std::vector<core::Shape> getAllShapes() const;

    /**
     * @brief Get error message if read failed
     */
    std::string errorMessage() const;

private:
    core::Shape m_shape;
    std::vector<core::Shape> m_shapes;
    std::string m_error;
};

} // namespace io
} // namespace opencad
