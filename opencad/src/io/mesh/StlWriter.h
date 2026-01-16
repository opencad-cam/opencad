#pragma once
/**
 * @file StlWriter.h
 * @brief STL file export for 3D printing and mesh processing
 * 
 * OpenCAD - Modular CAD/CAE Platform
 * I/O Module
 */

#include "core/geometry/Shape.h"
#include <string>

namespace opencad {
namespace io {

/**
 * @class StlWriter
 * @brief Writes STL (STereoLithography) mesh files
 */
class StlWriter {
public:
    StlWriter();
    ~StlWriter();

    /**
     * @brief Set mesh deflection (accuracy)
     * Lower = more triangles, better accuracy
     * @param deflection Linear deflection value
     */
    void setDeflection(double deflection);

    /**
     * @brief Enable/disable ASCII format (default: binary)
     */
    void setAsciiMode(bool ascii);

    /**
     * @brief Write shape as STL
     * @param shape Shape to export
     * @param filename Output file path
     * @return True if successful
     */
    bool write(const core::Shape& shape, const std::string& filename);

    /**
     * @brief Get error message if write failed
     */
    std::string errorMessage() const;

private:
    double m_deflection = 0.1;
    bool m_ascii = false;
    std::string m_error;
};

} // namespace io
} // namespace opencad
