#include "Camera.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "Random.h"

Camera::Camera(Vector3 lookFrom, Vector3 lookAt, Vector3 up, float verticalFovInDegrees, float aspect, float aperture, float focusDistance)
{
    lensRadius = aperture / 2.0f;

    float theta = verticalFovInDegrees * (float)M_PI / 180.0f;
    float halfHeight = tan(theta / 2.0f);
    float halfWidth = aspect * halfHeight;
    origin = lookFrom;

    Vector3 w = unitVector(lookFrom - lookAt);
    u = unitVector(cross(up, w));
    v = cross(w, u);

    lowerLeftCorner = origin - halfWidth * focusDistance * u - halfHeight * focusDistance * v - focusDistance * w;
    horizontal = 2 * halfWidth * focusDistance * u;
    vertical = 2 * halfHeight * focusDistance * v;
}

Ray Camera::GetRay(float s, float t) const
{
    Vector3 rd = lensRadius * RandomInUnitDisk();
    Vector3 offset = u * rd.x() + v * rd.y();
    return Ray(origin + offset, lowerLeftCorner + s * horizontal + t * vertical - origin - offset);
}