#pragma once
#include "Material.h"
#include "Hitable.h"
#include "Random.h"

class Lambertian : public Material
{
public:
    Lambertian(const Vector3& a) :
        albedo(a)
    {}

    bool Scatter(const Ray& in, const HitRecord& record, Vector3& attenuation, Ray& scattered) const
    {
        Vector3 target = record.p + record.normal + RandomInUnitSphere();
        scattered = Ray(record.p, target - record.p);
        attenuation = albedo;

        return true;
    }

    Vector3 albedo;
};