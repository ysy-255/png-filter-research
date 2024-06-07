#ifndef BMP_H
#define BMP_H


#include "./FILE.hpp"
#include "./IMAGE.hpp"


struct BMP{
	// BITMAPFILEHEADER
	std::string    bfType;      // must be 'BM' in BITMAPFILEHEADER
	unsigned int   bfSize;      // size of the file in bytes
	unsigned short bfReserved1; // must be set to 0
	unsigned short bfReserved2; // must be set to 0
	unsigned int   bfOffBits;   // offset from the beginning of the BITMAPFILEHEADER structure to the start of the actual bits

	// BITMAPINFO
	unsigned int   biSize;         	// size of the header (minus the color table)
	unsigned int   biWidth;        	// width in pixel
	unsigned int   biHeight;       	// height in pixel
	unsigned short biPlanes;       	// should always be 1
	unsigned short biBitCount;     	// defines the color resolution (in bits per pixel)
	unsigned int   biCompression;  	// type of compression; BI_RGB(value is 0) format is recommended
	unsigned int   biSizeImage;    	// should contain the size of the bitmap (not must)
	unsigned int   biXPelsPerMeter; // the desirable dimensions of the bitmap
	unsigned int   biYPelsPerMeter; // the desirable dimensions of the bitmap
	unsigned int   biClrUsed;       // define about the number of colors in the color table
	unsigned int   biClrImportant;  // first x colors of the color table are important

	IMAGE ImageData;
};

void readBMP(
		const std::string & ImagePath,
		BMP & BMPimage
	){
	std::ifstream ImageFile(ImagePath, std::ios::binary | std::ios::ate);
	unsigned int ImageSize = ImageFile.tellg();
	ImageFile.seekg(0);
	std::vector<unsigned char> FileData(ImageSize);
	ImageFile.read(reinterpret_cast<char*>(FileData.data()), ImageSize);
	BMPimage.bfType += char(FileData[0x00]),
	BMPimage.bfType += char(FileData[0x01]);
	if(BMPimage.bfType != "BM"){
		char c;
		std::cerr << "unsupported file type: " << BMPimage.bfType << "\nccontinue? [Y/n]";
		std::cin >> c;
		if(c != 'Y' && c != 'y') return;
	}

	BMPimage.bfSize      = UV4_UI(FileData, 0x02, true);
	BMPimage.bfReserved1 = UV2_US(FileData, 0x06, true);
	BMPimage.bfReserved2 = UV2_US(FileData, 0x08, true);
	BMPimage.bfOffBits	 = UV4_UI(FileData, 0x0A, true);
	
	BMPimage.biSize	= UV4_UI(FileData, 0x0E, true);
	if(BMPimage.biSize == 40){
		BMPimage.biWidth         = UV4_UI(FileData, 0x12, true);
		BMPimage.biHeight        = UV4_UI(FileData, 0x16, true);
		BMPimage.biPlanes        = UV2_US(FileData, 0x1A, true); if(BMPimage.biPlanes != 1){std::cerr << "biPlanes is not 1: " << BMPimage.biPlanes << std::endl; return;}
		BMPimage.biBitCount      = UV2_US(FileData, 0x1C, true); if(BMPimage.biBitCount != 24){std::cerr << "unsupported biBitCount: " << BMPimage.biBitCount << " (bits/pixel)" << std::endl; return;}
		BMPimage.biCompression   = UV4_UI(FileData, 0x1E, true); if(BMPimage.biCompression != 0){std::cerr << "unsupported biCompression: " << BMPimage.biCompression << std::endl; return;}
		BMPimage.biSizeImage     = UV4_UI(FileData, 0x22, true); if(BMPimage.biSizeImage == 0)BMPimage.biSizeImage = ((((BMPimage.biWidth * BMPimage.biBitCount) + 31) & ~31) >> 3) * BMPimage.biHeight;
		BMPimage.biXPelsPerMeter = UV4_UI(FileData, 0x26, true);
		BMPimage.biYPelsPerMeter = UV4_UI(FileData, 0x2A, true);
		BMPimage.biClrUsed       = UV4_UI(FileData, 0x2E, true); if(BMPimage.biClrUsed > 0){ std::cerr << "Color Pallete is unsupported" << std::endl; return;}
		BMPimage.biClrImportant  = UV4_UI(FileData, 0x32, true);
		BMPimage.ImageData = IMAGE(BMPimage.biWidth, BMPimage.biHeight);
		unsigned char extend = ((BMPimage.biWidth + 3) & ~3) - BMPimage.biWidth;
		unsigned int address = BMPimage.bfOffBits;
		for(int h = BMPimage.biHeight - 1; h >= 0 ; h++){
			for(int w = 0; w < BMPimage.biWidth; w++){
				BMPimage.ImageData.B[h][w] = FileData[address ++];
				BMPimage.ImageData.G[h][w] = FileData[address ++];
				BMPimage.ImageData.R[h][w] = FileData[address ++];
			}
			address += extend;
		}
	}
	else{
		std::cerr << "unsupported DIB size: " << BMPimage.biSize << std::endl;
		return;
	}
	return;
}

void writeBMP(
		const IMAGE & ImageData,
		const std::string & outPath
	){
	std::ofstream out(outPath, std::ios::binary);
	unsigned char extend = ((ImageData.width + 3) & ~3) - ImageData.width;
	out << "BM"; // bfType
	UI_write((ImageData.width * 3 + extend) * ImageData.height + 54, out, true); //bfSize
	US_write(0, out, 0); // bfReserved1
	US_write(0, out, 0); // bfReserved2
	UI_write(54, out, true); // bfOffBits
	UI_write(40, out, true); // biSize
	UI_write(ImageData.width, out, true); // biWidth
	UI_write(ImageData.height, out, true); // biHeight
	US_write(1, out, true); // biPlanes
	US_write(24, out, true); // biBitCount
	UI_write(0, out, 0); // biCompression
	UI_write((ImageData.width * 3 + extend) * ImageData.height, out, true); // biSizeImage
	UI_write(0, out, 0); // biXPelsPerMeter
	UI_write(0, out, 0); // biYPelsPerMeter
	UI_write(0, out, 0); // biClrUsed
	UI_write(0, out, 0); // biClrImportant
	for(int h = ImageData.height - 1; h >= 0; h--){
		for(int w = 0; w < ImageData.width; w++){
			out.put(ImageData.B[h][w]);
			out.put(ImageData.G[h][w]);
			out.put(ImageData.R[h][w]);
		}
		for(int j = 0; j < extend; j++){
			out.put(0x00);
		}
	}
	return;
}


#endif
