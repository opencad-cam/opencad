#pragma once
/**
 * @file CutFeature.h
 * @brief Cut/Pocket - remove material by extruding sketch
 * 
 * OpenCAD - Modular CAD/CAE Platform
 */

#include <TopoDS_Shape.hxx>
#include <memory>

namespace opencad {
namespace sketch { class Sketch; }
namespace part {

/**
 * @class CutFeature
 * @brief Creates a pocket/cut by extruding and subtracting sketch profile
 */
class CutFeature {
public:
    CutFeature() = default;
    
    /// Cut from a base shape using sketch profile
    /// @param sketch The 2D sketch profile to cut
    /// @param base The solid to cut from
    /// @param depth Cut depth (will cut inward)
    /// @param throughAll If true, cut through entire body
    /// @return The resulting cut shape
    TopoDS_Shape execute(const sketch::Sketch& sketch,
                         const TopoDS_Shape& base,
                         double depth,
                         bool throughAll = false);
    
    /// Get last error message
    const std::string& errorMessage() const { return m_error; }
    
private:
    std::string m_error;
};

} // namespace part
} // namespace opencad
