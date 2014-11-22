#include <Magick++.h>
#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include <cctype>
#include <vector>

using namespace std;
using namespace Magick;

void usage() {
	cout << "USAGE" << endl;
	cout << "Flag \t argument \t\t\t meaning" << endl;
	cout << "-I \t <input file> \t\t\t Specify input file" << endl;
	cout << "-O \t <output file> \t\t\t Specify output file" << endl;
	cout << "-g \t v(vertical) or h(horisontal) \t Use gradient as input instead of an input file" << endl;
	cout << "-t \t value between 0 and 1 \t\t Use threshold method with given level" << endl;
	cout << "-r \t \t\t\t\t Use random threshold" << endl;
	cout << "-o \t \t\t\t\t Use ordered dithering" << endl;
	cout << "-f \t \t\t\t\t Use forward diffusion dithering" << endl;
	cout << "-b \t \t\t\t\t Use both forward and backward diffusion dithering" << endl;
	cout << "-s \t \t\t\t\t Use Floyd-Steinberg dithering" << endl;
	cout << "Examples: ./main -gh -t0.7 -O gradien_thold_0.7.png" << endl;
}


void randomDitheing(const Image &in, Image &out) {
	const int width = in.size().width();
	const int height = in.size().height();

	const PixelPacket *inpp = in.getConstPixels(0, 0, width, height);
	PixelPacket *outpp = out.getPixels(0, 0, width, height);

	for (size_t i = 0; i < width * height; i++) {
		float randone = static_cast<float>(rand()) / RAND_MAX;
		Quantum level = randone * QuantumRange;
		if (inpp[i].green > level)
			outpp[i] = ColorMono(true);  /* white */
		else
			outpp[i] = ColorMono(false); /* black */
	}
}
 
 float orderedLevel (int x, int y) {
 	int matrix [3][3] = {{5,13,7}, {11, 1, 17}, {3, 15, 9}};
 	return matrix[x%3][y%3]/18.0; 
 }


void orderedDitheing(const Image &in, Image &out) {
	const int width = in.size().width();
	const int height = in.size().height();

	const PixelPacket *inpp = in.getConstPixels(0, 0, width, height);
	PixelPacket *outpp = out.getPixels(0, 0, width, height);

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float levelNorm = orderedLevel(x, y);
			Quantum level = levelNorm * QuantumRange;
			if (inpp[y*width + x].green > level)
				outpp[y*width + x] = ColorMono(true);  /* white */
			else
				outpp[y*width + x] = ColorMono(false); /* black */
		}
	}
}

void threshold(const Image &in, Image &out, double level) {
	const int width = in.size().width();
	const int height = in.size().height();

	const PixelPacket *inpp = in.getConstPixels(0, 0, width, height);
	PixelPacket *outpp = out.getPixels(0, 0, width, height);

	for (size_t i = 0; i < width * height; i++) {
		if (inpp[i].green > level* QuantumRange)
			outpp[i] = ColorMono(true);  /* white */
		else
			outpp[i] = ColorMono(false); /* black */
	}
}

int truncColor (int value) {
	if (value > QuantumRange/2) {
		return QuantumRange;
	}
	else
		return 0;
}

void forwardDitheing(const Image &in, Image &out) {
	const int width = in.size().width();
	const int height = in.size().height();

	const PixelPacket *inpp = in.getConstPixels(0, 0, width, height);
	PixelPacket *outpp = out.getPixels(0, 0, width, height);

	for (int y = 0; y < height; y++) {
		int error = 0;
		for (int x = 0; x < width; x++){
			int value = inpp[y*width + x].green + error;

			int newValue = truncColor(value);
			error = value - newValue;
			if (newValue == 0)
				outpp[y*width + x] = ColorMono (false);
			else 
				outpp[y*width + x] = ColorMono (true);		
		}
	}
}

void bothDitheing(const Image &in, Image &out) {
	const int width = in.size().width();
	const int height = in.size().height();

	const PixelPacket *inpp = in.getConstPixels(0, 0, width, height);
	PixelPacket *outpp = out.getPixels(0, 0, width, height);

	for (int y = 0; y < height; y++) {
		int error = 0;
		int xstart = 0;
		int xstop = width - 1;
		int xinc = 1;

		if (y%2 == 1) {
			std::swap(xstart, xstop);
			xinc = -xinc;
		}

		for (int x = xstart; xinc * x <= xstop; x += xinc){
			int value = inpp[y*width + x].green + error;

			int newValue = truncColor(value);
			error = value - newValue;
			if (newValue == 0)
				outpp[y*width + x] = ColorMono (false);
			else 
				outpp[y*width + x] = ColorMono (true);		
		}
	}
}

