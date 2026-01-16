// Draw profile overlays in ProfileSelect mode (highlight selected profiles)
void SketchView2D::drawProfileOverlays(QPainter &painter) {
  if (!m_sketch || m_profiles.empty())
    return;

  painter.save();

  // Draw all profiles with different colors
  for (size_t i = 0; i < m_profiles.size(); ++i) {
    const TopoDS_Wire &wire = m_profiles[i];

    // Check if this profile is selected
    bool isSelected = false;
    for (const auto &sel : m_selectedProfiles) {
      if (sel.first == static_cast<int>(i)) {
        isSelected = true;
        break;
      }
    }

    // Check if this profile is hovered
    bool isHovered = (m_hoveredProfileIndex == static_cast<int>(i));

    // Choose color based on state
    QColor outlineColor;
    int lineWidth;
    if (isSelected) {
      outlineColor = QColor(0, 255, 0, 255); // Bright green for selected
      lineWidth = 4;
    } else if (isHovered) {
      outlineColor = QColor(255, 255, 0, 255); // Yellow for hovered
      lineWidth = 3;
    } else {
      // Use profile color from palette
      outlineColor = m_profileColors[i % m_profileColors.size()];
      lineWidth = 2;
    }

    painter.setPen(QPen(outlineColor, lineWidth));
    painter.setBrush(Qt::NoBrush);

    // Draw wire outline
    try {
      TopExp_Explorer edgeExp(wire, TopAbs_EDGE);

      while (edgeExp.More()) {
        TopoDS_Edge edge = static_cast<const TopoDS_Edge &>(edgeExp.Current());

        // Get edge curve and sample points
        Standard_Real first, last;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);

        if (!curve.IsNull()) {
          // Sample points along the edge
          int numSamples = 30;
          QPointF prevPt;
          for (int j = 0; j <= numSamples; ++j) {
            double param = first + (last - first) * j / numSamples;
            gp_Pnt point3D;
            curve->D0(param, point3D);

            // Convert 3D point to 2D sketch coordinates
            gp_Pnt2d point2D(point3D.X(), point3D.Y());
            QPointF screenPt = worldToScreen(point2D);

            if (j > 0) {
              painter.drawLine(prevPt, screenPt);
            }
            prevPt = screenPt;
          }
        }

        edgeExp.Next();
      }
    } catch (...) {
      // Skip invalid profiles
      continue;
    }
  }

  painter.restore();
}
