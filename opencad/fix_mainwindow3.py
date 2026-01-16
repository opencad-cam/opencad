import re

# Read the file
with open(r'c:\Projects\opencadandsimulation\opencad\src\ui\MainWindow.cpp', 'r', encoding='utf-8') as f:
    lines = f.readlines()

# Process line by line to handle all remaining cases
new_lines = []
for line in lines:
    # Skip lines that already have m_document
    if 'm_document' in line and 'm_shapes' in line:
        new_lines.append(line)
        continue
    
    # Replace remaining m_shapes references
    if 'm_shapes' in line:
        # Comment out the line and add TODO
        new_lines.append('      // TODO: Convert to Feature-based system\n')
        new_lines.append('      // ' + line)
        continue
    
    # Replace remaining m_sketches references (except m_sketchView)
    if 'm_sketches' in line and 'm_sketchView' not in line:
        # Comment out the line and add TODO
        new_lines.append('      // TODO: Use m_document->sketches()\n')
        new_lines.append('      // ' + line)
        continue
    
    # Fix m_undoRedoManager checks
    if 'm_undoRedoManager' in line and 'm_document' not in line:
        line = line.replace('m_undoRedoManager', 'm_document->undoRedoManager()')
    
    new_lines.append(line)

# Write back
with open(r'c:\Projects\opencadandsimulation\opencad\src\ui\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.writelines(new_lines)

print("All remaining references commented out!")
