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

void PNGstream2img(
		const std::vector<unsigned char> & datastream,
		IMAGE & img, std::vector<unsigned char> & methods,
		const unsigned int & width,
		const unsigned int & height,
		const unsigned char & color
	){
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

void writePNG_datastream(
		const unsigned int width,
		const unsigned int height,
		std::vector<unsigned char> & datastream,
		const std::string & outPath,
		const bool AdditionalFilter
	){
	#define CRC32_V(crc, vec) crc32_z(crc, reinterpret_cast<const Bytef*>(vec.data()), vec.size())
	#define CRC32_S(crc, str) crc32_z(crc, reinterpret_cast<const Bytef*>(str), strlen(str))
	#define WRITE_V(out, vec) out.write(reinterpret_cast<char*>(vec.data()), vec.size())
	unsigned int crc;
	unsigned int offset;
	std::ofstream out(outPath, std::ios::binary);


	out << (unsigned char)0x89 << 'P' << 'N' << 'G' << '\r' << '\n' << (unsigned char)0x1A << '\n'; // シグネチャ


	// IHDRここから

	std::vector<unsigned char> IHDR(13);
	offset = 0;
	UI_write_UV(width, IHDR, offset , false);
	UI_write_UV(height, IHDR, offset , false);
	IHDR[offset ++] = 8; // Bit_depth
	IHDR[offset ++] = 2; // Color_type
	IHDR[offset ++] = 0; // Compression_method
	IHDR[offset ++] = AdditionalFilter; // Filter Method の研究のため他と区別
	IHDR[offset ++] = 0; // Interlace_method

	UI_write(13, out, false); // LENGTH

	out << "IHDR"; // TYPE

	WRITE_V(out, IHDR); // DATA

	crc = crc32(0L, Z_NULL, 0);
	crc = CRC32_S(crc, "IHDR");
	crc = CRC32_V(crc, IHDR);
	UI_write(crc, out, false); // CRC

	// IHDRここまで


	// IDATここから

	std::vector<unsigned char> compressed_data((width * 3 + 1) * height + 0x1000);
	z_stream z;
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;
	if(deflateInit2(&z, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 /*32768*/, 8, Z_FILTERED) != Z_OK){
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

	UI_write(compressed_size, out, false); // LENGTH

	out << "IDAT"; // TYPE

	WRITE_V(out, compressed_data); // DATA

	crc = crc32(0L, Z_NULL, 0);
	crc = CRC32_S(crc, "IDAT");
	crc = CRC32_V(crc, compressed_data);
	UI_write(crc, out, false); // CRC

	// IDATここまで


	// IENDここから

	UI_write(0, out, false); // LENGTH
	out << "IEND"; // TYPE
	/* NO DATA */
	crc = crc32(0L, Z_NULL, 0);
	crc = CRC32_S(crc, "IEND");
	UI_write(crc, out, false); // CRC

	// IENDここまで

	return;
}

void writePNG(
		const IMAGE & ImageData,
		const std::string & outPath,
		const std::vector<unsigned char> & methods
	){
	bool AdditionalFilter = false;
	for(const unsigned char & uc : methods) AdditionalFilter |= (uc > 4);
	std::vector<unsigned char> datastream((ImageData.width * 3 + 1) * ImageData.height);
	unsigned int offset = 0;
	for(int h = 0; h < ImageData.height; h++){
		datastream[offset ++] = methods[h];
		for(int w = 0; w < ImageData.width; w++){
			datastream[offset ++] = ImageData.R[h][w];
			datastream[offset ++] = ImageData.G[h][w];
			datastream[offset ++] = ImageData.B[h][w];
		}
	}
	writePNG_datastream(ImageData.width, ImageData.height, datastream, outPath, AdditionalFilter);
	return;
}

void writePNG(
		const IMAGE & ImageData,
		const std::string & outPath
	){
	std::vector<unsigned char> datastream((ImageData.width * 3 + 1) * ImageData.height);
	unsigned int offset = 0;
	for(int h = 0; h < ImageData.height; h++){
		datastream[offset ++] = 0;
		for(int w = 0; w < ImageData.width; w++){
			datastream[offset ++] = ImageData.R[h][w];
			datastream[offset ++] = ImageData.G[h][w];
			datastream[offset ++] = ImageData.B[h][w];
		}
	}
	writePNG_datastream(ImageData.width, ImageData.height, datastream, outPath, false);
	return;
}

unsigned long long ratePredictor(std::vector<unsigned char> & vec){
	unsigned long long result = 0;
	for(unsigned int i = 0; i < vec.size(); i++){
		if(vec[i] == 0){
			result += 32;
		}
		else{
			result += __builtin_clz(std::min(vec[i], (unsigned char)(256 - vec[i])));
		}
	}
	return result;
}

int signedVec(const std::vector<std::vector<unsigned char>> & vec, const int & row, const int & col){
	return row < 0 || col < 0 ? 0 : vec[row][col];
}

std::vector<unsigned char> chooser(const IMAGE & data, const int row){
	std::vector<std::vector<unsigned char>>results(5, std::vector<unsigned char>(data.width * 3 + 1));
	unsigned long long predictedRate[5];
	for(unsigned char FilterType = 0; FilterType < 5; FilterType ++){
		switch(FilterType){
			case 0:{
				results[0][0] = 0;
				unsigned int offset = 0;
				for(int w = 0; w < data.width; w++){
					results[0][++ offset] = data.R[row][w];
					results[0][++ offset] = data.G[row][w];
					results[0][++ offset] = data.B[row][w];
				}
				break;
			}
			case 1:{
				results[1][0] = 1;
				unsigned int offset = 0;
				for(int w = 0; w < data.width; w++){
					results[1][++ offset] = data.R[row][w] - signedVec(data.R, row, w - 1);
					results[1][++ offset] = data.G[row][w] - signedVec(data.G, row, w - 1);
					results[1][++ offset] = data.B[row][w] - signedVec(data.B, row, w - 1);
				}
				break;
			}
			case 2:{
				results[2][0] = 2;
				unsigned int offset = 0;
				for(int w = 0; w < data.width; w++){
					results[2][++ offset] = data.R[row][w] - signedVec(data.R, row - 1, w);
					results[2][++ offset] = data.G[row][w] - signedVec(data.G, row - 1, w);
					results[2][++ offset] = data.B[row][w] - signedVec(data.B, row - 1, w);
				}
				break;
			}
			case 3:{
				results[3][0] = 3;
				unsigned int offset = 0;
				for(int w = 0; w < data.width; w++){
					results[3][++ offset] = data.R[row][w] - (signedVec(data.R, row, w - 1) + signedVec(data.R, row - 1, w)) / 2;
					results[3][++ offset] = data.G[row][w] - (signedVec(data.G, row, w - 1) + signedVec(data.G, row - 1, w)) / 2;
					results[3][++ offset] = data.B[row][w] - (signedVec(data.B, row, w - 1) + signedVec(data.B, row - 1, w)) / 2;
				}
				break;
			}
			case 4:{
				results[4][0] = 4;
				unsigned int offset = 0;
				for(int w = 0; w < data.width; w++){
					results[4][++ offset] = data.R[row][w] - PaethPredictor(signedVec(data.R, row, w - 1), signedVec(data.R, row - 1, w), signedVec(data.R, row - 1, w - 1));
					results[4][++ offset] = data.G[row][w] - PaethPredictor(signedVec(data.G, row, w - 1), signedVec(data.G, row - 1, w), signedVec(data.G, row - 1, w - 1));
					results[4][++ offset] = data.B[row][w] - PaethPredictor(signedVec(data.B, row, w - 1), signedVec(data.B, row - 1, w), signedVec(data.B, row - 1, w - 1));
				}
				break;
			}
		}
	}
	unsigned long long max_value = 0;
	unsigned char max_element = 0;
	for(int i = 0; i < 5; i++){
		predictedRate[i] = ratePredictor(results[i]);
		// std::cout << i << ": " << predictedRate[i] << ", ";
		if(predictedRate[i] > max_value){
			max_value = predictedRate[i];
			max_element = i;
		}
	}
	return results[max_element];
}

std::vector<unsigned char> filterer(PNG & data, bool change){
	unsigned int streamWidth = data.Width * 3 + 1;
	std::vector<unsigned char> datastream(streamWidth * data.Height);
	for(unsigned int h = 0; h < data.Height; h++){
		std::memcpy(datastream.data() + streamWidth * h, chooser(data.ImageData, h).data(), streamWidth);
		data.methods[h] = datastream[streamWidth * h];
		if(change){
			unsigned int offset = streamWidth * h;
			for(unsigned int w = 0; w < data.Width; w++){
				data.ImageData.R[h][w] = datastream[++ offset];
				data.ImageData.G[h][w] = datastream[++ offset];
				data.ImageData.B[h][w] = datastream[++ offset];
			}
		}
	}
	return datastream;
}

#endif
