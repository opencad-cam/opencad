/**
 * @file StepReader.cpp
 * @brief STEP file reader implementation
 */

#include "StepReader.h"

#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Iterator.hxx>
#include <BRep_Builder.hxx>

namespace opencad {
namespace io {

StepReader::StepReader() = default;
StepReader::~StepReader() = default;

bool StepReader::read(const std::string& filename) {
    m_shapes.clear();
    m_error.clear();

    STEPControl_Reader reader;
    IFSelect_ReturnStatus status = reader.ReadFile(filename.c_str());

    if (status != IFSelect_RetDone) {
        switch (status) {
            case IFSelect_RetVoid:
                m_error = "No file loaded";
                break;
            case IFSelect_RetError:
                m_error = "Error reading file";
                break;
            case IFSelect_RetFail:
                m_error = "Failed to read file";
                break;
            default:
                m_error = "Unknown error";
        }
        return false;
    }

    // Transfer all roots
    int numRoots = reader.NbRootsForTransfer();
    if (numRoots == 0) {
        m_error = "No shapes found in file";
        return false;
    }

    // Transfer shapes
    reader.TransferRoots();

    int numShapes = reader.NbShapes();
    if (numShapes == 0) {
        m_error = "No shapes transferred";
        return false;
    }

    // Collect all shapes
    for (int i = 1; i <= numShapes; ++i) {
        TopoDS_Shape shape = reader.Shape(i);
        if (!shape.IsNull()) {
            m_shapes.push_back(core::Shape(shape));
        }
    }

    // Create compound if multiple shapes
    if (m_shapes.size() == 1) {
        m_shape = m_shapes[0];
    } else if (m_shapes.size() > 1) {
        BRep_Builder builder;
        TopoDS_Compound compound;
        builder.MakeCompound(compound);
        
        for (const auto& s : m_shapes) {
            builder.Add(compound, s.occShape());
        }
        m_shape = core::Shape(compound);
    }

    return true;
}

core::Shape StepReader::getShape() const {
    return m_shape;
}

std::vector<core::Shape> StepReader::getAllShapes() const {
    return m_shapes;
}

std::string StepReader::errorMessage() const {
    return m_error;
}

} // namespace io
} // namespace opencad
