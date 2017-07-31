#pragma once
#include "Ray.h"

class Camera
{
public:
    Camera(Vector3 lookFrom, Vector3 lookAt, Vector3 up, float verticalFovInDegrees, float aspect, float aperture, float focusDistance);
        
    Ray GetRay(float s, float t) const;

    Vector3 lowerLeftCorner;
    Vector3 horizontal;
    Vector3 vertical;
    Vector3 origin;
    Vector3 u;
    Vector3 v;
    float lensRadius;
};