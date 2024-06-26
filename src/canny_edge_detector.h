#pragma once
#include "image.h"
#include "kernel.h"

namespace Canny {

constexpr Image::Kernel<5> gaussian_kernel {
  {
    2,  4,  5,  4, 2,
    4,  9, 12,  9, 4,
    5, 12, 15, 12, 5,
    4,  9, 12,  9, 4,
    2,  4,  5,  4, 2,
  },
  159
};

// Sobel 5x5
// https://www.hlevkin.com/hlevkin/47articles/SobelScharrGradients5x5.pdf
constexpr Image::Kernel<5> gradient_x_kernel {
  {
     -5,  -4, 0,  4,  5,
     -8, -10, 0, 10,  8,
    -10, -20, 0, 20, 10,
     -8, -10, 0, 10,  8,
     -5,  -4, 0,  4,  5,
  },
  240
};

constexpr Image::Kernel<5> gradient_y_kernel {
  {
    -5,  -8, -10,  -8, -5,
    -4, -10, -20, -10, -4,
     0,   0,   0,   0,  0,
    +4, +10, +20, +10, +4,
    +5,  +8, +10,  +8, +5
  },
  240
};

static_assert(gradient_x_kernel.size() == gradient_y_kernel.size());

constexpr int padding_requirement = gaussian_kernel.size() / 2;
Image::Image_t detect_edges(const Image::Image_t&, float threshold = -1.0f);

} // namespace Canny