#pragma once
#include "image.h"
#include "bezier_curve.h"

namespace Renderer {

auto render_greyscale(int, int, const std::vector<BezierCurve>&)
   -> Image::GreyscaleImage;
   
auto render_color(int, int, const std::vector<BezierCurve>&, const Image::ColorImage&)
   -> Image::ColorImage;

}; // namespace Renderer