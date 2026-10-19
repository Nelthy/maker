#include "collision_basic/vector_poly.h"
vector_poly::vector_poly(const std::vector<vec2f> &base_vertices) {
  vertices = base_vertices;
  vec2f centroid = calculate_centroid();
  for (auto &v : vertices) {
    v -= centroid;
  }
}
float vector_poly::calculate_area() const {
  // Shoelace formula for calculating area
  float area = 0.0;

  for (int i = 0; i < vertices.size(); ++i) {
    int j = (i + 1) % vertices.size();
    area += (vertices[i].x * vertices[j].y - vertices[j].x * vertices[i].y);
  }

  return 0.5f * std::abs(area);
}

float vector_poly::calculate_area_inertia() const {
  // Calculate Inertia
  float In = 0.0;
  for (int i = 0; i < vertices.size(); ++i) {
    int j = (i + 1) % vertices.size();
    float xi = vertices[i].x;
    float yi = vertices[i].y;
    float xj = vertices[j].x;
    float yj = vertices[j].y;

    In += (xi * xi + yi * yi + xi * xj + yi * yj + xj * xj + yj * yj) * (xi * yj - xj * yi);
  }
  return std::abs((In / 12.0f));
}

vec2f vector_poly::calculate_centroid() const {
  float area = 0.0;

  for (int i = 0; i < vertices.size(); ++i) {
    int j = (i + 1) % vertices.size();
    area += (vertices[i].x * vertices[j].y - vertices[j].x * vertices[i].y);
  }
  area *= 0.5;

  vec2f c{};
  for (unsigned i = 0; i < vertices.size(); ++i) {
    int j = (i + 1) % vertices.size();
    c.x += (vertices[i].x + vertices[j].x) * (vertices[i].x * vertices[j].y - vertices[j].x * vertices[i].y);
    c.y += (vertices[i].y + vertices[j].y) * (vertices[i].x * vertices[j].y - vertices[j].x * vertices[i].y);
  }

  return c * (1.f / (6 * area));
}
vector_poly vector_poly::square_shape(float scale_x, float scale_y) {
  return vector_poly({{0, 0}, {0, 50 * scale_y}, {50 * scale_x, 50 * scale_y}, {50 * scale_x, 0}});
};

vector_poly vector_poly::pentagon_shape(float scale_x, float scale_y) {
  return vector_poly({{0.0f, 50.0f * scale_y},
                      {47.6f * scale_x, 15.5f * scale_y},
                      {29.4f * scale_x, -40.5f * scale_y},
                      {-29.4f * scale_x, -40.5f * scale_y},
                      {-47.6f * scale_x, 15.5f * scale_y}});
};

vector_poly vector_poly::hexagon_shape(float scale_x, float scale_y) {
  return vector_poly({
    {50.0f * scale_x, 0.0f},
    {25.0f * scale_x, 43.3f * scale_y},
    {-25.0f * scale_x, 43.3f * scale_y},
    {-50.0f * scale_x, 0.0f},
    {-25.0f * scale_x, -43.3f * scale_y},
    {25.0f * scale_x, -43.3f * scale_y}
  });
}

