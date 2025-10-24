#pragma once
#include "bezier_curve.h"
#include "image.h"

namespace Tracer {

auto trace(const Image::GreyscaleImage&) -> std::vector<BezierCurve>;

}  // namespace Tracer