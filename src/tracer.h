#pragma once
#include "image.h"
#include "bezier_curve.h"

namespace Tracer {

auto trace(const Image::GreyscaleImage&) -> std::vector<BezierCurve>;

} // namespace Tracer