#include "tracer.h"

#include <algorithm>
#include <array>
#include <glm/glm.hpp>

#include "bezier_curve.h"
#include "image.h"

namespace rng = std::ranges;
using Image::BinaryImage;

class DirsMap {
public:
  static constexpr int R = 2;

  DirsMap() : dirs_map((2 * R + 1) * (2 * R + 1)) {
    std::vector<glm::ivec2> dirs;
    for (int x = -R; x <= R; ++x) {
      for (int y = -R; y <= R; ++y) {
        if (x * x + y * y <= R * R) {
          dirs.emplace_back(x, y);
        }
      }
    }

    for (auto dir : dirs) {
      std::vector<glm::ivec2> aligned_dirs;
      for (auto v : dirs) {
        if (v == glm::ivec2(0)) continue;
        if (dir.x * v.x + dir.y * v.y >= 0) {
          aligned_dirs.push_back(v);
        }
      }

      if (dir != glm::ivec2(0)) {
        rng::sort(aligned_dirs, {}, [dir](const glm::ivec2 v) {
          glm::vec2 _dir { dir }, _v { v };
          float length_dir = glm::length(_dir);
          float length_v = glm::length(_v);

          constexpr float eps = 1e-8;
          float angle = glm::dot(_dir, _v) / (length_dir * length_v + eps);
          return std::make_pair(length_v, -angle);
        });
      }

      auto idx = hash_idx(dir);
      dirs_map[idx] = std::move(aligned_dirs);
    }
  }

  auto operator[](glm::ivec2 v) const noexcept -> const std::vector<glm::ivec2>& {
    auto idx = hash_idx(v);
    return dirs_map[idx];
  }

private:
  std::size_t hash_idx(glm::ivec2 v) const noexcept {
    return (2 * R + 1) * (v.y + R) + (v.x + R);
  }

  std::vector<std::vector<glm::ivec2>> dirs_map;
};

BinaryImage fix_image(const BinaryImage& img) {
  auto result = img;

  Image::apply(img.width(), img.height(), [&](int x, int y) {
    if (img[x + 1, y] && img[x - 1, y]) {
      if (img[x, y + 1] && !img[x, y + 2] && !img[x + 1, y + 1] && !img[x - 1, y + 1]) {
        result[x, y] = 1;
        result[x, y + 1] = 0;
      }

      if (img[x, y - 1] && !img[x, y - 2] && !img[x + 1, y - 1] && !img[x - 1, y - 1]) {
        result[x, y] = 1;
        result[x, y - 1] = 0;
      }

    } else if (img[x, y + 1] && img[x, y - 1]) {
      if (img[x + 1, y] && !img[x + 2, y] && !img[x + 1, y + 1] && !img[x + 1, y - 1]) {
        result[x, y] = 1;
        result[x + 1, y] = 0;
      }

      if (img[x - 1, y] && !img[x - 2, y] && !img[x - 1, y + 1] && !img[x - 1, y - 1]) {
        result[x, y] = 1;
        result[x - 1, y] = 0;
      }
    }
  });

  return result;
}

class PathFinder {
public:
  PathFinder(const BinaryImage& image)
      : m_image { image }, visited { m_image.width(), m_image.height(), DirsMap::R } {
  }

  glm::ivec2 search_corner(glm::ivec2 v, glm::ivec2 p = { -1, -1 }) {
    visited[v.x, v.y] = true;

    glm::ivec2 corner = v;
    const auto& dirs = (p.x == -1 ? dirs_map[{ 0, 0 }] : dirs_map[v - p]);
    for (auto dir : dirs) {
      auto u = v + dir;
      if (visited[u.x, u.y] || !m_image[u.x, u.y]) continue;
      corner = search_corner(u, v);
      break;
    }

    visited[v.x, v.y] = false;
    return corner;
  }

