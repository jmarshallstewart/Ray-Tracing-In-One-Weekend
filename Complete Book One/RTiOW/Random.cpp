#include "Random.h"
#include <stdlib.h>
#include <time.h>

void SeedRng()
{
    SeedRng((unsigned int)time(nullptr));
}

void SeedRng(unsigned int seed)
{
	srand(seed);
}

float GetUniformRandom()
{
    return (float)rand() / RAND_MAX;
}

// draw distribution
Vector3 RandomInUnitSphere()
{
    Vector3 p;

    do
    {
        float r1 = GetUniformRandom();
        float r2 = GetUniformRandom();
        float r3 = GetUniformRandom();

        p = 2.0f * Vector3(r1, r2, r3) - Vector3(1, 1, 1);
    } while (p.squaredLength() > 1.0f);

    return p;
}

Vector3 RandomInUnitDisk()
{
    Vector3 p;

    do
    {
        float r1 = GetUniformRandom();
        float r2 = GetUniformRandom();

        p = 2.0f * Vector3(r1, r2, 0.0f) - Vector3(1, 1, 0);
    } while (dot(p, p) >= 1.0f);

    return p;
}