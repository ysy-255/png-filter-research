#ifndef PNG_H
#define PNG_H


#include <zlib.h>

#include "./IMAGE.hpp"
#include "./FILE.hpp"


struct PNG{
	std::vector<unsigned char> signature;

	// IHDR
	unsigned int Width;
	unsigned int Height;
	unsigned char Bit_depth;
	unsigned char Color_type;
	unsigned char Compression_method;
	unsigned char Filter_method;
	unsigned char Interlace_method;

	IMAGE ImageData;
	std::vector<unsigned char> methods;
};

unsigned char PaethPredictor(unsigned char a /* left */, unsigned char b /* above */, unsigned char c /* upper left */){
	unsigned short pa = abs(b - c);
	unsigned short pb = abs(a - c);
	unsigned short pc = abs(a + b - c - c);
	if(pa <= pb && pa <= pc) return a;
	if(pb <= pc) return b;
	return c;
}

void PNGstream2img(const std::vector<unsigned char> & datastream, IMAGE & img, std::vector<unsigned char> & methods, const unsigned int & width, const unsigned int & height, const unsigned char & color){
	methods.resize(height);
	img = IMAGE(height, width);
	unsigned int offset = 0;
	unsigned char filter;
	for(unsigned int y = 0; y < height; y++){
		filter = datastream[offset ++];
		methods[y] = filter;
		switch(filter){
			case 0:
				for(unsigned int x = 0; x < width; x++){
					img.R[y][x] = datastream[offset ++];
					img.G[y][x] = datastream[offset ++];
					img.B[y][x] = datastream[offset ++];
					if(color == 4)offset ++;
				}
				break;
			case 1:
				img.R[y][0] = datastream[offset ++];
				img.G[y][0] = datastream[offset ++];
				img.B[y][0] = datastream[offset ++];
				if(color == 4)offset ++;
				for(unsigned int x = 1; x < width; x++){
					img.R[y][x] = datastream[offset ++] + img.R[y][x - 1];
					img.G[y][x] = datastream[offset ++] + img.G[y][x - 1];
					img.B[y][x] = datastream[offset ++] + img.B[y][x - 1];
					if(color == 4)offset ++;
				}
				break;
			case 2:
				if(y == 0){
					for(unsigned int x = 0; x < width; x++){
						img.R[0][x] = datastream[offset ++];
						img.G[0][x] = datastream[offset ++];
						img.B[0][x] = datastream[offset ++];
						if(color == 4)offset ++;
					}
				}
				else{
					for(unsigned int x = 0; x < width; x++){
						img.R[y][x] = datastream[offset ++] + img.R[y - 1][x];
						img.G[y][x] = datastream[offset ++] + img.G[y - 1][x];
						img.B[y][x] = datastream[offset ++] + img.B[y - 1][x];
						if(color == 4)offset ++;
					}
				}
				break;
			case 3:
				if(y == 0){
					img.R[0][0] = datastream[offset ++];
					img.G[0][0] = datastream[offset ++];
					img.B[0][0] = datastream[offset ++];
					if(color == 4)offset ++;
					for(unsigned int x = 1; x < width; x++){
						img.R[0][x] = datastream[offset ++] + img.R[0][x - 1] / 2;
						img.G[0][x] = datastream[offset ++] + img.G[0][x - 1] / 2;
						img.B[0][x] = datastream[offset ++] + img.B[0][x - 1] / 2;
						if(color == 4)offset ++;
					}
				}
				else{
					img.R[y][0] = datastream[offset ++] + img.R[y - 1][0] / 2;
					img.G[y][0] = datastream[offset ++] + img.G[y - 1][0] / 2;
					img.B[y][0] = datastream[offset ++] + img.B[y - 1][0] / 2;
					if(color == 4)offset ++;
					for(unsigned int x = 1; x < width; x++){
						img.R[y][x] = datastream[offset ++] + (img.R[y][x - 1] + img.R[y - 1][x]) / 2;
						img.G[y][x] = datastream[offset ++] + (img.G[y][x - 1] + img.G[y - 1][x]) / 2;
						img.B[y][x] = datastream[offset ++] + (img.B[y][x - 1] + img.B[y - 1][x]) / 2;
						if(color == 4)offset ++;
					}
				}
				break;
			case 4:
				if(y == 0){
					img.R[0][0] = datastream[offset ++];
					img.G[0][0] = datastream[offset ++];
					img.B[0][0] = datastream[offset ++];
					if(color == 4)offset ++;
					for(unsigned int x = 1; x < width; x++){
						img.R[0][x] = datastream[offset ++] + PaethPredictor(img.R[0][x - 1], 0, 0);
						img.G[0][x] = datastream[offset ++] + PaethPredictor(img.G[0][x - 1], 0, 0);
						img.B[0][x] = datastream[offset ++] + PaethPredictor(img.B[0][x - 1], 0, 0);
						if(color == 4)offset ++;
					}
				}
				else{
					img.R[y][0] = datastream[offset ++] + PaethPredictor(0, img.R[y - 1][0], 0);
					img.G[y][0] = datastream[offset ++] + PaethPredictor(0, img.G[y - 1][0], 0);
					img.B[y][0] = datastream[offset ++] + PaethPredictor(0, img.B[y - 1][0], 0);
					if(color == 4)offset ++;
					for(unsigned int x = 1; x < width; x++){
						img.R[y][x] = datastream[offset ++] + PaethPredictor(img.R[y][x - 1], img.R[y - 1][x], img.R[y - 1][x - 1]);
						img.G[y][x] = datastream[offset ++] + PaethPredictor(img.G[y][x - 1], img.G[y - 1][x], img.G[y - 1][x - 1]);
						img.B[y][x] = datastream[offset ++] + PaethPredictor(img.B[y][x - 1], img.B[y - 1][x], img.B[y - 1][x - 1]);
						if(color == 4)offset ++;
					}
				}

		}
	}
	return;
}

