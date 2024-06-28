#include <iostream>
#include "tracer.h"
#include "renderer.h"
#include "canny_edge_detector.h"

int main() {

  try {
    auto image = Image::load("NobitaNobi.png", Canny::padding_requirement);
    auto canny_result = Canny::detect_edges(image);
    auto curves = Tracer::trace(canny_result);
    auto result = Renderer::render_color(image.width(), image.height(), curves, image);
    Image::save_as_png(result, "output.png");
    
  } catch (const std::exception& e) {
    std::cerr << "Exception occured: {}" << e.what() << std::endl;
  }

  return 0;
}