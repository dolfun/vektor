#include "image.h"
#include "canny_edge_detector.h"

int main() {
  Image::Image image = Image::load("building.png", Canny::padding_requirement);
  image = Canny::detect_edges(image);
  Image::save_as_png(image, "output.png");

  return 0;
}