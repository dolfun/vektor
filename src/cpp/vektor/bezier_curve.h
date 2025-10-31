#pragma once
#include <glm/vec2.hpp>

struct BezierCurve {
  glm::dvec2 p0, p1, p2, p3;

  void apply_scale(double val) {
    p0 *= val;
    p1 *= val;
    p2 *= val;
    p3 *= val;
  }
};