#include <iostream>
#include <map>
#include <string>

#include "vektor/bezier_curve.h"
#include "vektor/canny_edge_detector.h"
#include "vektor/image_io.h"
#include "vektor/renderer.h"
#include "vektor/tracer.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    return -1;
  }

  std::map<std::string, std::string> args;
  args["-o"] = "output.png";
  args["-s"] = "1.0";

  for (int i = 2; i < argc; ++i) {
    if (i + 1 < argc && argv[i][0] == '-') {
      args[std::string { argv[i] }] = std::string { argv[i + 1] };
    } else {
      args[std::string { argv[i] }] = "";
    }
  }

  try {
    std::string path { argv[1] };
    auto source_image = Image::load(path.c_str(), Canny::padding_requirement);
    auto canny_result = Canny::detect_edges(source_image);
    auto curves = Tracer::trace(canny_result);
    std::vector<BezierCurveWithColor> colored_curves(curves.begin(), curves.end());

    int width = source_image.width() * std::stof(args["-s"]);
    int height = source_image.height() * std::stof(args["-s"]);

    std::string output_path = args["-o"];
    if (args.contains("-c")) {
      for (auto& [curve, color] : colored_curves) {
        color = Renderer::compute_curve_color(curve, source_image);
      }

      auto result = Renderer::render_color(width, height, colored_curves);
      Image::save_as_png(result, output_path.c_str());

    } else {
      auto result = Renderer::render_greyscale(width, height, colored_curves);
      Image::save_as_png(result, output_path.c_str());
    }

  } catch (const std::exception& e) {
    std::cerr << "Exception occured: {}" << e.what() << std::endl;
  }

  return 0;
}