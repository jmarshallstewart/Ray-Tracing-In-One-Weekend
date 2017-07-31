#include <math.h>
#include <stdlib.h>
#include <fstream>		// to write results to ppm file.
#include <iostream>
#include <chrono>		// to record start and end time and calculate elapsed time.
#include <thread>		// split up image into rows and trace in threads.
#include <vector>		// used for storing worker threads.
#include <atomic>
#include <mutex>		// lock for some cout stuff that occurs at the end of a thread just before join().
using namespace std;
using namespace chrono;

static mutex coutMutex;

atomic<int> jobsDone{ 0 };
atomic<int> nextJob{ 0 };
int width = 800;
int height = 400;
int numSamples = 100;
int* pixelBuffer = nullptr;

class Vector3
{
public:
	Vector3() {}

	Vector3(float e0, float e1, float e2)
	{
		e[0] = e0;
		e[1] = e1;
		e[2] = e2;
	}

	inline float x() const { return e[0]; }
	inline float y() const { return e[1]; }
	inline float z() const { return e[2]; }

	inline float r() const { return e[0]; }
	inline float g() const { return e[1]; }
	inline float b() const { return e[2]; }

	inline Vector3& operator+=(const Vector3& v);
	inline Vector3& operator/=(const float t);

	float e[3];

	inline float length() const
	{
		return sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2]);
	}
};

inline std::istream& operator>>(std::istream& is, Vector3& t)
{
	is >> t.e[0] >> t.e[1] >> t.e[2];
	return is;
}

inline std::ostream& operator<<(std::ostream &os, const Vector3& t)
{
	os << t.e[0] << " " << t.e[1] << " " << t.e[2];
	return os;
}

inline Vector3 operator+(const Vector3& v1, const Vector3& v2)
{
	return Vector3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
}

inline Vector3 operator-(const Vector3& v1, const Vector3& v2)
{
	return Vector3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
}

inline Vector3 operator*(float t, const Vector3& v)
{
	return Vector3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline Vector3 operator*(const Vector3& v, float t)
{
	return Vector3(t * v.e[0], t * v.e[1], t * v.e[2]);
}

inline Vector3 operator/(const Vector3& v, float t)
{
	return Vector3(v.e[0] / t, v.e[1] / t, v.e[2] / t);
}

inline Vector3& Vector3::operator+=(const Vector3& v)
{
	e[0] += v.e[0];
	e[1] += v.e[1];
	e[2] += v.e[2];

	return *this;
}

inline Vector3& Vector3::operator/=(const float t)
{
	float k = 1.0f / t;

	e[0] *= k;
	e[1] *= k;
	e[2] *= k;

	return *this;
}

inline Vector3 unitVector(Vector3 v)
{
	return v / v.length();
}

inline float dot(const Vector3& v1, const Vector3& v2)
{
	return v1.e[0] * v2.e[0] + v1.e[1] * v2.e[1] + v1.e[2] * v2.e[2];
}

class Ray
{
public:
	Ray() {}
	Ray(const Vector3& o, const Vector3& d) :
		origin(o),
		direction(d)
	{}

	Vector3 pointAtParameter(float t) const
	{
		return origin + t * direction;
	}

	Vector3 origin;
	Vector3 direction;
};

float GetUniformRandom()
{
	return (float)rand() / RAND_MAX;
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

class Camera
{
public:
	Ray GetRay(float s, float t) const
	{
		return Ray(origin, lowerLeftCorner + s * horizontal + t * vertical - origin);
	}

	Vector3 lowerLeftCorner{ -2, -1, -1 };
	Vector3 horizontal{ 4, 0, 0 };
	Vector3 vertical{ 0, 2, 0 };
	Vector3 origin{ 0, 0, 0 };
};

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

float HitSphere(const Vector3& center, float radius, const Ray& ray)
{
	Vector3 rayOriginToCenter = ray.origin - center;

	float a = dot(ray.direction, ray.direction);
	float b = 2.0f * dot(rayOriginToCenter, ray.direction);
	float c = dot(rayOriginToCenter, rayOriginToCenter) - radius * radius;
	float discriminant = b * b - 4.0f * a * c;

	if (discriminant < 0)
	{
		return -1.0f;
	}
	else
	{
		return (-b - sqrt(discriminant)) / (2.0f * a);
	}
}

Vector3 Color(const Ray& ray)
{
	float t = HitSphere(Vector3(0, 0, -1), 0.5f, ray);

	if (t > 0.0f)
	{
		Vector3 normal = unitVector(ray.pointAtParameter(t) - Vector3(0, 0, -1));
		return 0.5f * Vector3(normal.x() + 1, normal.y() + 1, normal.z() + 1);
	}

	// sky gradient
	Vector3 unitDirection = unitVector(ray.direction);
	t = 0.5f * (unitDirection.y() + 1.0f);
	return (1.0f - t) * Vector3(1, 1, 1) + t * Vector3(0.5f, 0.7f, 1);
}

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

Camera camera;

void Trace()
{
	do
	{
		int y = nextJob++;

		int index = ((height - 1) - y) * width * 3;

		for (int w = 0; w < width; ++w)
		{
			Vector3 pixelColor(0, 0, 0);

			for (int sample = 0; sample < numSamples; ++sample)
			{
				float u = (w + GetUniformRandom()) / (float)width;
				float v = (y + GetUniformRandom()) / (float)height;

				Ray ray = camera.GetRay(u, v);
				pixelColor += Color(ray);
			}

			pixelColor /= (float)numSamples;

			pixelBuffer[index++] = int(255.99f * pixelColor.r());
			pixelBuffer[index++] = int(255.99f * pixelColor.g());
			pixelBuffer[index++] = int(255.99f * pixelColor.b());
		}
		
		++jobsDone;

		if (jobsDone % 10 == 0)
		{
			lock_guard<mutex> lock(coutMutex);
			cout << jobsDone << "\\" << height << " jobs finished." << endl;
		}
	} while (nextJob < height);
}

int main()
{
	time_point<steady_clock> start = steady_clock::now();
		
	pixelBuffer = new int[width * height * 3];

	unsigned int numThreads = thread::hardware_concurrency();
	vector<thread> workers;

	unsigned int nextThreadIndex = 0;
	while (nextThreadIndex < numThreads && nextThreadIndex < (unsigned int)height)
	{
		workers.emplace_back(thread(Trace));
		++nextThreadIndex;
	}

	numThreads = (int)workers.size();
	cout << numThreads << " workers launched to complete " << height << " jobs." << endl;

	unsigned int i = 1;
	for (auto& worker : workers)
	{
		worker.join();
	}
	
	WritePpmFile(pixelBuffer, width, height, "render.ppm");
	delete[] pixelBuffer;

	duration<double> elapsedSeconds = steady_clock::now() - start;
	cout << "Elapsed time: " << elapsedSeconds.count() << " seconds" << endl;
	cin.get();
}