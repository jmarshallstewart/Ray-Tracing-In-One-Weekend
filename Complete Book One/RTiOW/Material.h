#pragma once

class Ray;
class Vector3;
class HitRecord;

class Material
{
public:
    virtual bool Scatter(const Ray& in, const HitRecord& record, Vector3& attenuation, Ray& scattered) const = 0;
};
