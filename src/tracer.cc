#include "tracer.h"
#include <array>
#include <queue>
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

auto compute_pivot(const Path& path) -> std::vector<int> {
  int n = path.size();
  std::vector<int> next_corner(n);  
  for (int i = n - 1, k = n - 1; i >= 0; --i) {
    if (path[i].x != path[k].x && path[i].y != path[k].y) {
      k = i + 1;
    }
    next_corner[i] = k;
  }

  auto cross = [] (const glm::ivec2& a, const glm::ivec2& b) {
    return a.x * b.y - a.y * b.x;
  };

  auto floor_div = [] (int a, int b) {
    return a >= 0 ? a / b : -1 - (-1 - a) / b;
  };

  std::vector<int> pivot(n);
  pivot[n - 1] = n - 1;
  for (int i = n - 2; i >= 0; --i) {
    std::array<int, 4> dir_count{};
    glm::ivec2 dir = path[i + 1] - path[i];
    int dir_index = (3 + 3 * dir.x + dir.y) / 2;
    ++dir_count[dir_index];

    glm::ivec2 constraint0{}, constraint1{};
    int k = next_corner[i];
    int k_prev = i;
    while (true) {
      glm::ivec2 dir = glm::sign(path[k] - path[k_prev]);
      int dir_index = (3 + 3 * dir.x + dir.y) / 2;
      ++dir_count[dir_index];

      if (dir_count[0] && dir_count[1] && dir_count[2] && dir_count[3]) {
        pivot[i] = k_prev;
        break;
      }

      glm::ivec2 curr = path[k] - path[i];
      if (cross(constraint0, curr) < 0 || cross(constraint1, curr) > 0) {
        glm::ivec2 dk = glm::sign(path[k] - path[k_prev]);
        glm::ivec2 curr = path[k_prev] - path[i];

        int a = cross(constraint0, curr);
        int b = cross(constraint0, dk);
        int c = cross(constraint1, curr);
        int d = cross(constraint1, dk);

        int j = INT_MAX;
        if (b < 0) {
          j = floor_div(a, -b);
        }
        if (d > 0) {
          j = glm::min(j, floor_div(-c, d));
        }

        pivot[i] = glm::min(k_prev + j, n - 1);
        break;
      }

      if (glm::abs(curr.x) > 1 || glm::abs(curr.y) > 1) {
        glm::ivec2 offset;
        offset.x = curr.x + ((curr.y >= 0 && (curr.y > 0 || curr.x < 0)) ? 1 : -1);
        offset.y = curr.y + ((curr.x <= 0 && (curr.x < 0 || curr.y < 0)) ? 1 : -1);
        if (cross(constraint0, offset) >= 0) {
          constraint0 = offset;
        }

        offset.x = curr.x + ((curr.y <= 0 && (curr.y < 0 || curr.x < 0)) ? 1 : -1);
        offset.y = curr.y + ((curr.x >= 0 && (curr.x > 0 || curr.y < 0)) ? 1 : -1);
        if (cross(constraint1, offset) <= 0) {
          constraint1 = offset;
        }
      }

      k_prev = k;
      k = next_corner[k];
      if (k_prev == n - 1) {
        pivot[i] = n - 1;
        break;
      }
    }
  }

  for (auto& x : pivot) {
    x = glm::min(x, n - 1);
  }

  return pivot;
}

struct PrefixSum {
  std::int64_t x, y, x2, y2, xy;
};

using PrefixSums_t = std::vector<PrefixSum>;
auto compute_prefix_sums(const Path& path) -> PrefixSums_t {
  int n = path.size() + 1;
  PrefixSums_t prefix_sums(n);

  for (int i = 0; i < n - 1; ++i) {
    auto p = path[i];
    prefix_sums[i + 1].x = prefix_sums[i].x + p.x;
    prefix_sums[i + 1].y = prefix_sums[i].y + p.y;
    prefix_sums[i + 1].x2 = prefix_sums[i].x2 + p.x * p.x;
    prefix_sums[i + 1].y2 = prefix_sums[i].y2 + p.y * p.y;
    prefix_sums[i + 1].xy = prefix_sums[i].xy + p.x * p.y;
  }

  return prefix_sums;
}

float compute_penalty(const Path& path, const PrefixSums_t& prefix_sums, int i, int j) {
  if (i > j) std::swap(i, j);

  float x  = static_cast<float>(prefix_sums[j + 1].x  - prefix_sums[i].x );
  float y  = static_cast<float>(prefix_sums[j + 1].y  - prefix_sums[i].y );
  float x2 = static_cast<float>(prefix_sums[j + 1].x2 - prefix_sums[i].x2);
  float y2 = static_cast<float>(prefix_sums[j + 1].y2 - prefix_sums[i].y2);
  float xy = static_cast<float>(prefix_sums[j + 1].xy - prefix_sums[i].xy);
  float k = static_cast<float>(j - i + 1);

  glm::vec2 p = glm::vec2(path[i] + path[j]) / 2.0f;
  float ey = static_cast<float>(path[j].x - path[i].x);
  float ex = -static_cast<float>(path[j].y - path[i].y);

  float a = (x2 - 2.0f*x*p.x) / k + p.x*p.x;
  float b = (xy - x*p.y - y*p.x) / k + p.x*p.y;
  float c = (y2 - 2.0f*y*p.y) / k + p.y*p.y;

  float s = ex*ex*a + 2.0f*ex*ey*b + ey*ey*c;
  return glm::sqrt(s);
}

