#pragma once
#include "Vector3.h"

void SeedRng();
void SeedRng(unsigned int seed);
float GetUniformRandom();
Vector3 RandomInUnitSphere();
Vector3 RandomInUnitDisk();