import cadquery as cq

WIDTH = 600
DEPTH = 500
HEIGHT = 600
PROFILE = 30
BED_THICKNESS = 10

posts = (
    cq.Workplane("XY")
    .pushPoints([
        ( WIDTH/2 - PROFILE/2,  DEPTH/2 - PROFILE/2),
        (-WIDTH/2 + PROFILE/2,  DEPTH/2 - PROFILE/2),
        ( WIDTH/2 - PROFILE/2, -DEPTH/2 + PROFILE/2),
        (-WIDTH/2 + PROFILE/2, -DEPTH/2 + PROFILE/2),
    ])
    .rect(PROFILE, PROFILE)
    .extrude(HEIGHT)
)

frame_x_bottom = (
    cq.Workplane("XY")
    .pushPoints([(0,  DEPTH/2 - PROFILE/2), (0, -DEPTH/2 + PROFILE/2)])
    .rect(WIDTH, PROFILE)
    .extrude(PROFILE)
)

frame_x_top = frame_x_bottom.translate((0, 0, HEIGHT - PROFILE))

frame_y_bottom = (
    cq.Workplane("XY")
    .pushPoints([( WIDTH/2 - PROFILE/2, 0), (-WIDTH/2 + PROFILE/2, 0)])
    .rect(PROFILE, DEPTH)
    .extrude(PROFILE)
)

frame_y_top = frame_y_bottom.translate((0, 0, HEIGHT - PROFILE))

bed = (
    cq.Workplane("XY")
    .workplane(offset=PROFILE)
    .rect(WIDTH - 100, DEPTH - 100)
    .extrude(BED_THICKNESS)
)

result = (
    posts
    .union(frame_x_bottom)
    .union(frame_x_top)
    .union(frame_y_bottom)
    .union(frame_y_top)
    .union(bed)
)