  void search_path(std::vector<glm::ivec2>& path, glm::ivec2 v, glm::ivec2 p = { -1, -1 }) {
    visited[v.x, v.y] = true;

    auto prev = (path.empty() ? v : path.back());
    if (glm::max(glm::abs(v.x - prev.x), glm::abs(v.y - prev.y)) > 1) {
      path.push_back((prev + v) / 2);

    } else if (prev.x != v.x && prev.y != v.y) {
      if (path.size() > 1) {
        glm::ivec2 dir = prev - path.end()[-2];
        glm::ivec2 next = prev + 2 * dir;

        if (m_image[next.x, next.y]) {
          path.push_back(v - dir);
        } else {
          path.push_back(prev + dir);
        }

      } else {
        path.push_back({ prev.x, v.y });
      }
    }
    path.push_back(v);

    const auto& dirs = (p.x == -1 ? dirs_map[{ 0, 0 }] : dirs_map[v - p]);
    for (auto dir : dirs) {
      auto u = v + dir;
      if (visited[u.x, u.y] || !m_image[u.x, u.y]) continue;
      search_path(path, u, v);
      break;
    }
  }

  auto result() {
    std::vector<std::vector<glm::ivec2>> paths;

    Image::apply(m_image.width(), m_image.height(), [&](int x, int y) {
      while (!visited[x, y]) {
        if (!m_image[x, y]) return;

        std::vector<glm::ivec2> path;
        auto corner = search_corner({ x, y });
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
  DirsMap dirs_map;
  const BinaryImage& m_image;
  Image::Image<char> visited;
};

class PathTracer {
public:
  PathTracer(const std::vector<glm::ivec2>& path)
      : n { static_cast<int>(path.size()) }, m_path { path }, m_pivot(n), m_sums(n + 1) {
    compute_pivot();
    compute_prefix_sums();
    compute_optimal_sequence();
    compute_vertices();
    compute_bezier_curves();
  }

  auto bezier_curves() const noexcept -> const std::vector<BezierCurve> {
    return m_curves;
  }

private:
  static constexpr double eps = 1e-8;

  struct Sums {
    double x, y, x2, y2, xy;
  };

  int n, m;
  std::vector<glm::ivec2> m_path;
  std::vector<int> m_pivot;
  std::vector<Sums> m_sums;
  std::vector<int> m_seq;
  std::vector<glm::dvec2> m_vertices;
  std::vector<BezierCurve> m_curves;

  void compute_pivot() noexcept;
  void compute_prefix_sums() noexcept;
  auto compute_sums(int i, int j) const noexcept;
  void compute_optimal_sequence() noexcept;
  void compute_vertices() noexcept;
  void compute_bezier_curves() noexcept;
};

void PathTracer::compute_pivot() noexcept {
  auto cross = [](glm::ivec2 a, glm::ivec2 b) { return a.x * b.y - a.y * b.x; };
  auto floor_div = [](int a, int b) { return a >= 0 ? a / b : -1 - (-1 - a) / b; };

  std::vector<int> next_corner(n);
  for (int i = n - 1, k = n - 1; i >= 0; --i) {
    if (m_path[i].x != m_path[k].x && m_path[i].y != m_path[k].y) {
      k = i + 1;
    }
    next_corner[i] = k;
  }

  for (int i = n - 2; i >= 0; --i) {
    std::array<int, 4> dir_count {};
    glm::ivec2 dir = m_path[i + 1] - m_path[i];
    int dir_index = (3 + 3 * dir.x + dir.y) / 2;
    ++dir_count[dir_index];

    glm::ivec2 constraint0 {}, constraint1 {};
    int k = next_corner[i];
    int k_prev = i;
    while (true) {
      glm::ivec2 dir = glm::sign(m_path[k] - m_path[k_prev]);
      int dir_index = (3 + 3 * dir.x + dir.y) / 2;
      ++dir_count[dir_index];

      if (dir_count[0] && dir_count[1] && dir_count[2] && dir_count[3]) {
        m_pivot[i] = k_prev;
        break;
      }

      glm::ivec2 curr = m_path[k] - m_path[i];
      if (cross(constraint0, curr) < 0 || cross(constraint1, curr) > 0) {
        glm::ivec2 dk = glm::sign(m_path[k] - m_path[k_prev]);
        glm::ivec2 curr = m_path[k_prev] - m_path[i];

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

        m_pivot[i] = k_prev + j;
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
        m_pivot[i] = n - 1;
        break;
      }
    }
  }

  m_pivot[n - 1] = n - 1;
  for (int i = 0; i < n; ++i) {
    m_pivot[i] = glm::min(m_pivot[i], n - 1);
  }

  int j = m_pivot[n - 1];
  for (int i = n - 2; i >= 0; --i) {
    if (m_pivot[i] >= i + 1 && m_pivot[i] < j) {
      j = m_pivot[i];
    }
    m_pivot[i] = j;
  }
}

void PathTracer::compute_prefix_sums() noexcept {
  for (int i = 0; i < n; ++i) {
    glm::dvec2 p = m_path[i] - m_path[0];
    m_sums[i + 1].x = m_sums[i].x + p.x;
    m_sums[i + 1].y = m_sums[i].y + p.y;
    m_sums[i + 1].x2 = m_sums[i].x2 + p.x * p.x;
    m_sums[i + 1].y2 = m_sums[i].y2 + p.y * p.y;
    m_sums[i + 1].xy = m_sums[i].xy + p.x * p.y;
  }
}

auto PathTracer::compute_sums(int i, int j) const noexcept {
  double x = m_sums[j + 1].x - m_sums[i].x;
  double y = m_sums[j + 1].y - m_sums[i].y;
  double x2 = m_sums[j + 1].x2 - m_sums[i].x2;
  double y2 = m_sums[j + 1].y2 - m_sums[i].y2;
  double xy = m_sums[j + 1].xy - m_sums[i].xy;
  auto k = static_cast<double>(j - i + 1);

  return std::make_tuple(x, y, x2, y2, xy, k);
}

void PathTracer::compute_optimal_sequence() noexcept {
  auto compute_penalty = [this](int i, int j) {
    auto [x, y, x2, y2, xy, k] = compute_sums(i, j);
    auto p = glm::dvec2(m_path[i] + m_path[j]) / 2.0 - glm::dvec2(m_path[0]);
    auto ey = static_cast<double>(m_path[j].x - m_path[i].x);
    auto ex = -static_cast<double>(m_path[j].y - m_path[i].y);
    double a = (x2 - 2.0 * x * p.x) / k + p.x * p.x;
    double b = (xy - x * p.y - y * p.x) / k + p.x * p.y;
    double c = (y2 - 2.0 * y * p.y) / k + p.y * p.y;
    double s = ex * ex * a + 2.0 * ex * ey * b + ey * ey * c;
    return glm::sqrt(s);
  };

  std::vector<int> clip0(n);
  clip0[0] = glm::max(1, m_pivot[0] - 1);
  clip0[n - 1] = n - 1;
  for (int i = 1; i < n - 1; ++i) {
    int c = m_pivot[i - 1] - 1;
    if (c == n - 2) c = n - 1;
    clip0[i] = glm::max(i + 1, c);
  }

  using Cost = std::pair<int, double>;
  std::vector<Cost> dist(n, { INT_MAX, 0.0 });
  std::vector<int> prev(n, -1);
  dist[0] = { 0, 0.0 };
  for (int i = 0; i < n; ++i) {
    if (dist[i].first == INT_MAX) continue;
    for (int j = i + 1; j <= clip0[i]; ++j) {
      Cost cand = { dist[i].first + 1, dist[i].second + compute_penalty(i, j) };
      if (cand < dist[j]) {
        dist[j] = cand;
        prev[j] = i;
      }
    }
  }

  for (int i = n - 1; i > 0;) {
    m_seq.push_back(i);
    i = prev[i];
  }
  m_seq.push_back(0);
  rng::reverse(m_seq);
  m = static_cast<int>(m_seq.size());
}

void PathTracer::compute_vertices() noexcept {
  auto compute_best_fit_line = [this](int i, int j) {
    auto [x, y, x2, y2, xy, k] = compute_sums(i, j);

    auto a = (x2 - x * x / k) / k;
    auto b = (xy - x * y / k) / k;
    auto c = (y2 - y * y / k) / k;

    auto lambda2 = (a + c + glm::sqrt((a - c) * (a - c) + 4 * b * b)) / 2;
    a -= lambda2;
    c -= lambda2;

    glm::dvec2 dir {};
    if (glm::abs(a) >= glm::abs(c)) {
      double l = glm::sqrt(a * a + b * b);
      if (l > eps) {
        dir = glm::dvec2(-b, a) / l;
      }
    } else {
      double l = glm::sqrt(c * c + b * b);
      if (l > eps) {
        dir = glm::dvec2(-c, b) / l;
      }
    }
    return std::make_pair(glm::dvec2(x, y) / k, dir);
  };

  auto apply_quadform = [](const glm::dmat3& q, glm::dvec2 p) {
    glm::dvec3 v = { p.x, p.y, 1.0 };
    double sum = 0.0;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        sum += v[i] * q[i][j] * v[j];
      }
    }
    return sum;
  };

  std::vector<glm::dmat3> singular_quadratic_forms(m - 1);
  for (int i = 0; i < m - 1; ++i) {
    auto [center, dir] = compute_best_fit_line(m_seq[i], m_seq[i + 1]);

    glm::dmat3 q {};
    double d = glm::dot(dir, dir);
    if (d < eps) {
      singular_quadratic_forms[i] = q;
      continue;
    }

    glm::dvec3 v;
    v.x = dir.y;
    v.y = -dir.x;
    v.z = -v.y * center.y - v.x * center.x;
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        q[i][j] = v[i] * v[j] / d;
      }
    }
    singular_quadratic_forms[i] = q;
  }

