import os
import urllib.request
import urllib.error

icons = [
    # Main
    "file", "folder-open", "device-floppy", "maximize",
    # Sketch
    "pencil", "check", "slash", "square", "circle", "vector-bezier", "point", 
    "scribble", "oval-vertical", "hexagon", "pill", "arrows-right",
    # Constraints
    "arrows-horizontal", "arrows-vertical", "math-perpendicular", "math-equal",
    "focus-2", "ruler-measure", "radius", "angle",
    # Features
    "box-margin", "3d-rotate", "border-radius", "cut", "target",
    # Primitives
    "box", "cylinder", "sphere", "cone", "settings",
    # Boolean
    "plus", "minus", "math-symbols",
    # Selection
    "box-model", "square", "minus", "point",
    # Assembly
    "layout-board", "file-import", "hand-grab", "link", "calculator", 
    "arrows-maximize", "folder", "target", "rotate", "copy", "crosshair"
]

out_dir = r"c:\Projects\opencadandsimulation\opencad\src\ui\icons"
os.makedirs(out_dir, exist_ok=True)

base_url = "https://raw.githubusercontent.com/tabler/tabler-icons/master/icons/outline/{}.svg"

downloaded = []

for icon in icons:
    url = base_url.format(icon)
    out_path = os.path.join(out_dir, icon + ".svg")
    try:
        req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
        with urllib.request.urlopen(req) as response:
            with open(out_path, "wb") as f:
                f.write(response.read())
        print(f"Downloaded {icon}")
        downloaded.append(icon)
    except Exception as e:
        print(f"Failed to download {icon}: {e}, creating default placeholder")
        with open(out_path, "w") as f:
            f.write('<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><circle cx="12" cy="12" r="10"></circle><path d="M9.09 9a3 3 0 0 1 5.83 1c0 2-3 3-3 3"></path><line x1="12" y1="17" x2="12.01" y2="17"></line></svg>')
        downloaded.append(icon)

# Generate resources.qrc
qrc_path = r"c:\Projects\opencadandsimulation\opencad\src\ui\resources.qrc"
with open(qrc_path, "w") as f:
    f.write('<!DOCTYPE RCC><RCC version="1.0">\n')
    f.write('<qresource prefix="/">\n')
    for icon in downloaded:
        f.write(f'    <file>icons/{icon}.svg</file>\n')
    f.write('</qresource>\n')
    f.write('</RCC>\n')

print(f"Created {qrc_path} with {len(downloaded)} icons")
