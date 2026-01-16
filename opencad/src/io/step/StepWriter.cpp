/**
 * @file StepWriter.cpp
 * @brief STEP file writer implementation
 */

#include "StepWriter.h"

#include <STEPControl_Writer.hxx>
#include <STEPControl_StepModelType.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Interface_Static.hxx>

namespace opencad {
namespace io {

StepWriter::StepWriter() = default;
StepWriter::~StepWriter() = default;

void StepWriter::setVersion(StepVersion version) {
    m_version = version;
}

bool StepWriter::write(const core::Shape& shape, const std::string& filename) {
    m_error.clear();

    if (shape.isNull()) {
        m_error = "Cannot export null shape";
        return false;
    }

    STEPControl_Writer writer;

    // Set STEP version
    switch (m_version) {
        case StepVersion::AP203:
            Interface_Static::SetCVal("write.step.schema", "AP203");
            break;
        case StepVersion::AP214:
            Interface_Static::SetCVal("write.step.schema", "AP214IS");
            break;
        case StepVersion::AP242:
            Interface_Static::SetCVal("write.step.schema", "AP242DIS");
            break;
    }

    // Transfer shape
    IFSelect_ReturnStatus status = writer.Transfer(
        shape.occShape(), 
        STEPControl_AsIs
    );

    if (status != IFSelect_RetDone) {
        m_error = "Failed to transfer shape for STEP export";
        return false;
    }

    // Write file
    status = writer.Write(filename.c_str());

    if (status != IFSelect_RetDone) {
        m_error = "Failed to write STEP file";
        return false;
    }

    return true;
}

std::string StepWriter::errorMessage() const {
    return m_error;
}

} // namespace io
} // namespace opencad
