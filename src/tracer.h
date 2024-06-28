#pragma once
#include "image.h"
#include <glm/vec2.hpp>

namespace Tracer {

struct BezierCurve {
  glm::vec2 p0, p1, p2, p3;
};

Image::ColorImage trace(const Image::GreyscaleImage&);

} // namespace Tracer