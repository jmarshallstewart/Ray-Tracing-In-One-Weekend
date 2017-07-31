#include <fstream>		// to write results to ppm file.
using namespace std;

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
			float r = w / (float)width;
			float g = h / (float)height;
			float b = 0.2f;

			int intRed = int(255.99f * r);
			int intGreen = int(255.99f * g);
			int intBlue = int(255.99f * b);

			outFile << intRed << " " << intGreen << " " << intBlue << endl;
		}
	}

	outFile.close();
}