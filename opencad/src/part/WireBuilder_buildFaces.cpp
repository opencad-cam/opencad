
TopoDS_Shape WireBuilder::buildFaces(const std::vector<TopoDS_Wire> &wires) {
  if (wires.empty())
    return TopoDS_Shape();

  // 1. Create planar face for each wire to check areas and containment
  struct WireInfo {
    TopoDS_Wire wire;
    TopoDS_Face tempFace;
    double area;
    bool used;
  };

  std::vector<WireInfo> wireInfos;
  for (const auto &wire : wires) {
    if (wire.IsNull())
      continue;

    BRepBuilderAPI_MakeFace mkFace(wire, true);
    // true = only planar
    if (mkFace.IsDone()) {
      TopoDS_Face face = mkFace.Face();
      GProp_GProps props;
      BRepGProp::SurfaceProperties(face, props);
      wireInfos.push_back({wire, face, props.Mass(), false});
    }
  }

  // 2. Sort by Area Descending (Largest first)
  std::sort(
      wireInfos.begin(), wireInfos.end(),
      [](const WireInfo &a, const WireInfo &b) { return a.area > b.area; });

  TopoDS_Compound resultCompound;
  BRep_Builder builder;
  builder.MakeCompound(resultCompound);
  bool addedAny = false;

  // 3. Process wires: Largest are Outers, smaller inside them are Holes
  for (size_t i = 0; i < wireInfos.size(); ++i) {
    if (wireInfos[i].used)
      continue;

    // Current largest is Outer
    WireInfo &outer = wireInfos[i];
    outer.used = true;

    BRepBuilderAPI_MakeFace faceBuilder(outer.wire, true);

    // Check remaining wires to see if they are holes inside this outer
    for (size_t j = i + 1; j < wireInfos.size(); ++j) {
      if (wireInfos[j].used)
        continue;

      WireInfo &potentialHole = wireInfos[j];

      // Check if hole is inside outer
      // Use a point from hole's wire/face
      // Get center or a vertex? BRepClass_FaceClassifier checks a point.
      // Let's use a point on the wire of the hole.

      // Or easier: Vertex of hole
      TopExp_Explorer exp(potentialHole.wire, TopAbs_VERTEX);
      if (exp.More()) {
        TopoDS_Vertex v = TopoDS::Vertex(exp.Current());
        gp_Pnt p = BRep_Tool::Pnt(v);

        BRepClass_FaceClassifier classifier(outer.tempFace, p, 1e-6);
        if (classifier.State() == TopAbs_IN) {
          // It is a hole!
          faceBuilder.Add(potentialHole.wire);
          potentialHole.used = true;
        }
      }
    }

    if (faceBuilder.IsDone()) {
      builder.Add(resultCompound, faceBuilder.Face());
      addedAny = true;
    }
  }

  if (!addedAny)
    return TopoDS_Shape();

  // If only one face in compound, return the face directly
  TopExp_Explorer exp(resultCompound, TopAbs_FACE);
  if (exp.More()) {
    TopoDS_Shape firstFace = exp.Current();
    exp.Next();
    if (!exp.More()) {
      return firstFace;
    }
  }

  return resultCompound;
}
