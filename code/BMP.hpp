#ifndef BMP_H
#define BMP_H

#include <cassert>

#include "./FILE.hpp"
#include "./IMAGE.hpp"


class BMP{
	public:
	// BITMAPFILEHEADER
	std::string bfType = "BM"; // must be 'BM' in BITMAPFILEHEADER
	uint32_t bfSize;           // size of the file in bytes
	uint16_t bfReserved1 = 0;  // must be set to 0
	uint16_t bfReserved2 = 0;  // must be set to 0
	uint32_t bfOffBits = 54;   // offset from the beginning of the BITMAPFILEHEADER structure to the start of the actual bits

	// BITMAPINFO
	uint32_t biSize = 40; // size of the header (minus the color table)
	uint32_t biWidth;     // width in pixel
	uint32_t biHeight;    // height in pixel
	uint16_t biPlanes = 1;    // should always be 1
	uint16_t biBitCount = 24; // defines the color resolution (in bits per pixel)
	uint32_t biCompression = 0;   // type of compression; BI_RGB(value is 0) format is recommended
	uint32_t biSizeImage = 0;     // should contain the size of the bitmap (not must)
	uint32_t biXPelsPerMeter = 0; // the desirable dimensions of the bitmap
	uint32_t biYPelsPerMeter = 0; // the desirable dimensions of the bitmap
	uint32_t biClrUsed = 0;       // define about the number of colors in the color table
	uint32_t biClrImportant = 0;  // first x colors of the color table are important

	IMAGE ImageData;

	BMP(){};
	BMP(const uint32_t W, const uint32_t H) : biWidth(W), biHeight(H), ImageData(W, H){}
	BMP(const IMAGE & img) : biWidth(img.width), biHeight(img.height), ImageData(img){}
	BMP(const std::string & path) {read(path);}

	bool read(const std::string & path){
		BMPstream = filepath2data(path);
		if(BMPstream.empty()) return true;
		size_t index = 0, newindex;
		newindex = index + 2;
		std::copy(&BMPstream[index], &BMPstream[newindex], bfType.data());
		index = newindex;
		assert(bfType == "BM");
		bfSize      = VU8_U32(BMPstream, index, true);
		bfReserved1 = VU8_U16(BMPstream, index, true);
		bfReserved2 = VU8_U16(BMPstream, index, true);
		bfOffBits	 = VU8_U32(BMPstream, index, true);

		biSize	= VU8_U32(BMPstream, index, true);
		assert(biSize == 40);
		biHeight        = VU8_U32(BMPstream, index, true);
		biWidth         = VU8_U32(BMPstream, index, true);
		biPlanes        = VU8_U16(BMPstream, index, true); assert(biPlanes == 1);
		biBitCount      = VU8_U16(BMPstream, index, true); assert(biBitCount == 24); fixed_row_length = ((biWidth * biBitCount + 31) >> 5) << 2;
		biCompression   = VU8_U32(BMPstream, index, true); assert(biCompression == 0);
		biSizeImage     = VU8_U32(BMPstream, index, true); if(biSizeImage == 0) biSizeImage = fixed_row_length * biHeight;
		biXPelsPerMeter = VU8_U32(BMPstream, index, true);
		biYPelsPerMeter = VU8_U32(BMPstream, index, true);
		biClrUsed       = VU8_U32(BMPstream, index, true); assert(biClrUsed == 0);
		biClrImportant  = VU8_U32(BMPstream, index, true);

		uint8_t rest = fixed_row_length - biWidth * 3;
		assert(rest < 4);
		ImageData = IMAGE(biWidth, biHeight);
		index = bfOffBits;
		for(uint32_t h = biHeight; h > 0; --h){
			for(uint32_t w = 0; w < biWidth; ++w){
				ImageData.B[h - 1][w] = BMPstream[index ++];
				ImageData.G[h - 1][w] = BMPstream[index ++];
				ImageData.R[h - 1][w] = BMPstream[index ++];
			}
			index += rest;
		}
		return false;
	}

	void write(const std::string & path){
		fixed_row_length = ((biWidth * biBitCount + 31) >> 5) << 2;
		bfOffBits = 54; biSize = 40;
		biSizeImage = fixed_row_length * biHeight;
		bfSize = bfOffBits + biSizeImage;
		BMPstream.resize(bfSize);

		size_t index = 0;

		std::copy(bfType.begin(), bfType.end(), &BMPstream[index]);
		index += 2;
		U32_write_VU8(bfSize, BMPstream, index, true);
		U16_write_VU8(bfReserved1, BMPstream, index, true);
		U16_write_VU8(bfReserved2, BMPstream, index, true);
		U32_write_VU8(bfOffBits, BMPstream, index, true);

		U32_write_VU8(biSize, BMPstream, index, true);
		U32_write_VU8(biWidth, BMPstream, index, true);
		U32_write_VU8(biHeight, BMPstream, index, true);
		U16_write_VU8(biPlanes, BMPstream, index, true);
		U16_write_VU8(biBitCount, BMPstream, index, true);
		U32_write_VU8(biCompression, BMPstream, index, true);
		U32_write_VU8(biSizeImage, BMPstream, index, true);
		U32_write_VU8(biXPelsPerMeter, BMPstream, index, true);
		U32_write_VU8(biYPelsPerMeter, BMPstream, index, true);
		U32_write_VU8(biClrUsed, BMPstream, index, true);
		U32_write_VU8(biClrImportant, BMPstream, index, true);

		uint8_t rest = fixed_row_length - biWidth * 3;
		assert(rest < 4);
		for(uint32_t h = biHeight; h > 0; --h){
			for(uint32_t w = 0; w < biWidth; ++w){
				BMPstream[index ++] = ImageData.B[h - 1][w];
				BMPstream[index ++] = ImageData.G[h - 1][w];
				BMPstream[index ++] = ImageData.R[h - 1][w];
			}
			for(uint8_t i = 0; i < rest; ++i){
				BMPstream[index ++] = 0;
			}
		}

		assert(index == BMPstream.size() && bfSize == BMPstream.size());

		std::ofstream out(path, std::ios::binary);
		out.write(reinterpret_cast<char*>(BMPstream.data()), BMPstream.size());
		out.close();
		return;
	}

	private:

	std::vector<uint8_t> BMPstream;

	uint32_t fixed_row_length;
};

#endif
