#include "pipeline.h"

#include <ranges>

#include "vektor/canny_edge_detector.h"
#include "vektor/renderer.h"
#include "vektor/tracer.h"

namespace Vektor {

bool BlurStage::update(
  const RawRGBImage& source_image,
  const PipelineConfig& config,
  bool to_update
) {
  if (to_update || config.kernel_size != m_kernel_size || config.nr_iterations != m_nr_iterations) {
    m_kernel_size = config.kernel_size;
    m_nr_iterations = config.nr_iterations;
    constexpr float h = 1.0f;
    result = Canny::apply_adaptive_blur(source_image.image(), h, m_kernel_size, m_nr_iterations);
    return true;
  }
  return false;
}

bool GradientStage::update(const RawRGBImage& blurred_image, bool to_update) {
  if (to_update) {
    result = Canny::compute_gradient(blurred_image.image());
    return true;
  }
  return false;
}

bool ThinningStage::update(const RawGradientImage& gradient_image, bool to_update) {
  if (to_update) {
    result = Canny::thin_edges(gradient_image.image());
    return true;
  }
  return false;
}

bool ThresholdStage::update(const RawGreyscaleImage& thinned_image, bool to_update) {
  if (to_update) {
    std::tie(tl, th) = Canny::compute_threshold(thinned_image.image());
    return true;
  }
  return false;
}

bool HysteresisStage::update(
  const RawGreyscaleImage& thinned_image,
  float tl,
  float th,
  const PipelineConfig& config,
  bool to_update
) {
  if (to_update || config.take_percentile != m_take_percentile) {
    m_take_percentile = config.take_percentile;
    result = Canny::apply_hysteresis(thinned_image.image(), tl, th, m_take_percentile);
    return true;
  }
  return false;
}

bool TracingStage::update(
  const RawBinaryImage& hysteresis_image,
  const RawRGBImage& source_image,
  bool to_update
) {
  if (to_update) {
    curves = Tracer::trace(hysteresis_image.image()) |
             std::ranges::to<std::vector<BezierCurveWithColor>>();

    for (auto& [curve, color] : curves) {
      color = Renderer::compute_curve_color(curve, source_image.image());
    }

    return true;
  }
  return false;
}

bool PlottingStage::update(
  const std::vector<BezierCurveWithColor>& curves,
  const RawRGBImage& source_image,
  const PipelineConfig& config,
  bool to_update
) {
  if (to_update || config.plot_scale != m_plot_scale ||
      config.background_color != m_background_color) {
    m_plot_scale = config.plot_scale;
    m_background_color = config.background_color;

    auto plot_width = static_cast<float>(source_image.width() * m_plot_scale);
    auto plot_height = static_cast<float>(source_image.height() * m_plot_scale);

    greyscale_plot = Renderer::render_greyscale(
      plot_width,
      plot_height,
      curves,
      m_background_color == PipelineConfig::BackgroundColor::black ? 0.0f : 1.0f
    );

    color_plot = Renderer::render_color(
      plot_width,
      plot_height,
      curves,
      m_background_color == PipelineConfig::BackgroundColor::black ? glm::vec3(0.0f)
                                                                   : glm::vec3(1.0f)
    );

    return true;
  }
  return false;
}

const RawRGBAImage& Pipeline::source_image() const noexcept {
  return m_source_image_rgba;
}

const RawRGBImage& Pipeline::blurred_image() const noexcept {
  return m_blur.result;
}

const RawGradientImage& Pipeline::gradient_image() const noexcept {
  return m_gradient.result;
}

const RawGreyscaleImage& Pipeline::thinned_image() const noexcept {
  return m_thinning.result;
}

const RawBinaryImage& Pipeline::hysteresis_image() const noexcept {
  return m_hysteresis.result;
}

const RawGreyscaleImage& Pipeline::greyscale_plot() const noexcept {
  return m_plotting.greyscale_plot;
}

const RawRGBImage& Pipeline::color_plot() const noexcept {
  return m_plotting.color_plot;
}

const std::vector<BezierCurveWithColor>& Pipeline::curves() const noexcept {
  return m_tracing.curves;
}

void Pipeline::set_config(Pipeline::Config config) {
  m_config = config;
  run_pipeline(false);
}

Pipeline::Config Pipeline::config() const noexcept {
  return m_config;
}

void Pipeline::run_pipeline(bool dirty) {
  if (m_source_image_rgba.width() == 0 || m_source_image_rgba.height() == 0) {
    m_blur.result.clear();
    m_gradient.result.clear();
    m_thinning.result.clear();
    m_threshold.th = m_threshold.tl = 0.0;
    m_hysteresis.result.clear();
    m_tracing.curves.clear();
    m_plotting.color_plot.clear();
    m_plotting.greyscale_plot.clear();

    return;
  }

  dirty = m_blur.update(m_source_image_rgb, m_config, dirty);
  dirty = m_gradient.update(m_blur.result, dirty);
  dirty = m_thinning.update(m_gradient.result, dirty);
  dirty = m_threshold.update(m_thinning.result, dirty);
  dirty = m_hysteresis.update(m_thinning.result, m_threshold.tl, m_threshold.th, m_config, dirty);
  dirty = m_tracing.update(m_hysteresis.result, m_source_image_rgb, dirty);
  m_plotting.update(m_tracing.curves, m_source_image_rgb, m_config, dirty);
}

}  // namespace Vektor