  m_vertices.resize(m);
  m_vertices[0] = static_cast<glm::dvec2>(m_path[m_seq[0]]);
  m_vertices[m - 1] = static_cast<glm::dvec2>(m_path[m_seq[m - 1]]);
  auto origin = static_cast<glm::dvec2>(m_path[m_seq[0]]);
  for (int i = 1; i < m - 1; ++i) {
    auto s = static_cast<glm::dvec2>(m_path[m_seq[i]]) - origin;
    auto q = singular_quadratic_forms[i] + singular_quadratic_forms[i - 1];

    glm::dvec2 w;
    while (true) {
      double det = q[0][0] * q[1][1] - q[0][1] * q[1][0];
      if (glm::abs(det) > eps) {
        w = glm::dvec2(
              -q[0][2] * q[1][1] + q[1][2] * q[0][1],
              q[0][2] * q[1][0] - q[1][2] * q[0][0]
            ) /
            det;
        break;
      }

      glm::dvec3 v;
      if (q[0][0] > q[1][1]) {
        v.x = -q[0][1], v.y = q[0][0];
      } else if (glm::abs(q[1][1]) > eps) {
        v.x = -q[1][1], v.y = q[1][0];
      } else {
        v.x = 1.0, v.y = 0.0;
      }

      double d = v.x * v.x + v.y * v.y;
      v.z = -v.y * s.y - v.x * s.x;
      for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
          q[i][j] += v[i] * v[j] / d;
        }
      }
    }

    auto del = glm::abs(w - s);
    if (del.x <= 0.5 && del.y <= 0.5) {
      m_vertices[i] = origin + w;
      continue;
    }

    double min_val = apply_quadform(q, s);
    glm::dvec2 min_vec = s;

    if (glm::abs(q[0][0]) > eps) {
      for (int z = 0; z < 2; ++z) {
        glm::dvec2 w;
        w.y = s.y - 0.5 + z;
        w.x = -(q[0][1] * w.y + q[0][2]) / q[0][0];
        double candidate = apply_quadform(q, w);
        if (glm::abs(w.x - s.x) <= 0.5 && candidate < min_val) {
          min_val = candidate;
          min_vec = w;
        }
      }
    }

    if (glm::abs(q[1][1]) > eps) {
      for (int z = 0; z < 2; ++z) {
        glm::dvec2 w;
        w.x = s.x - 0.5 + z;
        w.y = -(q[1][0] * w.x + q[1][2]) / q[1][1];
        double candidate = apply_quadform(q, w);
        if (glm::abs(w.y - s.y) <= 0.5 && candidate < min_val) {
          min_val = candidate;
          min_vec = w;
        }
      }
    }

    for (int i = 0; i < 2; ++i) {
      for (int j = 0; j < 2; ++j) {
        glm::dvec2 w = { s.x - 0.5 + i, s.y - 0.5 + j };
        double candidate = apply_quadform(q, w);
        if (candidate < min_val) {
          min_val = candidate;
          min_vec = w;
        }
      }
    }

    m_vertices[i] = origin + min_vec;
  }
}

