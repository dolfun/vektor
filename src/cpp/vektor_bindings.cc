#include <emscripten/bind.h>

#include "pipeline.h"
#include "vektor/image.h"

using namespace emscripten;
using namespace Vektor;

struct ImageView {
  ImageView() = default;

  template <typename T>
  ImageView(std::string_view name, const ImageWithBytes<T>& image) : name { name } {
    if (image.width() == 0 || image.height() == 0) {
      width = height = 0;
      data = val::null();
    } else {
      width = image.width(), height = image.height();
      const auto& bytes = image.bytes();
      data =
        val { typed_memory_view(bytes.size(), reinterpret_cast<const uint8_t*>(bytes.data())) };
    }
  }

  std::string name;
  int width, height;
  val data;
};

using Curve = BezierCurveWithColor;
template <glm::dvec2 BezierCurve::* Member>
auto make_setter() {
  return +[](const Curve& x) { return x.curve.*Member; };
}

template <glm::dvec2 BezierCurve::* Member>
auto make_getter() {
  return +[](Curve& x, glm::dvec2 v) { x.curve.*Member = v; };
}

void set_pipeline_source_image(Pipeline& pipeline, val image_data) {
  auto data = image_data["data"];
  auto length = data["length"].as<std::size_t>();

  std::vector<std::byte> buffer(length);
  val view { typed_memory_view(length, reinterpret_cast<uint8_t*>(buffer.data())) };
  view.call<void>("set", data);

  auto width = image_data["width"].as<int>();
  auto height = image_data["height"].as<int>();

  auto it = buffer.begin();
  Image::RGBAImage image(width, height);
  Image::apply(width, height, [&](int x, int y) {
    auto color = glm::vec4(it[0], it[1], it[2], it[3]) / 255.0f;
    image[x, y] = color;
    it += 4;
  });

  pipeline.set_source_image(std::move(image));
}

val get_pipeline_image_views(const Pipeline& pipeline) {
  if (pipeline.source_image().empty()) {
    return val::array();
  }

  // clang-format off
  std::vector<ImageView> image_views { 
    { "Source Image", pipeline.source_image() },
    { "Blurred Image", pipeline.blurred_image() },
    { "Gradient Image", pipeline.gradient_image() },
    { "Thinned Image", pipeline.thinned_image() },
    { "Hysteresis Image", pipeline.hysteresis_image() },
    { "Greyscale Plot", pipeline.greyscale_plot() },
    { "Color Plot", pipeline.color_plot() }
  };
  // clang-format on

  return val::array(image_views);
}

val get_pipeline_curves(const Pipeline& pipeline) {
  return val::array(pipeline.curves());
}

EMSCRIPTEN_BINDINGS(my_module) {
  enum_<PipelineConfig::BackgroundColor>("BackgroundColor")
    .value("black", PipelineConfig::BackgroundColor::black)
    .value("white", PipelineConfig::BackgroundColor::white);

  enum_<PipelineConfig::DesmosColor>("DesmosColor")
    .value("solid", PipelineConfig::DesmosColor::solid)
    .value("colorful", PipelineConfig::DesmosColor::colorful);

  value_object<PipelineConfig>("PipelineConfig")
    .field("kernelSize", &PipelineConfig::kernel_size)
    .field("nrIterations", &PipelineConfig::nr_iterations)
    .field("takePercentile", &PipelineConfig::take_percentile)
    .field("plotScale", &PipelineConfig::plot_scale)
    .field("backgroundColor", &PipelineConfig::background_color)
    .field("desmosColor", &PipelineConfig::desmos_color);

  static constexpr auto default_config = PipelineConfig::Default();
  constant("defaultPipelineConfig", default_config);

  value_object<glm::dvec2>("Vec2f").field("x", &glm::dvec2::x).field("y", &glm::dvec2::y);

  value_object<glm::vec3>("Vec3f")
    .field("r", &glm::vec3::r)
    .field("g", &glm::vec3::g)
    .field("b", &glm::vec3::b);

  value_object<Curve>("BezierCurve")
    .field("p0", make_setter<&BezierCurve::p0>(), make_getter<&BezierCurve::p0>())
    .field("p1", make_setter<&BezierCurve::p1>(), make_getter<&BezierCurve::p1>())
    .field("p2", make_setter<&BezierCurve::p2>(), make_getter<&BezierCurve::p2>())
    .field("p3", make_setter<&BezierCurve::p3>(), make_getter<&BezierCurve::p3>())
    .field("color", &Curve::color);

  value_object<ImageView>("ImageView")
    .field("name", &ImageView::name)
    .field("width", &ImageView::width)
    .field("height", &ImageView::height)
    .field("data", &ImageView::data);

  class_<Pipeline>("Pipeline")
    .constructor()
    .function("setSourceImage", &set_pipeline_source_image)
    .function("setConfig", &Pipeline::set_config)
    .property("config", &Pipeline::config)
    .property("imageViews", &get_pipeline_image_views, return_value_policy::take_ownership())
    .property("curves", &get_pipeline_curves, return_value_policy::take_ownership());
}