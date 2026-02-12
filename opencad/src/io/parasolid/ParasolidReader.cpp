#include "io/parasolid/ParasolidReader.h"
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRep_Builder.hxx>
#include <Geom_Circle.hxx>
#include <TopoDS_Compound.hxx>
#include <cctype>
#include <fstream>
#include <gp_Ax2.hxx>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


namespace opencad {
namespace io {

ParasolidReader::ParasolidReader() {}
ParasolidReader::~ParasolidReader() {}

bool ParasolidReader::read(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    m_error = "Could not open file";
    return false;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  parse(buffer.str());
  return true;
}

bool ParasolidReader::readFromBuffer(const std::string &buffer) {
  parse(buffer);
  return true;
}

namespace {
bool isFloat(const std::string &s) {
  if (s.empty())
    return false;
  bool hasDigit = false;
  bool hasDecimal = false;
  for (char c : s) {
    if (isdigit(c))
      hasDigit = true;
    else if (c == '.')
      hasDecimal = true;
    else if (c == '-' || c == '+' || c == 'e' || c == 'E') {
    } else
      return false;
  }
  return hasDigit;
}
} // namespace

void ParasolidReader::parse(const std::string &content) {
  m_shapes.clear();
  std::stringstream ss(content);
  std::string line;

  TopoDS_Compound compound;
  BRep_Builder builder;
  builder.MakeCompound(compound);

  int pointsFound = 0;

  // improved scanner: Look for coordinate patterns
  while (std::getline(ss, line)) {
    // Replace parens with spaces
    for (char &c : line)
      if (c == '(' || c == ')' || c == ',')
        c = ' ';

    std::stringstream ls(line);
    std::string token;
    std::vector<double> numbers;

    // Extract numbers from line
    while (ls >> token) {
      if (isFloat(token)) {
        try {
          numbers.push_back(std::stod(token));
        } catch (...) {
        }
      }
    }

    // Heuristic: If we see exactly 3 numbers and "POINT" or just 3 numbers in a
    // suspicious way? Too many false positives with just 3 numbers. Let's stick
    // to lines having "POINT" OR lines having exactly 3 numbers if they are
    // large enough to not be indices?

    bool isPointLine =
        (line.find("POINT") != std::string::npos) ||
        (line.find("VERTEX") != std::string::npos); // Common in debug dumps

    if (isPointLine && numbers.size() >= 3) {
      // Take the last 3 numbers usually? Or first 3?
      // "POINT 1 <x> <y> <z>" -> 4 numbers, index + coords

      if (numbers.size() == 3) {
        gp_Pnt pnt(numbers[0], numbers[1], numbers[2]);
        builder.Add(compound, BRepBuilderAPI_MakeVertex(pnt));
        pointsFound++;
      } else if (numbers.size() == 4) {
        // Index X Y Z
        gp_Pnt pnt(numbers[1], numbers[2], numbers[3]);
        builder.Add(compound, BRepBuilderAPI_MakeVertex(pnt));
        pointsFound++;
      }
    }
  }

  if (pointsFound > 0) {
    core::Shape shape(compound);
    m_shapes.push_back(shape);
  } else {
    // Fallback: Did we at least find the header?
    if (content.find("T_01") != std::string::npos) {
      m_error = "Found T_01 header but no resolvable POINTS.";
    } else {
      m_error = "No T_01 header or geometry matched.";
    }
  }
}

std::vector<core::Shape> ParasolidReader::getAllShapes() const {
  return m_shapes;
}

std::string ParasolidReader::errorMessage() const { return m_error; }

} // namespace io
} // namespace opencad
