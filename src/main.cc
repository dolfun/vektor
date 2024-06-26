#include <print>
#include "image.h"

int main() {
  Image::Image image = Image::load("building.png");
  Image::save_as_png(image, "output.png");

  return 0;
}