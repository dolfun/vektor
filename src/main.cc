#include <map>
#include <string>
#include <iostream>
#include "tracer.h"
#include "image_io.h"
#include "renderer.h"
#include "canny_edge_detector.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    return -1;
  }

  std::map<std::string, std::string> args;
  args["-o"] = "output.png";
  args["-s"] = "1.0";

  for (int i = 2; i < argc; ++i) {
    if (i + 1 < argc && argv[i][0] == '-') {
      args[std::string{ argv[i] }]  = std::string { argv[i + 1] };
    } else {
      args[std::string{ argv[i] }] = "";
    }
  }

  try {
    std::string path { argv[1] };
    auto image = Image::load(path.c_str(), Canny::padding_requirement);
    auto canny_result = Canny::detect_edges(image);
    auto curves = Tracer::trace(canny_result);

    int width = image.width() * std::stof(args["-s"]);
    int height = image.height() * std::stof(args["-s"]);

    std::string output_path = args["-o"];
    if (args.contains("-c")) {
      auto result = Renderer::render_color(width, height, curves, image);
      Image::save_as_png(result, output_path.c_str());

    } else {
      auto result = Renderer::render_greyscale(width, height, curves);
      Image::save_as_png(result, output_path.c_str());
    }
    
  } catch (const std::exception& e) {
    std::cerr << "Exception occured: {}" << e.what() << std::endl;
  }

  return 0;
}