#pragma once

#include <cmath>
#include <algorithm>

namespace outlaws {

struct Vertex2 {
    float x, z;     ///< x goes to the right, z goes forward.

    Vertex2() = default;
    Vertex2(const Vertex2&) = default;
    Vertex2(float x, float z) : x(x), z(z) {}

    float magnitude() const {
        return sqrtf(x * x + z * z);
    }

    void normalize() {
        *this = normalized();
    }

    [[nodiscard]]
    Vertex2 normalized() const {
        auto mag = magnitude();
        return { x / mag, z / mag };
    }

    Vertex2 perpendicularClockwise() const {
        return { z, -x };
    }

    float dot(Vertex2 other) const {
        return x * other.x + z * other.z;
    }

    Vertex2 operator-() const {
        return { -x, -z };
    }

    Vertex2 operator*(float value) const {
        return { x * value, z * value };
    }

    Vertex2 operator+(Vertex2 other) const {
        return { x + other.x, z + other.z };
    }

    Vertex2 operator-(Vertex2 other) const {
        return { x - other.x, z - other.z };
    }
};

struct Vector3 {
    float x, y, z;

    Vector3() = default;
    Vector3(const Vector3&) = default;
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(Vertex2 v) : x(v.x), y(0.0f), z(v.z) {}

    float magnitude() const {
        return sqrtf(x * x + y * y + z * z);
    }

    void normalize() {
        *this = normalized();
    }

    [[nodiscard]]
    Vector3 normalized() const {
        auto mag = magnitude();
        return { x / mag, y / mag, z / mag };
    }

    /// Cross product.
    /// forward x right == up
    Vector3 cross(Vector3 other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x,
        };
    }

    float dot(Vector3 other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 operator-() const {
        return { -x, -y, -z };
    }

    Vector3 operator*(float value) const {
        return { x * value, y *value, z * value };
    }

    Vector3 operator+(Vector3 other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    Vector3 operator-(Vector3 other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    static const Vector3 zero;      ///< All zeros.
    static const Vector3 right;     ///< x = 1
    static const Vector3 up;        ///< y = 1
    static const Vector3 forward;   ///< z = 1
};

/// Plane as given by equation: ax + by + cz = d.
struct Plane {
    Vector3 normal;     ///< Normalized normal of the plane.
    float dist;         ///< dot(normal, point on the plane)

    Plane() = default;
    Plane(const Plane&) = default;
    Plane(Vector3 normal, float dist) : normal(normal), dist(dist) {}

    /// Computes plane through 3 points.
    /// If points are ordered clockwise, then plane normal points up.
    /// Asserts in degenerate cases.
    Plane(Vector3 v0, Vector3 v1, Vector3 v2);

    /// Returns vertical plane that passes through given vertices.
    /// Normal is pointed to the right of vector v0 -> v1.
    Plane(Vertex2 v0, Vertex2 v1);

    /// Returns signed distance from the plane to given point.
    float signedDistance(Vector3 point) const {
        return normal.dot(point);
    }
};

/// Clamps value to given range.
inline float clamp(float value, float min, float max) {
    return std::max(min, std::min(value, max));
}

/// Returns -1 for negative numbers, 1 for positive numbers, 0 for zero.
inline int sign(float value) {
    if (value < -0.0f)
        return -1;
    if (value > 0.0f)
        return 1;
    return 0;
}

} // namespace outlaws
