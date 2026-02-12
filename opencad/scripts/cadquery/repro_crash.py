import cadquery as cq

# Parametric variables
num_teeth = 12
radius = 50
thickness = 10

# Create a gear-like shape manually since 'gear' isn't standard
# This mimics what the AI might try to do
result = (
    cq.Workplane("XY")
    .polarArray(radius, 360, num_teeth)
    .circle(5)
    .extrude(thickness)
)

# And a main body
body = cq.Workplane("XY").circle(radius - 5).extrude(thickness)

# Union
result = body.union(result)