void readPNG(
		const std::string & ImagePath,
		PNG & PNGimage
	){
	std::ifstream ImageFile(ImagePath, std::ios::binary | std::ios::ate);
	unsigned int ImageSize = ImageFile.tellg();
	ImageFile.seekg(0);
	std::vector<unsigned char> FileData(ImageSize);
	ImageFile.read(reinterpret_cast<char*>(FileData.data()), ImageSize);
	PNGimage.signature.resize(8);
	std::memcpy(PNGimage.signature.data(), FileData.data(), 8);
	if(PNGimage.signature != std::vector<unsigned char>{137,80,78,71,13,10,26,10}){
		char c;
		std::cerr << "unsupported file type" << "\nccontinue? [Y/n]";
		std::cin >> c;
		if(c != 'Y' && c != 'y') return;
	}

	unsigned int offset = 8;
	unsigned int length;
	std::string type; type.resize(4);
	unsigned char color;

	std::vector<unsigned char> compressed_data;
	std::vector<unsigned char> datastream;
	do{
		length = UV4_UI(FileData, offset, false);
		offset += 4;
		std::memcpy(type.data(), &FileData[offset], 4);
		offset += 4;
		if(type == "IHDR"){
			PNGimage.Width = UV4_UI(FileData, offset, false); offset += 4;
			PNGimage.Height = UV4_UI(FileData, offset, false);offset += 4;
			PNGimage.Bit_depth = FileData[offset ++]; if(PNGimage.Bit_depth != 8){std::cerr << "unsupported bit depth" << std::endl; return;}
			PNGimage.Color_type = FileData[offset ++]; if(PNGimage.Color_type != 2 && PNGimage.Color_type != 6){std::cerr << "unsupported color type" << std::endl; return;}
			PNGimage.Compression_method = FileData[offset ++]; if(PNGimage.Compression_method != 0){std::cerr << "unsupported compression method" << std::endl; return;}
			PNGimage.Filter_method = FileData[offset ++]; if(PNGimage.Filter_method != 0){std::cerr << "unsupported filter method" << std::endl; return;}
			PNGimage.Interlace_method = FileData[offset ++]; if(PNGimage.Interlace_method != 0){std::cerr << "unsupported interlace method" << std::endl; return;}

			switch(PNGimage.Color_type){
				case 2:
					color = 3;
					break;
				case 6:
					color = 4;
					break;
			}
			datastream.resize((PNGimage.Width * color + 1) * PNGimage.Height);
		}
		else if(type == "IDAT"){
			size_t currentSize = compressed_data.size();
			compressed_data.resize(currentSize + length);
			std::memcpy(&compressed_data[currentSize], &FileData[offset], length);
			offset += length;
		}
		else{
			offset += length;
		}
		offset += 4;
		// std::cerr << type << ":" << length << ";" << std::endl;
	} while(type != "IEND");

	z_stream z;
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	if(inflateInit(&z) != Z_OK){
		std::cerr << "z_streamの初期化に失敗" << std::endl;
		return;
	}
	z.next_in = compressed_data.data();
	z.avail_in = compressed_data.size();
	z.next_out = datastream.data();
	z.avail_out = datastream.size();
	int ret;
	do{
		ret = inflate(&z, Z_FINISH);
		if(ret == Z_STREAM_END)break;
		if(ret != Z_OK){
			std::cerr << "解凍処理に失敗" << std::endl;
			inflateEnd(&z);
			return;
		}
	} while(ret != Z_STREAM_END);
	inflateEnd(&z);
	PNGstream2img(datastream, PNGimage.ImageData, PNGimage.methods, PNGimage.Width, PNGimage.Height, color);
	return;
}

