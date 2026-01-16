/**
 * @file LoftFeature.cpp
 * @brief Loft feature implementation
 */

#include "LoftFeature.h"

#include <BRepOffsetAPI_ThruSections.hxx>
#include <TopoDS.hxx>

namespace opencad {
namespace part {

TopoDS_Shape LoftFeature::execute(const std::vector<TopoDS_Wire>& profiles,
                                   bool solid,
                                   bool ruled) {
    m_error.clear();
    
    if (profiles.size() < 2) {
        m_error = "At least 2 profiles required for loft";
        return TopoDS_Shape();
    }
    
    try {
        BRepOffsetAPI_ThruSections loft(solid, ruled);
        
        // Add all profiles
        for (const auto& profile : profiles) {
            if (!profile.IsNull()) {
                loft.AddWire(profile);
            }
        }
        
        loft.Build();
        
        if (loft.IsDone()) {
            return loft.Shape();
        } else {
            m_error = "Loft operation failed. Profiles may be incompatible.";
            return TopoDS_Shape();
        }
    } catch (const Standard_Failure& e) {
        m_error = "Exception: " + std::string(e.GetMessageString());
        return TopoDS_Shape();
    } catch (...) {
        m_error = "Unknown exception during loft";
        return TopoDS_Shape();
    }
}

} // namespace part
} // namespace opencad
