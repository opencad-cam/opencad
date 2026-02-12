import sys
import os
import json

# Add scripts/cadquery to path
sys.path.append(os.path.abspath(os.path.join(os.getcwd(), 'scripts', 'cadquery')))

try:
    import json_to_dsl
    import cadquery as cq
except ImportError as e:
    print(f"Import Error: {e}")
    sys.exit(1)

def test_box_creation():
    json_input = {
        "status": "ok",
        "intent": "create",
        "entities": ["box"],
        "parameters": {
            "width": 10,
            "height": 20,
            "depth": 5
        },
        "operations": [
            {
                "type": "fillet",
                "radius": 1
            }
        ]
    }
    
    print("Testing Box Creation...")
    try:
        result = json_to_dsl.build(json_input)
        if result:
            print("SUCCESS: Result generated")
            if isinstance(result, (cq.Workplane, cq.Shape, cq.Compound)):
                print("Result is a valid CadQuery object")
            else:
                 print(f"Result type: {type(result)}")
        else:
            print("FAILURE: No result")
    except Exception as e:
        print(f"FAILURE: Exception {e}")

def test_tube_creation():
    # Test "flute" mapping
    json_input = {
        "status": "ok",
        "intent": "create",
        "entities": ["flute"],
        "parameters": {
            "radius": 10,
            "height": 50
        }
    }
    print("\nTesting Tube (Flute) Creation...")
    try:
        result = json_to_dsl.build(json_input)
        if result:
            print("SUCCESS: Tube generated")
        else:
            print("FAILURE: No result for tube")
    except Exception as e:
        print(f"FAILURE: Exception {e}")

def test_ambiguous():
    json_input = {
        "status": "ambiguous",
        "intent": "create"
    }
    print("\nTesting Ambiguous Status...")
    result = json_to_dsl.build(json_input)
    if result is None:
        print("SUCCESS: Correctly returned None for ambiguous status")
    else:
        print("FAILURE: Should have returned None")

if __name__ == "__main__":
    test_box_creation()
    test_tube_creation()
    test_ambiguous()