auto compute_optimal_sequence(const Path& path) -> std::vector<int> {
  auto pivot = compute_pivot(path);
  auto prefix_sums = compute_prefix_sums(path);

  int n = path.size();
  std::vector<int> clip0(n);
  clip0[0] = pivot[0] - 1;
  clip0[n - 1] = n - 1;
  for (int i = 1; i < n - 1; ++i) {
    int c = pivot[i - 1] - 1;
    if (c == n - 2) c = n - 1;
    clip0[i] = c;
  }

  std::vector<int> clip1(n);
  for (int i = 1, j = 0; i < n - 1; ++i) {
    while (j < clip0[i]) {
      clip1[j + 1] = (i == 1) ? 0 : i;
      ++j;
    }
  }
  clip1[n - 1] = clip1[n - 2];

  using Cost_t = std::pair<int, float>;
  std::vector<std::vector<std::pair<int, Cost_t>>> graph(n);
  for (int i = 0; i < n; ++i) {
    for (int j = clip1[i]; j <= clip0[i]; ++j) {
      if (j == i) continue;
      float penalty = compute_penalty(path, prefix_sums, i, j);
      graph[i].emplace_back(j, Cost_t { 1, penalty });
      graph[j].emplace_back(i, Cost_t { 1, penalty });
    }
  }

  std::vector<unsigned char> vis(n);
  std::vector<Cost_t> dist(n, { 1e7, 0.0f });
  std::vector<int> prev(n);

  using Queue_t = std::pair<Cost_t, int>;
  std::priority_queue<Queue_t, std::vector<Queue_t>, std::greater<>> pq;
  pq.emplace(Cost_t { 0, 0.0f }, 0);
  dist[0] = { 0, 0.0f };
  while (!pq.empty()) {
    auto p = pq.top();
    pq.pop();

    auto x = p.second;
    if (vis[x]) continue;
    vis[x] = true;

    for (auto v : graph[x]) {
      int e = v.first;
      auto c1 = v.second;
      Cost_t c2 = { dist[x].first + c1.first, dist[x].second + c1.second };

      if (c2 < dist[e]) {
        prev[e] = x;
        dist[e] = c2;
        pq.emplace(dist[e], e);
      }
    }
  }

  std::vector<int> result;
  for (int i = n - 1; i > 0;) {
    result.push_back(i);
    i = prev[i];
  }
  result.push_back(0);
  rng::reverse(result);

  return result;
}

void draw_line(glm::vec2 p1, glm::vec2 p2, auto&& f) {
  auto i_part = [] (float x) {
    return glm::floor(x);
  };

  auto round = [&] (float x) {
    return i_part(x + 0.5f);
  };

  auto f_part = [] (float x) {
    return x - glm::floor(x);
  };

  auto rf_part = [&] (float x) {
    return 1.0f - f_part(x);
  };
  
  float x0 = p1.x, y0 = p1.y, x1 = p2.x, y1 = p2.y;
  bool steep = glm::abs(y1 - y0) > glm::abs(x1 - x0);
  if (steep) {
    std::swap(x0, y0);
    std::swap(x1, y1);
  }
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
  }

  float dx = x1 - x0, dy = y1 - y0;
  float gradient = dy / dx;
  if (dx == 0.0f) gradient = 1.0f;

  float xend = round(x0);
  float yend = y0 + gradient * (xend - x0);
  float xgap = rf_part(x0 + 0.5f);
  float xpxl1 = xend;
  float ypxl1 = i_part(yend);
  if (steep) {
    std::forward<decltype(f)>(f)(ypxl1       , xpxl1, rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(ypxl1 + 1.0f, xpxl1,  f_part(yend) * xgap);
  } else {
    std::forward<decltype(f)>(f)(xpxl1, ypxl1       , rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(xpxl1, ypxl1 + 1.0f,  f_part(yend) * xgap);
  }

  float intery = yend + gradient;
  xend = round(x1);
  yend = y1 + gradient * (xend - x1);
  xgap = f_part(x1 + 0.5f);
  float xpxl2 = xend;
  float ypxl2 = i_part(yend);
  if (steep) {
    std::forward<decltype(f)>(f)(ypxl2    , xpxl2, rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(ypxl2 + 1, xpxl2,  f_part(yend) * xgap);
  } else {
    std::forward<decltype(f)>(f)(xpxl2, ypxl2    , rf_part(yend) * xgap);
    std::forward<decltype(f)>(f)(xpxl2, ypxl2 + 1,  f_part(yend) * xgap);
  }

  if (steep) {
    for (float x = xpxl1 + 1.0f; x <= xpxl2 - 1.0f; x += 1.0f) {
      std::forward<decltype(f)>(f)(i_part(intery)       , x, rf_part(intery));
      std::forward<decltype(f)>(f)(i_part(intery) + 1.0f, x,  f_part(intery));
      intery += gradient;
    }
  } else {
    for (float x = xpxl1 + 1.0f; x <= xpxl2 - 1.0f; x += 1.0f) {
      std::forward<decltype(f)>(f)(x, i_part(intery)       , rf_part(intery));
      std::forward<decltype(f)>(f)(x, i_part(intery) + 1.0f,  f_part(intery));
      intery += gradient;
    }
  }
}

namespace Tracer {

ColorImage trace(const GreyscaleImage& image) {
  auto fixed_image = fix_image(image);

  PathFinder path_finder { fixed_image };
  auto paths = path_finder.result();

  int width = image.width();
  int height = image.height();
  ColorImage result { width, height };

  for (const auto& path : paths) {
    auto sequence = compute_optimal_sequence(path);

    int n = sequence.size();
    for (int i = 1; i < n; ++i) {
      glm::vec2 p1 = path[sequence[i - 1]];
      glm::vec2 p2 = path[sequence[i]];

      draw_line(p1, p2, [&] (float x, float y, float c) {
        result(x, y) = glm::vec3(c);
      });
    }
  }

  return result;
}

} // namespace Tracer