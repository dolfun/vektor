#include <print>
#include "tracer.h"
#include "canny_edge_detector.h"

int main() {

  try {
    auto image = Image::load("NobitaNobi.png", Canny::padding_requirement);
    auto canny_result = Canny::detect_edges(image);
    auto tracer_result = Tracer::trace(canny_result);
    Image::save_as_png(tracer_result, "output.png");
    
  } catch (const std::exception& e) {
    std::println("Exception occured: {}", e.what());
  }

  return 0;
}