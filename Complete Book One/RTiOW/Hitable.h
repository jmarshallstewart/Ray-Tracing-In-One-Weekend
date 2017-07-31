#pragma once
#include "Ray.h"

class Material;

class HitRecord
{
public:
    float t;
    Vector3 p;
    Vector3 normal;
    Material* material;
};

class Hitable
{
public:
    virtual bool hit(const Ray& r, float t_min, float t_max, HitRecord& record) const = 0;
};