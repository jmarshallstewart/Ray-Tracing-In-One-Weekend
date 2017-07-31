#include "Sphere.h"

/*
formula for sphere at the origin:
x*x + y*y + z*z == R*R

formula for sphere at any position:
(x-cx)*(x-cx) + (y-cy)*(y-cy) + (z-cz)*(z-cz) == R*R

in vector form (p = point ray hits the sphere, c = sphere center):
dot((p-c),(p-c)) == R*R

adding formula for our ray (p(t) = point on ray at time t:
dot((p(t)-c),(p(t)-c)) == R*R

full formula for using ray (ray formula is A + t*B):
dot((A + t*B -c),(A + t*B -c)) == R*R

FOIL, etc.:
t*t*dot(B,B) + 2*t*dot(B, A-C) + dot(A-C, A-C) - R*R == 0

A, B, and C are constants, so we solve the quadratic in the function below.

*/
bool Sphere::hit(const Ray& r, float t_min, float t_max, HitRecord& record) const
{
    Vector3 oc = r.origin - center;
    float a = dot(r.direction, r.direction);
    float b = dot(oc, r.direction);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b*b - a * c;

    if (discriminant > 0)
    {
        float temp = (-b - sqrt(b*b - a*c)) / a;

        if (temp < t_max && temp > t_min)
        {
            record.t = temp;
            record.p = r.pointAtParameter(temp);
            record.normal = (record.p - center) / radius;
            record.material = material;

            return true;
        }

        temp = (-b + sqrt(b*b - a*c)) / a;

        if (temp < t_max && temp > t_min)
        {
            record.t = temp;
            record.p = r.pointAtParameter(temp);
            record.normal = (record.p - center) / radius;
            record.material = material;

            return true;
        }
    }

    return false;
}