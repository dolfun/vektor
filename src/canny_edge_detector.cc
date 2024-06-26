#include "canny_edge_detector.h"
#include <cassert>
using Image_t = Image::Image;

namespace Canny {

Image_t apply_gaussian_blur(const Image_t& image) {
  assert(image.padding() == gaussian_kernel.size() / 2);

  int width = image.width();
  int height = image.height();  
  Image_t result { width, height, padding_requirement };

  Image::apply(width, height, [&] (int i, int j) {
    result(i, j) = Image::evaluate_kernel(gaussian_kernel, image, i, j);
  });

  return result;
}

Image_t detect_edges(const Image_t& source_image) {
  Image_t result = apply_gaussian_blur(source_image);
  return result;
}

} // namespace Canny