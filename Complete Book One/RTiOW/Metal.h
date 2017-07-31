#pragma once
#include "Material.h"
#include "Hitable.h"
#include "Random.h"

class Metal : public Material
{
public:
    Metal(const Vector3& a, float f) :
        albedo(a)
    {
        fuzz = (f < 1) ? f : 1;
    }

    bool Scatter(const Ray& in, const HitRecord& record, Vector3& attenuation, Ray& scattered) const
    {
        Vector3 reflected = reflect(unitVector(in.direction), record.normal);
        scattered = Ray(record.p, reflected + fuzz * RandomInUnitSphere());
        attenuation = albedo;

        return dot(scattered.direction, record.normal) > 0;
    }

    Vector3 albedo;
    float fuzz;
};