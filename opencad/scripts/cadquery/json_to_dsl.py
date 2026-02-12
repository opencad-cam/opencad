import cadquery as cq
import json
import os

def log_debug(data):
    try:
        with open("last_ai_json.log", "w", encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
    except Exception as e:
        print(f"Logging failed: {e}")

def build_box(params):
    width = params.get("width", 10)
    height = params.get("height", 10)
    depth = params.get("depth", 10)
    # Support 'size' list for compatibility
    if "size" in params and isinstance(params["size"], list) and len(params["size"]) == 3:
        width, height, depth = params["size"]
        
    return cq.Workplane("XY").box(width, height, depth)

def build_cylinder(params):
    radius = params.get("radius", 5)
    height = params.get("height", 10)
    return cq.Workplane("XY").cylinder(height, radius)

def build_sphere(params):
    radius = params.get("radius", 5)
    return cq.Workplane("XY").sphere(radius)

def build_tube(params):
    outer_radius = params.get("radius", 10)
    # Try to find inner radius, or default to half outer
    inner_radius = params.get("inner_radius", params.get("radius", 10) / 2.0)
    height = params.get("height", 20)
    
    return cq.Workplane("XY").circle(outer_radius).circle(inner_radius).extrude(height)

def build_cone(params):
    radius_bottom = params.get("radius", 10) # Base radius
    radius_top = params.get("radius_top", 0) # Top radius (0 for pointy cone)
    height = params.get("height", 20)
    
    # CadQuery cylinder can be a cone if radii differ? No, there is specific workplane method or using circle lofts.
    # cq.Workplane('XY').circle(r1).workplane(offset=h).circle(r2).loft()
    # Or just use the standard solid creation if available?
    # Simple cone:
    return cq.Solid.makeCone(radius_bottom, radius_top, height)

def apply_operations(workplane, operations):
    if not operations:
        return workplane
    
    # If it's a raw shape/solid (like makeCone), wrap it?
    # Operations usually expect a Workplane object.
    if isinstance(workplane, cq.Solid):
        workplane = cq.Workplane("XY").add(workplane)

    res = workplane
    for op in operations:
        op_type = op.get("type")
        if op_type == "fillet":
            radius = op.get("radius", 1)
            try:
                res = res.edges().fillet(radius)
            except Exception as e:
                print(f"Operation warning (fillet): {e}")
        elif op_type == "chamfer":
            length = op.get("length", 1)
            try:
                res = res.edges().chamfer(length)
            except Exception as e:
                print(f"Operation warning (chamfer): {e}")
                
    return res

def build(json_data):
    """
    Main entry point for the bridge.
    Expected input: a Python dictionary (parsed JSON).
    """
    if isinstance(json_data, str):
        try:
            json_data = json.loads(json_data)
        except json.JSONDecodeError:
            print("Error: Invalid JSON input")
            return None

    # Log the input for debugging
    log_debug(json_data)

    status = json_data.get("status", "error")
    if status != "ok":
        print(f"Skipping build due to status: {status}")
        return None

    intent = json_data.get("intent")
    entities = json_data.get("entities", [])
    params = json_data.get("parameters", {})
    ops = json_data.get("operations", [])

    result = None

    if intent == "create":
        # Simple entity matching
        entity = entities[0].lower() if entities else "box"
        
        # Mapping logic
        if hasattr(entity, "decode"): # Python 2 compat just in case
            entity = entity.decode("utf-8")
            
        print(f"Processing entity: {entity}")

        if any(x in entity for x in ["box", "kutu", "küp", "cube"]):
            result = build_box(params)
            
        elif any(x in entity for x in ["cylinder", "silindir"]):
            result = build_cylinder(params)
            
        elif any(x in entity for x in ["sphere", "küre", "top"]):
            result = build_sphere(params)
            
        elif any(x in entity for x in ["tube", "pipe", "boru", "flute", "flüt"]):
            result = build_tube(params)
            
        elif any(x in entity for x in ["cone", "koni"]):
            result = build_cone(params)
            
        else:
            # Fallback
            print(f"Error: Unknown entity '{entity}'. Cannot build.")
            # Do NOT return a default box, return None so user knows it failed.
            return None

    if result:
        result = apply_operations(result, ops)

    return result
