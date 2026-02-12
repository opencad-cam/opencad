import machine_generator
import cadquery as cq

# Test cases based on new Modelfile examples
tests = [
    {
        "machine": "primitive_part",
        "type": "box",
        "parameters": {"size": [10, 20, 30]}
    },
    {
        "machine": "primitive_part",
        "type": "tube",
        "parameters": {"radius": 20, "inner_radius": 10, "height": 5}
    },
    {
        "machine": "primitive_part",
        "type": "cylinder",
        "parameters": {"radius": 5, "height": 50}
    },
    {
        "machine": "primitive_part",
        "type": "sphere",
        "parameters": {"radius": 15}
    }
]

print("Running generator tests...")
for i, config in enumerate(tests):
    print(f"Test {i+1}: {config['type']}")
    try:
        result = machine_generator.build(config)
        if isinstance(result, (cq.Workplane, cq.Shape, cq.Assembly)):
            print("  SUCCESS: Generated valid CQ object")
        else:
            print(f"  FAILURE: Returned {type(result)}")
    except Exception as e:
        print(f"  ERROR: {e}")

print("Done.")
