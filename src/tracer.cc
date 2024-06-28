#include "tracer.h"
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

namespace rng = std::ranges;
using Image::GreyscaleImage;
using Image::ColorImage;

constexpr int R = 2;
struct ivec2_hash {
  std::size_t operator() (const glm::ivec2& v) const noexcept {
    return R * (v.y + R) + (v.x + R);
  }
};

using DirMap_t = std::unordered_map<glm::ivec2, std::vector<glm::ivec2>, ivec2_hash>;
DirMap_t dirs_map = [] {
  DirMap_t dirs_map;

  int count = 0;
  std::vector<glm::ivec2> dirs;
  for (int x = -R; x <= R; ++x) {
    for (int y = -R; y <= R; ++y) {
      if (x * x + y * y <= R * R) {
        dirs.emplace_back(x, y);
        ++count;
      }
    }
  }

  dirs_map.max_load_factor(0.25);
  dirs_map.reserve(count);
  for (auto dir : dirs) {
    for (auto v : dirs) {
      if (v == glm::ivec2(0)) continue;
      if (dir.x * v.x + dir.y * v.y >= 0) {
        dirs_map[dir].push_back(v);
      }
    }

    rng::sort(dirs_map[dir], {}, [dir] (const glm::ivec2 v) {
      glm::vec2 _dir { dir }, _v { v };
      float length_dir = glm::length(_dir);
      float length_v = glm::length(_v);
      float angle = glm::dot(_dir, _v) / (length_dir * length_v);
      return std::make_pair(length_v, -angle);
    });
  }

  return dirs_map;
} ();

GreyscaleImage fix_image(const GreyscaleImage& img) {
  auto result = img;

  Image::apply(img.width(), img.height(), [&] (int x, int y) {
    if (img(x + 1, y) && img(x - 1, y)) {
      if (img(x, y + 1) && !img(x, y + 2) && !img(x + 1, y + 1) && !img(x - 1, y + 1)) {
        result(x, y) = 1.0f;
        result(x, y + 1) = 0.0f;
      }

      if (img(x, y - 1) && !img(x, y - 2) && !img(x + 1, y - 1) && !img(x - 1, y - 1)) {
        result(x, y) = 1.0f;
        result(x, y - 1) = 0.0f;
      }

    } else if (img(x, y + 1) && img(x, y - 1)) {
      if (img(x + 1, y) && !img(x + 2, y) && !img(x + 1, y + 1)&& !img(x + 1, y - 1)) {
        result(x, y) = 1.0f;
        result(x + 1, y) = 0.0f;
      }

      if (img(x - 1, y) && !img(x - 2, y) && !img(x - 1, y + 1) && !img(x - 1, y - 1)) {
        result(x, y) = 1.0f;
        result(x - 1, y) = 0.0f;
      }
    }
  });

  return result;
}

using Path = std::vector<glm::ivec2>;
class PathFinder {
public:
  PathFinder(const GreyscaleImage& image)
    : m_image { image }, visited { m_image.width(), m_image.height(), R } {}

  glm::ivec2 search_corner(glm::ivec2 v, glm::ivec2 p = { -1, -1 }) {
    visited(v.x, v.y) = true;

    glm::ivec2 corner = v;
    const auto& dirs = (p.x == -1 ? dirs_map[{0, 0}] : dirs_map[v - p]);
    for (auto dir : dirs) {
      auto u = v + dir;
      if (visited(u.x, u.y) || !m_image(u.x, u.y)) continue;
      corner = search_corner(u, v);
      break;
    }

    visited(v.x, v.y) = false;
    return corner;
  }

  void search_path(Path& path, glm::ivec2 v, glm::ivec2 p = { -1, -1 }) {
    visited(v.x, v.y) = true;

    auto prev = (path.empty() ? v : path.back());
    if (glm::max(glm::abs(v.x - prev.x), glm::abs(v.y - prev.y)) > 1) {
      path.push_back((prev + v) / 2);

    } else if (prev.x != v.x && prev.y != v.y) {
      if (path.size() > 1) {
        glm::ivec2 dir = prev - *(path.end() - 2);

        glm::ivec2 next = prev + 2 * dir;
        if (m_image(next.x, next.y)) {
          path.push_back(v - dir);
        } else {
          path.push_back(prev + dir);
        }

      } else {
        path.push_back({prev.x, v.y});
      }
    }
    path.push_back(v);

    const auto& dirs = (p.x == -1 ? dirs_map[{0, 0}] : dirs_map[v - p]);
    for (auto dir : dirs) {
      auto u = v + dir;
      if (visited(u.x, u.y) || !m_image(u.x, u.y)) continue;
      search_path(path, u, v);
      break;
    }
  }

  auto result() -> std::vector<Path> {
    std::vector<Path> paths;

    Image::apply(m_image.width(), m_image.height(), [&] (int x, int y) {
      while (!visited(x, y)) {
        if (m_image(x, y) < 1.0f) return;

        Path path;
        auto corner = search_corner({x, y});
        search_path(path, corner);

        constexpr int path_size_threshold = 5;
        if (path.size() > path_size_threshold) {
          paths.emplace_back(std::move(path));
        }
      }
    });

    return paths;
  }

private:
  const GreyscaleImage& m_image;
  Image::Image<unsigned char> visited;
};

namespace Tracer {

ColorImage trace(const GreyscaleImage& image) {
  auto fixed_image = fix_image(image);

  PathFinder path_finder { fixed_image };
  auto paths = path_finder.result();

  int width = image.width();
  int height = image.height();
  ColorImage result { width, height };
  for (const auto& path : paths) {
    float i = 1.0;
    for (auto p : path) {
      glm::vec3 color = glm::mix(glm::vec3(0.0, 1.0, 0.0), glm::vec3(1.0, 0.0, 0.0), i / path.size());
      result(p.x, p.y) = color;
      i += 1.0f;
    }
    result(path.front().x, path.front().y) = glm::vec3(1.0);
    result(path.back().x, path.back().y) = glm::vec3(1.0);
  }

  return result;
}

} // namespace Tracer