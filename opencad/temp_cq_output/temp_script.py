import cadquery as cq

heel_height = 70
bottom_width = 30
bottom_depth = 35
top_width = 20
top_depth = 25
fillet_radius = 3

result = (
    cq.Workplane("XY").ellipse(bottom_width / 2, bottom_depth / 2)
    .workplane(offset=heel_height)
    .ellipse(top_width / 2, top_depth / 2)
    .loft(combine=True)
    .edges() 
    .fillet(fillet_radius)
)
