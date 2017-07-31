#include <math.h>
#include <stdlib.h>
#include <fstream>		// to write results to ppm file.
using namespace std;

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

	float e[3];
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

int main()
{
	int width = 200;
	int height = 100;

	ofstream outFile;
	outFile.open("render.ppm");

	outFile << "P3\n" << width << " " << height << "\n255\n";

	for (int h = height - 1; h >= 0; --h)
	{
		for (int w = 0; w < width; ++w)
		{
			Vector3 pixelColor(w / (float)width, h / (float)height, 0.2f);

			int intRed = int(255.99f * pixelColor.r());
			int intGreen = int(255.99f * pixelColor.g());
			int intBlue = int(255.99f * pixelColor.b());

			outFile << intRed << " " << intGreen << " " << intBlue << endl;
		}
	}

	outFile.close();
}


