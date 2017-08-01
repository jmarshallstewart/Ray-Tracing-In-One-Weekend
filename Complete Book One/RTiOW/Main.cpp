#include <iostream>		// cout
#include <fstream>		// to write results to ppm file.
#include <float.h>		// for FLT_MAX
#include <assert.h>		// assert(true)
#include <atomic>		// to count number of completed threads.
#include <chrono>		// to record start and end time and calculate elapsed time.
#include <thread>		// split up image into rows and trace in threads.
#include <vector>		// used for storing worker threads.
#include <mutex>		// lock for some cout stuff that occurs at the end of a thread just before join().
#include "Random.h"
#include "Sphere.h"
#include "HitableList.h"
#include "Material.h"
#include "Lambertian.h"
#include "Metal.h"
#include "Dielectric.h"
#include "Camera.h"
using namespace std;
using namespace chrono;

static mutex coutMutex;

const int nx = 640;
const int ny = 480;
const int ns = 100;
const int maxDepth = 50;
const int numThreadsScalar = 2;
const Vector3 up(0, 1, 0);
atomic<int> jobsDone{ 0 };
atomic<int> nextJob{ 0 };
int* pixelBuffer = nullptr;
Hitable* world = nullptr;
Vector3 lookFrom(0, 2.5, 4);
Vector3 lookAt(0, 0, -1);
float distanceToFocus = (lookFrom - lookAt).length();
float aperture = 0.03f;
Camera camera(lookFrom, lookAt, up, 90.0f, float(nx) / float(ny), aperture, distanceToFocus);
Dielectric dielectricMaterial(1.5f);

// buffer is an array of pixel data in rgb format, i.e., length = width * height * 3 ints.
void WritePpmFile(int* buffer, int width, int height, const char* fileName)
{
	ofstream outFile;
	outFile.open(fileName);

	outFile << "P3\n" << width << " " << height << "\n255\n";

	int length = width * height * 3;

	for (int i = 0; i < length; i += 3)
	{
		outFile << buffer[i] << " " << buffer[i + 1] << " " << buffer[i + 2] << "\n";
	}

	outFile.close();
}

Hitable* GenerateRandomScene()
{
	const int range = 7;

	Hitable** list = new Hitable*[250];

	list[0] = new Sphere(Vector3(0.0f, -1000.0f, 0.0f), 1000, new Lambertian(Vector3(0.5f, 0.5f, 0.5f)));

	int i = 1;

	for (int a = -range; a < range; ++a)
	{
		for (int b = -range; b < range; ++b)
		{
			float chooseMaterial = GetUniformRandom();

			Vector3 center(a + 1.5f * GetUniformRandom(), 0.2f, b + 1.5f * GetUniformRandom());

			if ((center - Vector3(4.0f, 0.2f, 0.0f)).length() > 0.9f)
			{
				if (chooseMaterial < 0.8f)
				{
					Material* mat = new Lambertian(Vector3(GetUniformRandom() * GetUniformRandom(), GetUniformRandom() * GetUniformRandom(), GetUniformRandom() * GetUniformRandom()));
					list[i++] = new Sphere(center, 0.2f, mat);
				}
				else if (chooseMaterial < 0.95f)
				{
					Material* mat = new Metal(Vector3(0.5f * (1 + GetUniformRandom()), 0.5f * (1 + GetUniformRandom()), 0.5f * (1 + GetUniformRandom())), 0.5f * GetUniformRandom());
					list[i++] = new Sphere(center, 0.2f, mat);
				}
				else
				{
					list[i++] = new Sphere(center, 0.2f, &dielectricMaterial);
				}
			}
		}
	}

	list[i++] = new Sphere(Vector3(-3.9f, 1, 0), 1.0f, new Lambertian(Vector3(0.4f, 0.9f, 0.1f)));
	list[i++] = new Sphere(Vector3(0, 1, 0), 1.0f, new Metal(Vector3(0.7f, 0.6f, 0.5f), 0.001f));
	list[i++] = new Sphere(Vector3(3.9f, 1, 0), 1.0f, &dielectricMaterial);

	cout << "World Objects Created: " << i << endl;
	return new HitableList(list, i);
}

Vector3 color(const Ray& r, Hitable* world, int depth)
{
	HitRecord record;

	if (world->hit(r, 0.001f, FLT_MAX, record))
	{
		Ray scattered;
		Vector3 attenuation;

		if (depth < maxDepth && record.material->Scatter(r, record, attenuation, scattered))
		{
			return attenuation * color(scattered, world, depth + 1);
		}
		else
		{
			return Vector3(0, 0, 0);
		}
	}
	else // hit background, gradient from white to light blue.
	{
		Vector3 unitDirection = unitVector(r.direction);
		float t = 0.5f * unitDirection.y() + 1.0f;
		return (1.0f - t) * Vector3(1.0f, 1.0f, 1.0f) + t * Vector3(0.5f, 0.7f, 1.0f);
	}
}

void trace()
{
	do
	{
		int y = nextJob++;

		int index = ((ny - 1) - y) * nx * 3;
		float oneOverNx = 1.0f / nx;
		float oneOverNy = 1.0f / ny;

		for (int i = 0; i < nx; ++i)
		{
			Vector3 col(0, 0, 0);

			for (int s = ns - 1; s >= 0; --s)
			{
				float u = float(i + GetUniformRandom()) * oneOverNx;
				float v = float(y + GetUniformRandom()) * oneOverNy;
				Ray r = camera.GetRay(u, v);
				col += color(r, world, 0);
			}

			col /= float(ns);
			col = Vector3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));

			pixelBuffer[index++] = int(255.99f * col[0]);
			pixelBuffer[index++] = int(255.99f * col[1]);
			pixelBuffer[index++] = int(255.99f * col[2]);
		}

		++jobsDone;

		if (jobsDone % 10 == 0)
		{
			lock_guard<mutex> lock(coutMutex);
			cout << jobsDone << "\\" << ny << " jobs finished." << endl;
		}
	} while (nextJob < ny);
}

int main()
{
	SeedRng();
	world = GenerateRandomScene();

	time_point<steady_clock> start = steady_clock::now();

	pixelBuffer = new int[nx * ny * 3];

	unsigned int numThreads = thread::hardware_concurrency() * numThreadsScalar;
	vector<thread> workers;

	unsigned int nextThreadIndex = 0;
	while (nextThreadIndex < numThreads && nextThreadIndex < ny)
	{
		workers.emplace_back(thread(trace));
		++nextThreadIndex;
	}

	numThreads = (int)workers.size();
	{
		lock_guard<mutex> lock(coutMutex);
		cout << numThreads << " workers launched to complete " << ny << " jobs." << endl;
	}
	
	for (auto& worker : workers)
	{
		worker.join();
	}

	duration<double> elapsedSeconds = steady_clock::now() - start;
	cout << "Elapsed time: " << elapsedSeconds.count() << " seconds" << endl;

	WritePpmFile(pixelBuffer, nx, ny, "render.ppm");
	delete[] pixelBuffer;
}