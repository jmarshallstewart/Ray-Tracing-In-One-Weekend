#pragma once
#include "Vector3.h"

class Ray
{
public:
    Ray() {}
    Ray(const Vector3& o, const Vector3& d) :
        origin(o),
        direction(d)
    {}

    Vector3 pointAtParameter(float t) const
    {
        return origin + t * direction;
    }

    Vector3 origin;
    Vector3 direction;
};