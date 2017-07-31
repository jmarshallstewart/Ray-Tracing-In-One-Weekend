#pragma once
#include "Hitable.h"

class Sphere : public Hitable
{
public:
    Sphere() {}
    Sphere(Vector3 c, float r, Material* mat) :
        center(c),
        radius(r),
        material(mat)
    {}

    bool hit(const Ray& r, float t_min, float t_max, HitRecord& record) const;

    Vector3 center;
    float radius;
    Material* material;
};