void writePNG(
	const IMAGE ImageData,
	const std::string outPath,
	std::vector<unsigned char> methods
	){
	bool AdditionalFilter = false;
	for(unsigned char & uc : methods) AdditionalFilter |= (uc > 4);
	std::ofstream out(outPath, std::ios::binary);
	out << (unsigned char)137 << (unsigned char)80 << (unsigned char)78 << (unsigned char)71
	    << (unsigned char) 13 << (unsigned char)10 << (unsigned char)26 << (unsigned char)10;
	UI_write(13, out, false);
	out << "IHDR";
	UI_write(ImageData.width, out, false);
	UI_write(ImageData.height, out, false);
	out << (unsigned char)8;
	out << (unsigned char)2;
	out << (unsigned char)0;
	out << (unsigned char)AdditionalFilter; // Filter Method の研究のため他と区別
	out << (unsigned char)0;
	UI_write(0, out, false); // CRC分からん

	std::vector<unsigned char> datastream((ImageData.width * 3 + 1) * ImageData.height);
	std::vector<unsigned char> compressed_data((ImageData.width * 3 + 1) * ImageData.height);
	int offset = 0;
	for(int h = 0; h < ImageData.height; h++){
		datastream[offset ++] = methods[h];
		for(int w = 0; w < ImageData.width; w++){
			datastream[offset ++] = ImageData.R[h][w];
			datastream[offset ++] = ImageData.G[h][w];
			datastream[offset ++] = ImageData.B[h][w];
		}
	}
	z_stream z;
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	if(deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK){
		std::cerr << "z_streamの初期化に失敗" << std::endl;
		return;
	}
	z.next_in = datastream.data();
	z.avail_in = datastream.size();
	z.next_out = compressed_data.data();
	z.avail_out = compressed_data.size();
	int ret;
	do{
		ret = deflate(&z, Z_FINISH);
		if(ret == Z_STREAM_END)break;
		if(ret != Z_OK){
			std::cerr << "圧縮処理に失敗" << std::endl;
			deflateEnd(&z);
			return;
		}
	} while(ret != Z_STREAM_END);
	deflateEnd(&z);
	size_t compressed_size = z.total_out;
	compressed_data.resize(compressed_size);
	UI_write(compressed_size, out, false);
	out << "IDAT";
	out.write(reinterpret_cast<char *>(compressed_data.data()), compressed_size);
	UI_write(0, out, false); // CRC分からん
	UI_write(0, out, false);
	out << "IEND";
	UI_write(0, out, false); // CRC分からん
	return;
}

#endif
