#pragma once
#include "image.h"
#include "kernel.h"

namespace Canny {

// Scharr 5x5
// https://www.hlevkin.com/hlevkin/47articles/SobelScharrGradients5x5.pdf
// clang-format off
constexpr Image::Kernel<5> gradient_x_kernel {
  {
    -1, -1, 0, 1, 1,
    -2, -2, 0, 2, 2,
    -3, -6, 0, 6, 3,
    -2, -2, 0, 2, 2,
    -1, -1, 0, 1, 1,
  },
  60
};

constexpr Image::Kernel<5> gradient_y_kernel {
  {
    -1, -2, -3, -2, -1,
    -1, -2, -6, -2, -1,
     0,  0,  0,  0,  0,
    +1, +2, +6, +2, +1,
    +1, +2, +3, +2, +1,
  },
  60
};
// clang-format on

constexpr int padding_requirement = gradient_x_kernel.size() / 2;

Image::ColorImage apply_adaptive_blur(const Image::ColorImage&, float, int nr_iterations = 1);
Image::GradientImage compute_gradient(const Image::ColorImage&);
Image::GreyscaleImage thin_edges(const Image::GradientImage&);
float compute_threshold(const Image::GreyscaleImage&);
Image::BinaryImage
apply_hysteresis(const Image::GreyscaleImage&, float, float, float take_percentile = 0.25f);

Image::BinaryImage detect_edges(const Image::ColorImage&, float threshold = -1.0f);

}  // namespace Canny