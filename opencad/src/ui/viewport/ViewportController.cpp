/**
 * @file ViewportController.cpp
 * @brief Viewport controller implementation
 */

#include "ViewportController.h"

#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>

namespace opencad {
namespace ui {

ViewportController::ViewportController() = default;
ViewportController::~ViewportController() = default;

void ViewportController::setContext(Handle(AIS_InteractiveContext) context) {
    m_context = context;
}

int ViewportController::addShape(const core::Shape& shape) {
    if (shape.isNull() || m_context.IsNull()) {
        return -1;
    }

    Handle(AIS_Shape) aisShape = new AIS_Shape(shape.occShape());
    m_context->Display(aisShape, AIS_Shaded, 0, true);
    m_context->SetMaterial(aisShape, Graphic3d_NOM_STEEL, false);

    m_shapes.push_back({shape, aisShape});
    return static_cast<int>(m_shapes.size() - 1);
}

void ViewportController::removeShape(int index) {
    if (index < 0 || index >= static_cast<int>(m_shapes.size())) {
        return;
    }

    if (!m_context.IsNull()) {
        m_context->Remove(m_shapes[index].second, false);
    }
    m_shapes.erase(m_shapes.begin() + index);
}

core::Shape ViewportController::getShape(int index) const {
    if (index < 0 || index >= static_cast<int>(m_shapes.size())) {
        return core::Shape();
    }
    return m_shapes[index].first;
}

int ViewportController::shapeCount() const {
    return static_cast<int>(m_shapes.size());
}

std::vector<int> ViewportController::getSelection() const {
    std::vector<int> selected;
    
    if (m_context.IsNull()) {
        return selected;
    }

    m_context->InitSelected();
    while (m_context->MoreSelected()) {
        Handle(AIS_InteractiveObject) obj = m_context->SelectedInteractive();
        
        for (size_t i = 0; i < m_shapes.size(); ++i) {
            if (m_shapes[i].second == obj) {
                selected.push_back(static_cast<int>(i));
                break;
            }
        }
        m_context->NextSelected();
    }

    return selected;
}

void ViewportController::clearSelection() {
    if (!m_context.IsNull()) {
        m_context->ClearSelected(true);
    }
}

void ViewportController::selectShape(int index) {
    if (index < 0 || index >= static_cast<int>(m_shapes.size())) {
        return;
    }

    if (!m_context.IsNull()) {
        m_context->AddOrRemoveSelected(m_shapes[index].second, true);
    }
}

void ViewportController::setDisplayMode(bool shaded) {
    if (m_context.IsNull()) return;

    for (auto& pair : m_shapes) {
        m_context->SetDisplayMode(pair.second, shaded ? AIS_Shaded : AIS_WireFrame, false);
    }
    m_context->UpdateCurrentViewer();
}

void ViewportController::setVisible(int index, bool visible) {
    if (index < 0 || index >= static_cast<int>(m_shapes.size())) {
        return;
    }

    if (m_context.IsNull()) return;

    if (visible) {
        m_context->Display(m_shapes[index].second, false);
    } else {
        m_context->Erase(m_shapes[index].second, false);
    }
    m_context->UpdateCurrentViewer();
}

} // namespace ui
} // namespace opencad
