#include <Magick++.h>
#include <iostream>

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
        if (inpp[i].green > QuantumRange / 2)
            outpp[i] = ColorMono(true);  /* white */
        else
            outpp[i] = ColorMono(false); /* black */
    }
}

int main(int argc, char **argv) {
    if (argc != 3)
        return usage(argv[0]);

    InitializeMagick(argv[0]);

    Image input;
    input.read(argv[1]);

    /* Convert image to 8-bit grayscale */
    input.quantizeColorSpace(GRAYColorspace);
    input.quantizeColors(256);
    input.quantize();

    Image output(input.size(), "white");

    process(input, output);

    output.write(argv[2]);

    return 0;
}
