#pragma once

#include <concepts>
#include <type_traits>
#include <utility>
#include <vector>

#include "glm/common.hpp"
#include "vektor/bezier_curve.h"
#include "vektor/image.h"

namespace Vektor {

template <typename T>
class ImageWithBytes {
  using Image_t = Image::Image<T>;

public:
  ImageWithBytes() = default;
  ImageWithBytes(const Image_t& image) : m_bytes(image_to_bytes(image)), m_image(image) {}
  ImageWithBytes(Image_t&& image) : m_bytes(image_to_bytes(image)), m_image(std::move(image)) {}

  int width() const noexcept {
    return m_image.width();
  }

  int height() const noexcept {
    return m_image.height();
  }

  bool empty() const noexcept {
    return m_bytes.empty();
  }

  const Image_t& image() const noexcept {
    return m_image;
  }

  const std::vector<std::byte>& bytes() const noexcept {
    return m_bytes;
  }

  void clear() noexcept {
    m_image.clear();
    m_bytes.clear();
  }

private:
  std::vector<std::byte> m_bytes;
  Image_t m_image;

  static auto image_to_bytes(const Image_t& image) {
    const int width = image.width();
    const int height = image.height();
    constexpr int NC = 4;
    constexpr float SCALE_FACTOR = 255.0f;
    constexpr float CLAMP = 255.0f;

    std::vector<std::byte> data(width * height * NC);
    Image::apply(width, height, [&](int x, int y) {
      int index = y * width + x;

      if constexpr (std::same_as<T, float>) {
        auto value = static_cast<std::byte>(glm::clamp(image[x, y] * SCALE_FACTOR, 0.0f, CLAMP));
        data[NC * index] = data[NC * index + 1] = data[NC * index + 2] = value;

      } else if constexpr (std::same_as<T, std::pair<float, float>>) {
        auto value =
          static_cast<std::byte>(glm::clamp(image[x, y].first * SCALE_FACTOR, 0.0f, CLAMP));
        data[NC * index] = data[NC * index + 1] = data[NC * index + 2] = value;

      } else if constexpr (std::same_as<T, char>) {
        auto value = static_cast<std::byte>(image[x, y] ? 255 : 0);
        data[NC * index] = data[NC * index + 1] = data[NC * index + 2] = value;

      } else if constexpr (std::same_as<T, glm::vec3>) {
        glm::vec3 color = glm::clamp(image[x, y] * SCALE_FACTOR, 0.0f, CLAMP);
        data[NC * index + 0] = static_cast<std::byte>(color.r);
        data[NC * index + 1] = static_cast<std::byte>(color.g);
        data[NC * index + 2] = static_cast<std::byte>(color.b);

      } else if constexpr (std::same_as<T, glm::vec4>) {
        glm::vec4 color = glm::clamp(image[x, y] * SCALE_FACTOR, 0.0f, CLAMP);
        data[NC * index + 0] = static_cast<std::byte>(color.r);
        data[NC * index + 1] = static_cast<std::byte>(color.g);
        data[NC * index + 2] = static_cast<std::byte>(color.b);
        data[NC * index + 3] = static_cast<std::byte>(color.a);
      }

      if constexpr (!std::same_as<T, glm::vec4>) {
        data[NC * index + 3] = static_cast<std::byte>(255);
      }
    });

    return data;
  }
};

using RawRGBAImage = ImageWithBytes<glm::vec4>;
using RawRGBImage = ImageWithBytes<glm::vec3>;
using RawGradientImage = ImageWithBytes<std::pair<float, float>>;
using RawGreyscaleImage = ImageWithBytes<float>;
using RawBinaryImage = ImageWithBytes<char>;

struct PipelineConfig {
  enum class BackgroundColor { black, white };
  enum class DesmosColor { solid, colorful };

  int kernel_size;
  int nr_iterations;
  float take_percentile;
  float plot_scale;
  BackgroundColor background_color;
  DesmosColor desmos_color;

  constexpr static PipelineConfig Default() {
    return { .kernel_size = 1,
             .nr_iterations = 1,
             .take_percentile = 0.25f,
             .plot_scale = 1.0f,
             .background_color = BackgroundColor::black,
             .desmos_color = DesmosColor::colorful };
  };
};

class BlurStage {
public:
  RawRGBImage result;

  bool update(const RawRGBImage&, const PipelineConfig&, bool);

private:
  int m_kernel_size = 0;
  int m_nr_iterations = 0;
};

class GradientStage {
public:
  RawGradientImage result;

  bool update(const RawRGBImage&, bool);
};

class ThinningStage {
public:
  RawGreyscaleImage result;

  bool update(const RawGradientImage&, bool);
};

class ThresholdStage {
public:
  float tl = 0.0f;
  float th = 0.0f;

  bool update(const RawGreyscaleImage&, bool);
};

class HysteresisStage {
public:
  RawBinaryImage result;

  bool update(const RawGreyscaleImage&, float, float, const PipelineConfig&, bool);

private:
  float m_take_percentile = -1.0f;
};

class TracingStage {
public:
  std::vector<BezierCurveWithColor> curves;

  bool update(const RawBinaryImage&, const RawRGBImage&, bool);
};

class PlottingStage {
public:
  RawGreyscaleImage greyscale_plot;
  RawRGBImage color_plot;

  bool
  update(const std::vector<BezierCurveWithColor>&, const RawRGBImage&, const PipelineConfig&, bool);

private:
  float m_plot_scale = 0.0f;
  PipelineConfig::BackgroundColor m_background_color = PipelineConfig::BackgroundColor::black;
};

class Pipeline {
public:
  using Config = PipelineConfig;

  const RawRGBAImage& source_image() const noexcept;
  const RawRGBImage& blurred_image() const noexcept;
  const RawGradientImage& gradient_image() const noexcept;
  const RawGreyscaleImage& thinned_image() const noexcept;
  const RawBinaryImage& hysteresis_image() const noexcept;
  const RawGreyscaleImage& greyscale_plot() const noexcept;
  const RawRGBImage& color_plot() const noexcept;
  const std::vector<BezierCurveWithColor>& curves() const noexcept;

  template <typename T>
    requires std::same_as<std::remove_cvref_t<T>, Image::RGBAImage>
  void set_source_image(T&& img) {
    Image::RGBImage rgb_image { img.width(), img.height() };
    Image::apply(img.width(), img.height(), [&](int x, int y) {
      glm::vec4 color = img[x, y];
      rgb_image[x, y] = glm::vec3(color.r, color.g, color.b);
    });

    m_source_image_rgba = std::forward<T>(img);
    m_source_image_rgb = std::move(rgb_image);

    run_pipeline(true);
  }

  void set_config(Config);
  Config config() const noexcept;

private:
  Config m_config = Config::Default();
  RawRGBAImage m_source_image_rgba;
  RawRGBImage m_source_image_rgb;

  BlurStage m_blur;
  GradientStage m_gradient;
  ThinningStage m_thinning;
  ThresholdStage m_threshold;
  HysteresisStage m_hysteresis;
  TracingStage m_tracing;
  PlottingStage m_plotting;

  void run_pipeline(bool);
};

}  // namespace Vektor