void PathTracer::compute_bezier_curves() noexcept {
  auto straight_line_to_bezier = [](glm::dvec2 p1, glm::dvec2 p2) {
    auto m = (p1 + p2) / 2.0;
    BezierCurve curve { .p0 = p1, .p1 = m, .p2 = m, .p3 = p2 };
    return curve;
  };

  auto denom = [](glm::dvec2 p0, glm::dvec2 p2) {
    glm::dvec2 r { -glm::sign(p2.y - p0.y), glm::sign(p2.x - p0.x) };
    return r.y * (p2.x - p0.x) - r.x * (p2.y - p0.y);
  };

  auto area_parallelogram = [](glm::dvec2 p0, glm::dvec2 p1, glm::dvec2 p2) {
    auto u1 = p1 - p0, u2 = p2 - p0;
    return u1.x * u2.y - u2.x * u1.y;
  };

  if (m < 2) return;
  if (m == 2) {
    m_curves.push_back(straight_line_to_bezier(m_vertices[0], m_vertices[1]));
    return;
  }

  for (int i = 0; i <= m - 3; ++i) {
    int j = i + 1;
    int k = i + 2;
    auto p0 = glm::dvec2(m_vertices[i] + m_vertices[j]) / 2.0;
    auto p3 = glm::dvec2(m_vertices[k] + m_vertices[j]) / 2.0;

    if (i == 0) p0 = m_vertices[0];
    if (i == m - 3) p3 = m_vertices[m - 1];

    double den = denom(m_vertices[i], m_vertices[k]);
    double alpha = 4.0 / 3.0;
    if (den > eps) {
      double dd = glm::abs(area_parallelogram(m_vertices[i], m_vertices[j], m_vertices[k]) / den);
      alpha = dd > 1.0 ? (1.0 - 1.0 / dd) : 0.0;
      alpha /= 0.75;
    }

    if (alpha >= 1.0) {
      m_curves.emplace_back(straight_line_to_bezier(p0, m_vertices[j]));
      m_curves.emplace_back(straight_line_to_bezier(m_vertices[j], p3));

    } else {
      double a0 = 4.0 * (glm::sqrt(2.0) - 1) / 3.0;
      alpha = glm::clamp(alpha, a0, 1.0);
      double t = 0.5 + alpha * 0.5;
      auto p1 = m_vertices[i] + t * (m_vertices[j] - m_vertices[i]);
      auto p2 = m_vertices[k] + t * (m_vertices[j] - m_vertices[k]);
      m_curves.emplace_back(p0, p1, p2, p3);
    }
  }
}

namespace Tracer {

auto trace(const BinaryImage& image) -> std::vector<BezierCurve> {
  auto fixed_image = fix_image(image);

  PathFinder path_finder { fixed_image };
  auto paths = path_finder.result();

  std::vector<BezierCurve> curves;
  for (const auto& path : paths) {
    PathTracer tracer { path };
    curves.append_range(tracer.bezier_curves());
  }

  double scale = 1.0 / image.width();
  for (auto& curve : curves) {
    curve.apply_scale(scale);
  }

  return curves;
}

}  // namespace Tracer