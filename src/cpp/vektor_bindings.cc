#include <emscripten/bind.h>

#include <glm/vec3.hpp>

#include "vektor/canny_edge_detector.h"
#include "vektor/image.h"

using namespace emscripten;

template <typename T>
void create_image_class_bindings(const char* name) {
  using Image = Image::Image<T>;
  class_<Image>(name)
    .constructor<int, int>()
    .property("width", &Image::width)
    .property("height", &Image::height)
    .function(
      "getPixel",
      static_cast<T (*)(const Image&, int, int)>([](const auto& image, int x, int y) {
        return image[x, y];
      })
    )
    .function(
      "setPixel",
      static_cast<void (*)(Image&, int, int, T)>([](auto& image, int x, int y, T value) {
        image[x, y] = value;
      })
    );
}

EMSCRIPTEN_BINDINGS(my_module) {
  value_object<glm::vec3>("Vec3f")
    .field("r", &glm::vec3::r)
    .field("g", &glm::vec3::g)
    .field("b", &glm::vec3::b);

  create_image_class_bindings<glm::vec3>("ColorImage");
  create_image_class_bindings<float>("GreyscaleImage");

  function("detectEdges", &Canny::detect_edges, return_value_policy::take_ownership());
}