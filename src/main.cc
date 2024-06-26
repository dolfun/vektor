#include "image.h"
#include "canny_edge_detector.h"

int main() {
  auto image = Image::load("NobitaNobi.png", Canny::padding_requirement);
  auto result = Canny::detect_edges(image);
  Image::save_as_png(result, "output.png");

  return 0;
}