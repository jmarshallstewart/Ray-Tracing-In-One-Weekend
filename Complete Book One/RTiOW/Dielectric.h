#pragma once
#include "Material.h"
#include "Ray.h"
#include "Hitable.h"

float Schlick(float cosine, float ref_idx)
{
    float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
    r0 = r0 * r0;

    return r0 + (1.0f - r0) * pow((1.0f - cosine), 5);
}

class Dielectric : public Material
{
public:
    Dielectric(float ri) :
        ref_idx(ri)
    {}

    bool Scatter(const Ray& in, const HitRecord& record, Vector3& attenuation, Ray& scattered) const
    {
        Vector3 outwardNormal;
        Vector3 reflected = reflect(in.direction, record.normal);
        
        float niOverNt;
        attenuation = Vector3(1.0f, 1.0f, 1.0f);
        Vector3 refracted;
        float reflect_prob;
        float cosine;

        if (dot(in.direction, record.normal) > 0)
        {
            outwardNormal = -record.normal;
            niOverNt = ref_idx;
            cosine = ref_idx * dot(in.direction, record.normal) / in.direction.length();
        }
        else
        {
            outwardNormal = record.normal;
            niOverNt = 1.0f / ref_idx;
            cosine = -dot(in.direction, record.normal) / in.direction.length();
        }

        if (refract(in.direction, outwardNormal, niOverNt, refracted))
        {
            reflect_prob = Schlick(cosine, ref_idx);
        }
        else
        {
            reflect_prob = 1.0f;
        }

        float r = GetUniformRandom();

        if (r < reflect_prob)
        {
            scattered = Ray(record.p, reflected);
        }
        else
        {
            scattered = Ray(record.p, refracted);
        }
    
        return true;
    }

    float ref_idx;
};