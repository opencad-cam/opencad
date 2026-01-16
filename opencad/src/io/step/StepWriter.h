#pragma once
/**
 * @file StepWriter.h
 * @brief STEP file export
 * 
 * OpenCAD - Modular CAD/CAE Platform
 * I/O Module
 */

#include "core/geometry/Shape.h"
#include <string>

namespace opencad {
namespace io {

/**
 * @class StepWriter
 * @brief Writes STEP (ISO 10303) files
 */
class StepWriter {
public:
    enum class StepVersion {
        AP203,  // Configuration controlled design
        AP214,  // Automotive design
        AP242   // Model based 3D engineering
    };

    StepWriter();
    ~StepWriter();

    /**
     * @brief Set STEP version/AP
     */
    void setVersion(StepVersion version);

    /**
     * @brief Write shape to STEP file
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
    StepVersion m_version = StepVersion::AP214;
    std::string m_error;
};

} // namespace io
} // namespace opencad
