import re

# Read the file
with open(r'c:\Projects\opencadandsimulation\opencad\src\ui\MainWindow.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# Additional replacements for missed cases
# m_shapes.size()
content = re.sub(r'm_shapes\.size\(\)', r'm_document->getAllShapes().size()', content)

# m_shapes.push_back(...)
content = re.sub(r'm_shapes\.push_back\(([^)]+)\)', r'// TODO: Convert to Feature\n      // m_shapes.push_back(\1)', content)

# m_shapes.pop_back()
content = re.sub(r'm_shapes\.pop_back\(\)', r'// TODO: Remove last feature\n      // m_shapes.pop_back()', content)

# m_shapes.erase(...)
content = re.sub(r'm_shapes\.erase\(([^)]+)\)', r'// TODO: Remove features\n      // m_shapes.erase(\1)', content)

# m_sketches.size()
content = re.sub(r'm_sketches\.size\(\)', r'm_document->sketches().size()', content)

# m_sketches[index]
content = re.sub(r'm_sketches\[([^\]]+)\]', r'm_document->sketches()[\1]', content)

# Write back
with open(r'c:\Projects\opencadandsimulation\opencad\src\ui\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("Additional replacements completed!")
