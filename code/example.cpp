#include "./BMP.hpp"
#include "./PNG.hpp"

int main(){
	
	PNG PNGimage1;
	readPNG("../test images/しぐたん.png", PNGimage1);
	writeBMP(PNGimage1.ImageData, "../out/png2bmp.bmp");
	
	
	BMP BMPimage1;
	readBMP("../test images/しぐたん.bmp", BMPimage1);
	std::vector<unsigned char> methods(BMPimage1.ImageData.height, 0 /*すべてNoneでフィルターされたものとする*/);

	writePNG(BMPimage1.ImageData, "../out/bmp2png.png", methods);


	IMAGE image1 = PNGimage1.ImageData;
	image2imos(image1);
	writeBMP(image1, "../out/imos.bmp");
	imos2image(image1);
	writeBMP(image1, "../out/imos_re.bmp");

	IMAGE image2 = image_enlarge(image1, 0.4, 0.5, NEAREST_NEIGHBOR);
	writeBMP(image2, "../out/0.4×0.5.bmp");
	IMAGE image3 = image_enlarge(image2, 2.5, 2, NEAREST_NEIGHBOR);
	writeBMP(image3, "../out/1.0×1.0.bmp");

	float gap = image_compare(image1, image3);
	std::cout << "1画素あたりの差: " << gap << " / 256" << std::endl;
	
	return 0;
}


// $ g++ example.cpp -o example -lz
// $ ./example

// 1画素あたりの差: 8.83176 / 256
