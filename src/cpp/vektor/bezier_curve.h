#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct BezierCurve {
  glm::dvec2 p0, p1, p2, p3;

  static inline void scale(BezierCurve& curve, double scale) noexcept {
    curve.p0 *= scale;
    curve.p1 *= scale;
    curve.p2 *= scale;
    curve.p3 *= scale;
  }
};

struct BezierCurveWithColor {
  BezierCurve curve;
  glm::vec3 color;

  BezierCurveWithColor() = default;
  BezierCurveWithColor(const BezierCurve& curve, glm::vec3 color = glm::vec3(0.0f))
      : curve { curve }, color { color } {}
};