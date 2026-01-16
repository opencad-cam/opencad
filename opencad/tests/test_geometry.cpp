/**
 * @file test_geometry.cpp
 * @brief Unit tests for core geometry module
 */

#include "core/geometry/Shape.h"
#include "core/geometry/Primitives.h"
#include "core/geometry/BooleanOps.h"
#include "core/geometry/Transform.h"

#include <iostream>
#include <cmath>
#include <cassert>

using namespace opencad::core;

// Test helper
#define TEST(name) std::cout << "Testing " << name << "... "; 
#define PASS() std::cout << "PASSED" << std::endl;
#define FAIL(msg) std::cout << "FAILED: " << msg << std::endl; return 1;

const double EPSILON = 1e-6;

bool nearlyEqual(double a, double b, double eps = EPSILON) {
    return std::abs(a - b) < eps;
}

int testBox() {
    TEST("Box creation");
    
    Shape box = Primitives::makeBox(10.0, 20.0, 30.0);
    
    if (box.isNull()) {
        FAIL("Box is null");
    }
    
    if (box.shapeTypeString() != "Solid") {
        FAIL("Box should be a Solid");
    }
    
    // Check volume: 10 * 20 * 30 = 6000
    double vol = box.volume();
    if (!nearlyEqual(vol, 6000.0, 1.0)) {
        FAIL("Box volume incorrect: " + std::to_string(vol));
    }
    
    PASS();
    return 0;
}

int testCylinder() {
    TEST("Cylinder creation");
    
    Shape cyl = Primitives::makeCylinder(5.0, 10.0);
    
    if (cyl.isNull()) {
        FAIL("Cylinder is null");
    }
    
    // Check volume: π * r² * h = π * 25 * 10 = 785.398...
    double vol = cyl.volume();
    double expected = M_PI * 25.0 * 10.0;
    if (!nearlyEqual(vol, expected, 1.0)) {
        FAIL("Cylinder volume incorrect: " + std::to_string(vol));
    }
    
    PASS();
    return 0;
}

int testSphere() {
    TEST("Sphere creation");
    
    Shape sphere = Primitives::makeSphere(10.0);
    
    if (sphere.isNull()) {
        FAIL("Sphere is null");
    }
    
    // Check volume: 4/3 * π * r³ = 4/3 * π * 1000 = 4188.79...
    double vol = sphere.volume();
    double expected = (4.0/3.0) * M_PI * 1000.0;
    if (!nearlyEqual(vol, expected, 10.0)) {
        FAIL("Sphere volume incorrect: " + std::to_string(vol));
    }
    
    PASS();
    return 0;
}

int testBooleanFuse() {
    TEST("Boolean Fuse");
    
    Shape box1 = Primitives::makeBox(0, 0, 0, 10, 10, 10);
    Shape box2 = Primitives::makeBox(5, 0, 0, 10, 10, 10);
    
    Shape fused = BooleanOps::fuse(box1, box2);
    
    if (fused.isNull()) {
        FAIL("Fused shape is null");
    }
    
    // Volume should be less than 2 boxes (1500 < vol < 2000)
    double vol = fused.volume();
    if (vol <= 1000.0 || vol >= 2000.0) {
        FAIL("Fused volume unexpected: " + std::to_string(vol));
    }
    
    PASS();
    return 0;
}

int testBooleanCut() {
    TEST("Boolean Cut");
    
    Shape box = Primitives::makeBox(20.0, 20.0, 20.0);
    Shape cyl = Primitives::makeCylinder(5.0, 30.0);
    
    cyl = Transform::translate(cyl, 0, 0, -5);
    
    Shape result = BooleanOps::cut(box, cyl);
    
    if (result.isNull()) {
        FAIL("Cut result is null");
    }
    
    // Volume should be box - cylinder hole
    double vol = result.volume();
    double boxVol = 20.0 * 20.0 * 20.0;
    double cylVol = M_PI * 25.0 * 20.0;
    double expected = boxVol - cylVol;
    
    if (!nearlyEqual(vol, expected, 50.0)) {
        FAIL("Cut volume unexpected: " + std::to_string(vol) + " vs " + std::to_string(expected));
    }
    
    PASS();
    return 0;
}

int testTranslate() {
    TEST("Translation");
    
    Shape box = Primitives::makeBox(10.0, 10.0, 10.0);
    Shape moved = Transform::translate(box, 100.0, 0.0, 0.0);
    
    if (moved.isNull()) {
        FAIL("Translated shape is null");
    }
    
    double cx, cy, cz;
    moved.centerOfMass(cx, cy, cz);
    
    if (!nearlyEqual(cx, 100.0, 1.0)) {
        FAIL("Center X incorrect: " + std::to_string(cx));
    }
    
    PASS();
    return 0;
}

int testRotation() {
    TEST("Rotation");
    
    Shape box = Primitives::makeBox(10.0, 20.0, 5.0);
    
    // Rotate 90 degrees around Z
    Shape rotated = Transform::rotateZ(box, M_PI / 2.0);
    
    if (rotated.isNull()) {
        FAIL("Rotated shape is null");
    }
    
    // Volume should be preserved
    double origVol = box.volume();
    double rotVol = rotated.volume();
    
    if (!nearlyEqual(origVol, rotVol, 1.0)) {
        FAIL("Volume changed after rotation");
    }
    
    PASS();
    return 0;
}

int main() {
    std::cout << "=== OpenCAD Geometry Tests ===" << std::endl;
    
    int failures = 0;
    
    failures += testBox();
    failures += testCylinder();
    failures += testSphere();
    failures += testBooleanFuse();
    failures += testBooleanCut();
    failures += testTranslate();
    failures += testRotation();
    
    std::cout << "==============================" << std::endl;
    
    if (failures == 0) {
        std::cout << "All tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << failures << " test(s) FAILED!" << std::endl;
        return 1;
    }
}
