#include "BMP.hpp"
#include "PNG_opt.hpp"

int main(){

	PNG_opt png("../test images/しぐたん.png");
	BMP bmp("../test images/しぐたん.bmp");
	
	PNG png2(bmp.ImageData);
	BMP bmp2(png.ImageData);

	png.write("../out/png.png");
	bmp.write("../out/bmp.bmp");
	png2.write("../out/bmp2png.png");
	bmp2.write("../out/png2bmp.bmp");

	png.filterer_opt(1, 5, true, false);
	png.write("../out/filtered.png");
	
	return 0;
}


// $ g++ -O2 example.cpp -lz
