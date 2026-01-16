import re

# Read the file
with open(r'c:\Projects\opencadandsimulation\opencad\src\ui\MainWindow.cpp', 'r', encoding='utf-8') as f:
    content = f.read()

# Replace m_shapes with document getAllShapes() where appropriate
# For checks like m_shapes.empty()
content = re.sub(r'm_shapes\.empty\(\)', r'm_document->getAllShapes().empty()', content)

# For iterations like for (const auto &shape : m_shapes)
content = re.sub(r'for \(const auto &shape : m_shapes\)', r'for (const auto &shape : m_document->getAllShapes())', content)

# For m_shapes.back()
content = re.sub(r'm_shapes\.back\(\)', r'm_document->getAllShapes().back()', content)

# For m_shapes[0] or m_shapes[index]
content = re.sub(r'm_shapes\[(\d+)\]', r'm_document->getAllShapes()[\1]', content)

# Replace m_sketches with m_document->sketches()
content = re.sub(r'm_sketches\.push_back', r'm_document->addSketch', content)
content = re.sub(r'for \(const auto &sketch : m_sketches\)', r'for (const auto &sketch : m_document->sketches())', content)

# Replace m_undoRedoManager with m_document->undoRedoManager()
content = re.sub(r'm_undoRedoManager->', r'm_document->undoRedoManager()->', content)
content = re.sub(r'if \(m_undoRedoManager\)', r'if (m_document->undoRedoManager())', content)

# Write back
with open(r'c:\Projects\opencadandsimulation\opencad\src\ui\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.write(content)

print("Replacements completed!")
