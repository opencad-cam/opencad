#include "sketch/Sketch.h"
#include "sketch/entities/SketchLine.h"
#include <iostream>
#include <memory>
#include <vector>


int main() {
  opencad::sketch::Sketch sketch;
  auto line = sketch.addLine(0, 0, 10, 0);
  std::cout << "Before: " << line->length() << std::endl;
  sketch.addLength(line, 20);
  auto status = sketch.solve();
  std::cout << "After solve status: " << (int)status << std::endl;
  std::cout << "After: " << line->length() << std::endl;

  // Print coords
  std::cout << line->startPoint().X() << ", " << line->startPoint().Y()
            << " to " << line->endPoint().X() << ", " << line->endPoint().Y()
            << std::endl;
  return 0;
}
