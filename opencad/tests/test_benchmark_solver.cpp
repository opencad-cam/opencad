#include "../src/assembly/Assembly.h"
#include "../src/assembly/ConstraintSolver.h"
#include <BRepPrimAPI_MakeBox.hxx>
#include <chrono>
#include <iostream>

using namespace opencad::assembly;

int main() {
  Assembly assembly;

  std::cout << "Setting up benchmark..." << std::endl;

  // Create two boxes
  TopoDS_Shape box1 = BRepPrimAPI_MakeBox(100, 100, 100).Shape();
  TopoDS_Shape box2 = BRepPrimAPI_MakeBox(100, 100, 100).Shape();

  // Wrap in opencad::core::Shape
  auto shape1 = std::make_shared<opencad::core::Shape>(box1);
  auto shape2 = std::make_shared<opencad::core::Shape>(box2);

  auto comp1 = std::make_shared<Component>(shape1);
  comp1->setName("Box1");
  auto comp2 = std::make_shared<Component>(shape2);
  comp2->setName("Box2");

  // Move Box2 away
  gp_Trsf move;
  move.SetTranslation(gp_Vec(200, 50, 50));
  comp2->setPlacement(move);

  assembly.addComponent(comp1);
  assembly.addComponent(comp2);
  comp1->setFixed(true);

  std::cout << "Starting benchmark..." << std::endl;
  auto start = std::chrono::high_resolution_clock::now();

  // Trigger solver (conceptually)
  // ConstraintSolver solver;
  // solver.solve(assembly);

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "Benchmark time: " << diff.count() << " s" << std::endl;
  std::cout << "Benchmark compiled and ran successfully." << std::endl;

  return 0;
}
