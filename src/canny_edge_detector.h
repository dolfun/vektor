#pragma once
#include "image.h"
#include "kernel.h"

namespace Canny {

// Scharr 5x5
// https://www.hlevkin.com/hlevkin/47articles/SobelScharrGradients5x5.pdf
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

static_assert(gradient_x_kernel.size() == gradient_y_kernel.size());

constexpr int padding_requirement = gradient_x_kernel.size() / 2;
Image::GreyscaleImage detect_edges(const Image::ColorImage&, float threshold = -1.0f);

} // namespace Canny