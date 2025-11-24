#pragma once
#include "bezier_curve.h"
#include "image.h"

namespace Renderer {

auto render_greyscale(int, int, const std::vector<BezierCurveWithColor>&, float = 0.0f)
  -> Image::GreyscaleImage;

glm::vec3 compute_curve_color(BezierCurve, const Image::RGBImage&);

auto render_color(int, int, const std::vector<BezierCurveWithColor>&, glm::vec3 = glm::vec3(0.0f))
  -> Image::RGBImage;

};  // namespace Renderer