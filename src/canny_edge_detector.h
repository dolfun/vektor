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
  159.0f
};

constexpr Image::Kernel<3> sobel_kernel_x {
  +1, 0, -1,
  +2, 0, -2,
  +1, 0, -1,
};

constexpr Image::Kernel<3> sobel_kernel_y {
  +1, +2, +1,
   0,  0,  0,
  -1, -2, -1,
};

constexpr Image::Kernel<3> gradient_x_kernel = sobel_kernel_x;
constexpr Image::Kernel<3> gradient_y_kernel = sobel_kernel_y;
static_assert(gradient_x_kernel.size() == gradient_y_kernel.size());

constexpr int padding_requirement = gaussian_kernel.size() / 2;
Image::Image_t detect_edges(const Image::Image_t&);

} // namespace Canny