void floydDitheing(const Image &in, Image &out) {
	const int width = in.size().width();
	const int height = in.size().height();

	const PixelPacket *inpp = in.getConstPixels(0, 0, width, height);
	PixelPacket *outpp = out.getPixels(0, 0, width, height);

	vector <int> image ((width+2)*(height+1), 0);
	for (int y = 0; y < height+1; y++) {
		for (int x = 1; x < width+1; x++) {
			image [y*(width +2) + x] = inpp [y*width + x - 1].green;
		}
	}

	for (int y = 0; y < height; y++) {
		int xstart = 0;
		int xstop = width - 1;
		int xinc = 1;

		if (y%2 == 1) {
			std::swap(xstart, xstop);
			xinc = -xinc;
		}

		for (int x = xstart; xinc * x <= xstop; x += xinc){
			int index = y * (width + 2) + x + 1;
			int value = image[index];

			int newValue = truncColor(value);
			int error = value - newValue;

			/* distribute error to neighbours */
			image [index + xinc] += 5*error/16;
			image [index + width + 2] += 7*error/16;
			image [index + width + 2 + xinc] += 3*error/16;
			image [index + width + 2 - xinc] += error/16;

			if (newValue == 0)
				outpp[y*width + x] = ColorMono (false);
			else 
				outpp[y*width + x] = ColorMono (true);		
		}
	}
}

enum Mode {
	NOT_SET, 
	THRESHOLD,
	RANDOM,
	ORDERED,
	FORWARD,
	BOTH,
	FLOYD_STEINBERG
}; 

enum Direction {
		HORISONTAL,
		VERTICAL
	};

void try_set_mode(Mode &mode, Mode value) {
	if (mode == NOT_SET) {
		mode = value;
	}
	else {
		cerr << "Multiple modes specified" << endl;
		exit(1);
	}
}

void paintGradient (Image &image, Direction direction) {
	image.size(Geometry(512, 512));

	const int width = image.size().width();
	const int height = image.size().height();

	PixelPacket *outpp = image.getPixels(0, 0, width, height);

	if (direction == HORISONTAL) {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x ++) {
				outpp [y*width + x] = ColorGray (1 - y*1.0/(height - 1));
			}
		}
	}
	else {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x ++) {
				outpp [y*width + x] = ColorGray (1 - x*1.0/(width - 1));
			}
		}		
	}

}

int main(int argc, char **argv) {

	string inputFileName, outputFileName; 

	double level = 0.5;
	bool generateGradient = false;

	InitializeMagick(argv[0]);

	//considering horisontal gradient as default
	Direction gradientDir = HORISONTAL;

	static struct option long_opts[] = {
		{"input",			required_argument,	NULL, 'I'},
		{"output",			required_argument,	NULL, 'O'},
		{"gradient",		optional_argument,	NULL, 'g'},
		{"threshold",		optional_argument,	NULL, 't'},
		{"random",			no_argument,		NULL, 'r'},
		{"ordered",			no_argument,		NULL, 'o'},
		{"forward",			no_argument,		NULL, 'f'},
		{"both",			no_argument,		NULL, 'b'},
		{"floyd-steinberg",	no_argument,		NULL, 's'},
		{"help",			no_argument,		NULL, 'h'},
	};

	Mode mode = NOT_SET;

	int code, index;
	while ((code = getopt_long(argc, argv, "I:O:g::t::rofbsh", long_opts, &index)) != -1) {
		switch (code) {
			case 'I':
				inputFileName = optarg;
				break;

			case 'O':
				outputFileName = optarg;
				break;

			case 'g':
				if (optarg != NULL) {
					if (tolower(optarg[0]) == 'v') 
						gradientDir = VERTICAL;
					else if (tolower(optarg[0]) == 'h') 
						gradientDir = HORISONTAL;
					else {
						cerr << "Gradient direction value must be either horisontal (h) or vertical (v)" << endl;
						return 1;
					}
				}
				generateGradient = true;
				break;

			case 't': 
				if (optarg != NULL) {
					level = atof(optarg);
					if ((level < 0 ) || (level >1 )) {
						cerr << "Threshold level must be a value between 0 and 1" << endl;
						return 1;
					}
				}
				try_set_mode(mode, THRESHOLD);
				break;

			case 'r':
				try_set_mode(mode, RANDOM);
				break;

			case 'o':
				try_set_mode(mode, ORDERED);
				break;

			case 'f':
				try_set_mode(mode, FORWARD);
				break;

			case 'b':
				try_set_mode(mode, BOTH);
				break;	

			case 's':
				try_set_mode(mode, FLOYD_STEINBERG);
				break;

			case 'h':
				usage();
				return 0;

			default:
				cerr << "Unknown option `" << code << "'" << endl;	
				return 1;
		}
	}

	bool inputGiven = (inputFileName != "");
	bool outputGiven = (outputFileName != "");

	if (inputGiven == generateGradient) {
		cerr << "You should either set input file or gradient flag (-g)" << endl;
		return 1;
	}

	if (outputGiven == false) {
		cerr << "You should set output file" << endl;
		return 1;
	}

	Image input;
	if (inputGiven) {
		input.read(inputFileName);
		/* Convert image to 256 color grayscale */
		input.quantizeColorSpace(GRAYColorspace);
		input.quantizeColors(256);
		input.quantize();		
	}
	else 
		paintGradient (input, gradientDir);

	Image output(input.size(), "white");

	switch (mode) {
		case NOT_SET: 
			output = input;
			break;

		case THRESHOLD:
			threshold(input, output, level);
			break;

		case RANDOM:
			randomDitheing(input, output);
			break;

		case ORDERED:
			orderedDitheing(input, output);
			break;

		case FORWARD:
			forwardDitheing(input, output);
			break;

		case BOTH:
			bothDitheing(input, output);
			break;

	 	case FLOYD_STEINBERG:
			floydDitheing(input, output);
			break;   		
	}

	output.write(outputFileName);

	return 0;
}
