import sys
import os
import argparse
import traceback

def main():
    parser = argparse.ArgumentParser(description='Run a CadQuery script and export to STEP.')
    parser.add_argument('--script', required=True, help='Path to the CadQuery script file')
    parser.add_argument('--output', required=True, help='Path to the output STEP file')
    args = parser.parse_args()

    script_path = args.script
    output_path = args.output

    if not os.path.exists(script_path):
        print(f"Error: Script file not found at {script_path}", file=sys.stderr)
        sys.exit(1)

    with open(script_path, 'r', encoding='utf-8') as f:
        script_content = f.read()

    # Fallback cleanup: Strip markdown backticks if they leaked through
    def clean_code(code):
        import re
        # Find first ```python or ``` and last ```
        code = code.strip()
        if code.startswith("```"):
            # Remove start tag
            code = re.sub(r'^```(python)?\n?', '', code, flags=re.IGNORECASE)
            # Remove end tag if exists
            if code.endswith("```"):
                code = code[:-3]
        return code.strip()

    script_content = clean_code(script_content)

    # Context for execution
    # We want to catch what the user intended to show.
    # Standard CQ usage often ends with show_object(result) or just result = ...
    # We will look for 'result' variable in local scope after execution.

    ctx = {}
    
    # Add valid import paths if needed, though usually standard python env handles this.
    try:
        import cadquery as cq
        ctx['cq'] = cq
    except ImportError:
        print("Error: cadquery module not found. Please ensure it is installed in your Python environment.", file=sys.stderr)
        sys.exit(1)

    try:
        exec(script_content, ctx)
    except Exception as e:
        print("Error executing script:", file=sys.stderr)
        traceback.print_exc()
        sys.exit(1)

    # Extract result
    # Priority 1: 'result' variable
    # Priority 2: Use the last accessible Workplane object if possible (harder to detect reliably without parsing)
    
    result_obj = ctx.get('result')

    if result_obj is None:
        # Check if they used show_object mock? 
        # For now, let's enforce 'result' variable usage or 'part'
        result_obj = ctx.get('part')

    if result_obj is None:
         print("Error: No 'result' or 'part' variable found in script scope. Please assign your final object to 'result'.", file=sys.stderr)
         sys.exit(1)

    # Export
    try:
        # If it's a Workplane, we can export directly. 
        # If it's a Shape, we can utilize Workplane wrapper or direct export if supported.
        
        # Ensure directory exists
        out_dir = os.path.dirname(output_path)
        if out_dir and not os.path.exists(out_dir):
            os.makedirs(out_dir)

        # Use CQ's export
        # exporters.export(result_obj, output_path, exporters.ExportTypes.STEP)
        from cadquery import exporters
        exporters.export(result_obj, output_path, exporters.ExportTypes.STEP)
        
        print(f"Successfully exported to {output_path}")

    except Exception as e:
        print("Error during export:", file=sys.stderr)
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()
