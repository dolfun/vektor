#pragma once
#include "kernel.h"

namespace Canny {

static constexpr Image::Kernel<5> gaussian_kernel {
  {
    2, 4, 5, 4, 2,
    4, 9, 12, 9, 4,
    5, 12, 15, 12, 5,
    4, 9, 12, 9, 4,
    2, 4, 5, 4, 2
  },
  159.0f
};

constexpr int padding_requirement = gaussian_kernel.size() / 2;
Image::Image detect_edges(const Image::Image&);

} // namespace Canny