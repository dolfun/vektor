#pragma once
#include <glm/vec2.hpp>

struct BezierCurve {
  glm::vec2 p0, p1, p2, p3;

  void apply_scale(float val) {
    p0 *= val;
    p1 *= val;
    p2 *= val;
    p3 *= val;
  }
};