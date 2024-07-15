#include "./BMP.hpp"
#include "./PNG.hpp"

int main(){
	
	PNG PNGimage1;
	readPNG("../test images/しぐたん.png", PNGimage1);
	writeBMP(PNGimage1.ImageData, "../out/png2bmp.bmp");
	
	
	BMP BMPimage1;
	readBMP("../test images/しぐたん.bmp", BMPimage1);
	writePNG(BMPimage1.ImageData, "../out/bmp2png.png");

	PNG image1 = PNGimage1;
	filterer(image1, true);
	writePNG(image1.ImageData, "../out/filter.png", image1.methods);
	writePNG(image1.ImageData, "../out/filter_raw.png"); // filtered, but no methods data

	image1 = PNGimage1;
	std::vector<unsigned char> datastream = filterer(image1, false);
	writePNG_datastream(image1.Width, image1.Height, datastream, "../out/faster.png", false); // faster than writePNG


	IMAGE image2 = PNGimage1.ImageData;
	image2imos(image2);
	writePNG(image2, "../out/imos.png");
	imos2image(image2);
	writePNG(image2, "../out/imos_re.png");

	IMAGE image3 = image_enlarge(image2, 0.4, 0.5, NEAREST_NEIGHBOR);
	writePNG(image3, "../out/0.4×0.5.png");
	IMAGE image4 = image_enlarge(image3, 2.5, 2, NEAREST_NEIGHBOR);
	writePNG(image4, "../out/1.0×1.0.png");

	float gap = image_compare(image2, image4);
	std::cout << "1画素あたりの差: " << gap << " / 256" << std::endl;
	
	return 0;
}


// $ g++ example.cpp -o example -lz
// $ ./example

// 1画素あたりの差: 8.83176 / 256
