#pragma once
#include "bezier_curve.h"
#include "image.h"

namespace Renderer {

auto render_greyscale(int, int, const std::vector<BezierCurve>&) -> Image::GreyscaleImage;

auto render_color(int, int, const std::vector<BezierCurve>&, const Image::ColorImage&)
  -> Image::ColorImage;

};  // namespace Renderer