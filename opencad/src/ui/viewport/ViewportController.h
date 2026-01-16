#pragma once
/**
 * @file ViewportController.h
 * @brief Viewport interaction and selection controller
 * 
 * OpenCAD - Modular CAD/CAE Platform
 * UI Module
 */

#include "core/geometry/Shape.h"
#include <AIS_InteractiveContext.hxx>
#include <vector>

namespace opencad {
namespace ui {

/**
 * @class ViewportController
 * @brief Controls viewport interaction, selection, and shape management
 */
class ViewportController {
public:
    ViewportController();
    ~ViewportController();

    /**
     * @brief Set the AIS context to control
     */
    void setContext(Handle(AIS_InteractiveContext) context);

    /**
     * @brief Add shape to the scene
     * @return Index of added shape
     */
    int addShape(const core::Shape& shape);

    /**
     * @brief Remove shape by index
     */
    void removeShape(int index);

    /**
     * @brief Get shape by index
     */
    core::Shape getShape(int index) const;

    /**
     * @brief Get number of shapes
     */
    int shapeCount() const;

    /**
     * @brief Get selected shapes
     */
    std::vector<int> getSelection() const;

    /**
     * @brief Clear selection
     */
    void clearSelection();

    /**
     * @brief Select shape by index
     */
    void selectShape(int index);

    /**
     * @brief Set display mode (shaded, wireframe)
     */
    void setDisplayMode(bool shaded);

    /**
     * @brief Toggle visibility of a shape
     */
    void setVisible(int index, bool visible);

private:
    Handle(AIS_InteractiveContext) m_context;
    std::vector<std::pair<core::Shape, Handle(AIS_InteractiveObject)>> m_shapes;
};

} // namespace ui
} // namespace opencad
