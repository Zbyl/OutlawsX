#include "geometry.h"

#include "zerrors.h"

namespace outlaws {

const Vector3 Vector3::zero     = { 0.0f, 0.0f, 0.0f };
const Vector3 Vector3::right    = { 1.0f, 0.0f, 0.0f };
const Vector3 Vector3::up       = { 0.0f, 1.0f, 0.0f };
const Vector3 Vector3::forward  = { 0.0f, 0.0f, 1.0f };

Plane::Plane(Vector3 v0, Vector3 v1, Vector3 v2) {
    auto dir0 = v1 - v0;
    auto dir1 = v2 - v0;
    normal = dir0.cross(dir1);
    normal.normalize();

    auto normalMagnitude = normal.magnitude();
    ZASSERT(normalMagnitude < 2.0f);    // A bit dity check for degenerate cases.

    dist = normal.dot(v0);
}

Plane::Plane(Vertex2 v0, Vertex2 v1)
    : Plane({ v0.x, 0.0f, v0.z }, { v0.x, 1.0f, v0.z }, { v1.x, 0.0f, v1.z })
{
}

} // namespace outlaws
