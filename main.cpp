#include <Magick++.h>
#include <iostream>
#include <stdlib.h>
#include <getopt.h>
#include <cctype>

using namespace std;
using namespace Magick;

int usage(const char *prog) {
    std::cout << "USAGE: " << prog << " input output" << std::endl;
    return 1;
}

void process(const Image &in, Image &out) {
    const int width = in.size().width();
    const int height = in.size().height();

    const PixelPacket *inpp = in.getConstPixels(0, 0, width, height);
    PixelPacket *outpp = out.getPixels(0, 0, width, height);

    for (size_t i = 0; i < width * height; i++) {
    	float randone = static_cast<float>(rand()) / RAND_MAX;
    	Quantum level = randone * (QuantumRange - 1);

    	//Quantum level = QuantumRange / 2
        if (inpp[i].green > level)
            outpp[i] = ColorMono(true);  /* white */
        else
            outpp[i] = ColorMono(false); /* black */
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

    for (int y = 0; y < height; y++) {
    	for (int x = 0; x < width; x ++) {
    		outpp [y*width + x] = ColorGray (1 - x*1.0/(width - 1));
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
	};

	Mode mode = NOT_SET;

	int code, index;
	while ((code = getopt_long(argc, argv, "I:O:g::t::rofbs", long_opts, &index)) != -1) {
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
	    /* Convert image to 8-bit grayscale */
	    input.quantizeColorSpace(GRAYColorspace);
	    input.quantizeColors(256);
	    input.quantize();    	
    }
    else 
    	paintGradient (input, gradientDir);

    Image output(input.size(), "white");

    process(input, output);

    output.write(outputFileName);

    return 0;
}
