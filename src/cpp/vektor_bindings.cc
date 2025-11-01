#include <emscripten/bind.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "vektor/bezier_curve.h"
#include "vektor/canny_edge_detector.h"
#include "vektor/renderer.h"
#include "vektor/tracer.h"

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
  value_object<glm::dvec2>("Vec2f").field("x", &glm::dvec2::x).field("y", &glm::dvec2::y);

  value_object<Image::Gradient>("Gradient")
    .field("mag", &Image::Gradient::first)
    .field("angle", &Image::Gradient::second);

  value_object<glm::vec3>("Vec3f")
    .field("r", &glm::vec3::r)
    .field("g", &glm::vec3::g)
    .field("b", &glm::vec3::b);

  value_object<BezierCurve>("BezierCurve")
    .field("p0", &BezierCurve::p0)
    .field("p1", &BezierCurve::p1)
    .field("p2", &BezierCurve::p2)
    .field("p3", &BezierCurve::p3);

  register_vector<BezierCurve>("BezierCurveArray");

  create_image_class_bindings<glm::vec3>("ColorImage");
  create_image_class_bindings<Image::Gradient>("GradientImage");
  create_image_class_bindings<float>("GreyscaleImage");
  create_image_class_bindings<char>("BinaryImage");

  function("quantizeImage", &Canny::quantize_image, return_value_policy::take_ownership());
  function("applyAdaptiveBlur", &Canny::apply_adaptive_blur, return_value_policy::take_ownership());
  function("computeGradient", &Canny::compute_gradient, return_value_policy::take_ownership());
  function("thinEdges", &Canny::thin_edges, return_value_policy::take_ownership());
  function("computeThreshold", &Canny::compute_threshold, return_value_policy::take_ownership());
  function("applyHysteresis", &Canny::apply_hysteresis, return_value_policy::take_ownership());
  function("detectEdges", &Canny::detect_edges, return_value_policy::take_ownership());

  function("traceEdges", &Tracer::trace, return_value_policy::take_ownership());
  function("renderCurves", &Renderer::render_greyscale, return_value_policy::take_ownership());
}