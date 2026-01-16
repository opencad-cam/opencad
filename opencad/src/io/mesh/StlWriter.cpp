/**
 * @file StlWriter.cpp
 * @brief STL file writer implementation
 */

#include "StlWriter.h"

#include <StlAPI_Writer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>

namespace opencad {
namespace io {

StlWriter::StlWriter() = default;
StlWriter::~StlWriter() = default;

void StlWriter::setDeflection(double deflection) {
    m_deflection = deflection;
}

void StlWriter::setAsciiMode(bool ascii) {
    m_ascii = ascii;
}

bool StlWriter::write(const core::Shape& shape, const std::string& filename) {
    m_error.clear();

    if (shape.isNull()) {
        m_error = "Cannot export null shape";
        return false;
    }

    // Mesh the shape first
    BRepMesh_IncrementalMesh mesher(shape.occShape(), m_deflection);
    mesher.Perform();

    if (!mesher.IsDone()) {
        m_error = "Failed to mesh shape for STL export";
        return false;
    }

    // Write STL
    StlAPI_Writer writer;
    writer.ASCIIMode() = m_ascii;

    if (!writer.Write(shape.occShape(), filename.c_str())) {
        m_error = "Failed to write STL file";
        return false;
    }

    return true;
}

std::string StlWriter::errorMessage() const {
    return m_error;
}

} // namespace io
} // namespace opencad
