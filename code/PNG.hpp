#ifndef PNG_H
#define PNG_H


#include <zlib.h>

#include <string>
#include <string.h>
#include <array>
#include <cassert>


#include "./IMAGE.hpp"
#include "./FILE.hpp"

#define PNG_IHDR_SIZE 13
#define PNG_IEND_SIZE 0

class PNG{
	public:

	// IHDR
	uint32_t Width;
	uint32_t Height;
	uint8_t Bit_depth = 8;
	uint8_t Color_type = 2; // 2:RGB 6:RGBA
	uint8_t Compression_method = 0; // 0
	uint8_t Filter_method = 0; // 0
	uint8_t Interlace_method = 0; // 0:no 1:has

	IMAGE ImageData;

	uint8_t channel_num = 3;
	bool has_alpha = false;
	std::vector<uint8_t> filtered_stream;
	std::vector<uint8_t> filter_methods;

	std::vector<std::vector<uint8_t>> &R = ImageData.R;
	std::vector<std::vector<uint8_t>> &G = ImageData.G;
	std::vector<std::vector<uint8_t>> &B = ImageData.B;

	PNG(){};
	PNG(const uint32_t W, const uint32_t H) : Width(W), Height(H), ImageData(W, H){}
	PNG(const IMAGE & img) : Width(img.width), Height(img.height), ImageData(img){}
	PNG(const std::string & path) {read(path);}

	bool read(const std::string & path){
		PNGstream = filepath2data(path);
		if(PNGstream.empty()) return true;
		size_t index = 0, newindex;
		newindex = index + 8;
		std::copy(&PNGstream[index], &PNGstream[newindex], signature.data());
		index = newindex;
		if(signature != correct_signature) return true;

		std::vector<uint8_t> IDATstream;

		std::string type(4,'\0');
		do{
			uint32_t length = VU8_U32(PNGstream, index, false);
			newindex = index + 4;
			std::copy(&PNGstream[index], &PNGstream[newindex], type.data());
			index = newindex;

			if(type == "IHDR"){
				assert(index == 8 + 4 + 4);
				Width = VU8_U32(PNGstream, index, false);
				Height = VU8_U32(PNGstream, index, false);
				Bit_depth = PNGstream[index ++];
				Color_type = PNGstream[index ++];
				Compression_method = PNGstream[index ++];
				Filter_method = PNGstream[index ++];
				Interlace_method = PNGstream[index ++];
				channel_num = Color_type == 2 ? 3 : Color_type == 6 ? 4 : 0;
				has_alpha = channel_num == 4;
				IDATstream.reserve((channel_num * Width + 1) * Height);
			}
			else if(type == "IDAT"){
				assert(index >= 8 + 4 + 4 + 4 + 4 + 1 + 1 + 1 + 1 + 1 + 4 + 4);
				size_t current_size = IDATstream.size();
				IDATstream.resize(current_size + length);
				newindex = index + length;
				std::copy(&PNGstream[index], &PNGstream[newindex], &IDATstream[current_size]);
				index = newindex;
			}
			else{
				assert(index >= 8 + 4 + 4 + 4 + 4 + 1 + 1 + 1 + 1 + 1 + 4 + 4);
				index += length;
			}
			index += 4;
		} while(type != "IEND");

		IDATstream_Inflate(IDATstream);

		unfilterer();

		if(Color_type == 6 && channel_num == 4 && has_alpha){
			Color_type = 2;
			channel_num = 3;
			has_alpha = false;
		}

		return false;
	}

	void unfilterer(){
		assert(Bit_depth == 8);
		assert(channel_num == 3 || channel_num == 4);
		assert(Compression_method == 0);
		assert(Filter_method == 0);
		assert(Interlace_method == 0);
		assert(filtered_stream.size() == (channel_num * Width + 1) * Height);

		filter_methods.resize(Height, 0);
		ImageData = IMAGE(Width, Height);
		size_t index = 0;
		for(uint32_t h = 0; h < Height; ++h){
			filter_methods[h] = filtered_stream[index ++];
			assert(filter_methods[h] < 5);
			(this->*uf_funcs[filter_methods[h]])(h, index);
		}
		
		return;
	}


	void write(const std::string & path, const bool is_filtered = false){

		PNGstream.resize((channel_num * Width + 1) * Height + 1000);
		size_t index = 0;

		std::copy(correct_signature.begin(), correct_signature.end(), &PNGstream[index]);
		index += correct_signature.size();
		
		U32_write_VU8(PNG_IHDR_SIZE, PNGstream, index, false);
		PNGstream[index ++] = 'I';
		PNGstream[index ++] = 'H';
		PNGstream[index ++] = 'D';
		PNGstream[index ++] = 'R';
		U32_write_VU8(Width, PNGstream, index, false);
		U32_write_VU8(Height, PNGstream, index, false);
		PNGstream[index ++] = Bit_depth;
		PNGstream[index ++] = Color_type;
		PNGstream[index ++] = Compression_method;
		PNGstream[index ++] = Filter_method;
		PNGstream[index ++] = Interlace_method;
		U32_write_VU8(crc32_z(0, &PNGstream[index - PNG_IHDR_SIZE - 4], PNG_IHDR_SIZE + 4), PNGstream, index, false);

		if(!is_filtered) filterer();
		std::vector<uint8_t> IDATstream = filtered_stream_Deflate();
		U32_write_VU8(IDATstream.size(), PNGstream, index, false);
		PNGstream[index ++] = 'I';
		PNGstream[index ++] = 'D';
		PNGstream[index ++] = 'A';
		PNGstream[index ++] = 'T';
		std::copy(IDATstream.begin(), IDATstream.end(), &PNGstream[index]);
		size_t newindex = index + IDATstream.size();
		U32_write_VU8(crc32_z(0, &PNGstream[index - 4], IDATstream.size() + 4), PNGstream, newindex, false);
		index = newindex;
		
		U32_write_VU8(0, PNGstream, index, false);
		PNGstream[index ++] = 'I';
		PNGstream[index ++] = 'E';
		PNGstream[index ++] = 'N';
		PNGstream[index ++] = 'D';
		U32_write_VU8(crc32_z(0, &PNGstream[index - 4], 4), PNGstream, index, false);

		PNGstream.resize(index);

		std::ofstream out(path, std::ios::binary);
		out.write(reinterpret_cast<char*>(PNGstream.data()), PNGstream.size());
		out.close();
		return;
	}

	void filterer(){
		size_t index = 0;
		filtered_stream.resize((channel_num * Width + 1) * Height);
		filter_methods.resize(Height);
		std::vector<uint8_t> filtered_array[5];
		for(uint8_t i = 0; i < 5; ++i){
			filtered_array[i].resize(Width * 3 + 1);
		}
		for(uint32_t h = 0; h < Height; ++h){
			uint64_t sum_of_abs_min = UINT64_MAX;
			for(uint8_t i = 0; i < 5; ++i){
				if((this->*f_funcs[i])(h, filtered_array[i])){
					continue;
				}
				uint64_t sum_of_abs = abs_sum(filtered_array[i]);
				if(sum_of_abs < sum_of_abs_min){
					sum_of_abs_min = sum_of_abs;
					filter_methods[h] = i;
				}
			}
			std::copy(filtered_array[filter_methods[h]].begin(), filtered_array[filter_methods[h]].end(), &filtered_stream[index]);
			index += channel_num * Width + 1;
		}

		return;
	}


	protected:

	std::vector<uint8_t> PNGstream;

	std::array<uint8_t, 8> signature;
	static constexpr std::array<uint8_t, 8> correct_signature = {137, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};

	// a:left b:above c:upperleft
	uint8_t paeth_predictor(const uint8_t a, const uint8_t b, const uint8_t c){
		uint16_t pa = abs(short(b) - short(c));
		uint16_t pb = abs(short(a) - short(c));
		uint16_t pc = abs(short(a) + short(b) - short(c) - short(c));
		if(pa <= pb && pa <= pc) return a;
		if(pb <= pc) return b;
		return c;
	}

	void IDATstream_Inflate(std::vector<uint8_t> & compressed_stream_all){

		filtered_stream.clear();
		filtered_stream.resize((channel_num * Width + 1) * Height);

		z_stream z; z.zalloc = Z_NULL; z.zfree = Z_NULL; z.opaque = Z_NULL;
		assert(inflateInit(&z) == Z_OK);
		z.next_in = compressed_stream_all.data();
		z.avail_in = compressed_stream_all.size();
		z.next_out = filtered_stream.data();
		z.avail_out = filtered_stream.size();
		int ret;
		do{
			ret = inflate(&z, Z_FINISH);
			assert(ret == Z_OK || ret == Z_STREAM_END);
		} while(ret != Z_STREAM_END);
		inflateEnd(&z);

		return;
	}

	// unfilter functions
	void uf_None(const uint32_t h, size_t & index){
		for(uint32_t w = 0; w < Width; ++w){
			R[h][w] = filtered_stream[index ++];
			G[h][w] = filtered_stream[index ++];
			B[h][w] = filtered_stream[index ++]; if(has_alpha) index ++;
		}
		return;
	}
	void uf_Sub(const uint32_t h, size_t & index){
		R[h][0] = filtered_stream[index ++];
		G[h][0] = filtered_stream[index ++];
		B[h][0] = filtered_stream[index ++]; if(has_alpha) index ++;
		for(uint32_t w = 1; w < Width; ++w){
			R[h][w] = filtered_stream[index ++] + R[h][w - 1];
			G[h][w] = filtered_stream[index ++] + G[h][w - 1];
			B[h][w] = filtered_stream[index ++] + B[h][w - 1];  if(has_alpha) index ++;
		}
		return;
	}
	void uf_Up(const uint32_t h, size_t & index){
		if(h == 0){
			for(uint32_t w = 0; w < Width; ++w){
				R[h][w] = filtered_stream[index ++];
				G[h][w] = filtered_stream[index ++];
				B[h][w] = filtered_stream[index ++]; if(has_alpha) index ++;
			}
		}
		else{
			for(uint32_t w = 0; w < Width; ++w){
				R[h][w] = filtered_stream[index ++] + R[h - 1][w];
				G[h][w] = filtered_stream[index ++] + G[h - 1][w];
				B[h][w] = filtered_stream[index ++] + B[h - 1][w];  if(has_alpha) index ++;
			}
		}
		return;
	}
	void uf_Ave(const uint32_t h, size_t & index){
		if(h == 0){
			R[h][0] = filtered_stream[index ++];
			G[h][0] = filtered_stream[index ++];
			B[h][0] = filtered_stream[index ++]; if(has_alpha) index ++;
			for(uint32_t w = 1; w < Width; ++w){
				R[h][w] = filtered_stream[index ++] + R[h][w - 1] / 2;
				G[h][w] = filtered_stream[index ++] + G[h][w - 1] / 2;
				B[h][w] = filtered_stream[index ++] + B[h][w - 1] / 2;  if(has_alpha) index ++;
			}
		}
		else{
			R[h][0] = filtered_stream[index ++] + R[h - 1][0] / 2;
			G[h][0] = filtered_stream[index ++] + G[h - 1][0] / 2;
			B[h][0] = filtered_stream[index ++] + B[h - 1][0] / 2; if(has_alpha) index ++;
			for(uint32_t w = 1; w < Width; ++w){
				R[h][w] = filtered_stream[index ++] + (R[h][w - 1] + R[h - 1][w]) / 2;
				G[h][w] = filtered_stream[index ++] + (G[h][w - 1] + G[h - 1][w]) / 2;
				B[h][w] = filtered_stream[index ++] + (B[h][w - 1] + B[h - 1][w]) / 2; if(has_alpha) index ++;
			}
		}
	}
	void uf_Paeth(const uint32_t h, size_t & index){
		if(h == 0){
			R[h][0] = filtered_stream[index ++];
			G[h][0] = filtered_stream[index ++];
			B[h][0] = filtered_stream[index ++]; if(has_alpha) index ++;
			for(uint32_t w = 1; w < Width; ++w){
				R[h][w] = filtered_stream[index ++] + paeth_predictor(R[h][w - 1], 0, 0);
				G[h][w] = filtered_stream[index ++] + paeth_predictor(G[h][w - 1], 0, 0);
				B[h][w] = filtered_stream[index ++] + paeth_predictor(B[h][w - 1], 0, 0); if(has_alpha) index ++;
			}
		}
		else{
			R[h][0] = filtered_stream[index ++] + paeth_predictor(0, R[h - 1][0], 0);
			G[h][0] = filtered_stream[index ++] + paeth_predictor(0, G[h - 1][0], 0);
			B[h][0] = filtered_stream[index ++] + paeth_predictor(0, B[h - 1][0], 0); if(has_alpha) index ++;
			for(uint32_t w = 1; w < Width; ++w){
				R[h][w] = filtered_stream[index ++] + paeth_predictor(R[h][w - 1], R[h - 1][w], R[h - 1][w - 1]);
				G[h][w] = filtered_stream[index ++] + paeth_predictor(G[h][w - 1], G[h - 1][w], G[h - 1][w - 1]);
				B[h][w] = filtered_stream[index ++] + paeth_predictor(B[h][w - 1], B[h - 1][w], B[h - 1][w - 1]); if(has_alpha) index ++;
			}
		}
	}

	void (PNG::*uf_funcs[5])(const uint32_t h, size_t & index)
	 = {&PNG::uf_None, &PNG::uf_Sub, &PNG::uf_Up, &PNG::uf_Ave, &PNG::uf_Paeth};

	// filter functions
	bool f_None(const uint32_t h, std::vector<uint8_t> & filtered){
		size_t index = 0;
		filtered[index ++] = 0;
		for(uint32_t w = 0; w < Width; ++w){
			filtered[index ++] = R[h][w];
			filtered[index ++] = G[h][w];
			filtered[index ++] = B[h][w];
		}
		return false;
	}
	bool f_Sub(const uint32_t h, std::vector<uint8_t> & filtered){
		size_t index = 0;
		filtered[index ++] = 1;
		filtered[index ++] = R[h][0];
		filtered[index ++] = G[h][0];
		filtered[index ++] = B[h][0];
		for(uint32_t w = 1; w < Width; ++w){
			filtered[index ++] = R[h][w] - R[h][w - 1];
			filtered[index ++] = G[h][w] - G[h][w - 1];
			filtered[index ++] = B[h][w] - B[h][w - 1];
		}
		return false;
	}
	bool f_Up(const uint32_t h, std::vector<uint8_t> & filtered){
		if(h == 0) return true;
		size_t index = 0;
		filtered[index ++] = 2;
		for(uint32_t w = 0; w < Width; ++w){
			filtered[index ++] = R[h][w] - R[h - 1][w];
			filtered[index ++] = G[h][w] - G[h - 1][w];
			filtered[index ++] = B[h][w] - B[h - 1][w];
		}
		return false;
	}
	bool f_Ave(const uint32_t h, std::vector<uint8_t> & filtered){
		if(h == 0) return true;
		size_t index = 0;
		filtered[index ++] = 3;
		filtered[index ++] = R[h][0] - (R[h - 1][0] >> 1);
		filtered[index ++] = G[h][0] - (G[h - 1][0] >> 1);
		filtered[index ++] = B[h][0] - (B[h - 1][0] >> 1);
		for(uint32_t w = 1; w < Width; ++w){
			filtered[index ++] = R[h][w] - ((R[h][w - 1] + R[h - 1][w]) >> 1);
			filtered[index ++] = G[h][w] - ((G[h][w - 1] + G[h - 1][w]) >> 1);
			filtered[index ++] = B[h][w] - ((B[h][w - 1] + B[h - 1][w]) >> 1);
		}
		return false;
	}
	bool f_Paeth(const uint32_t h, std::vector<uint8_t> & filtered){
		if(h == 0) return true;
		size_t index = 0;
		filtered[index ++] = 4;
		filtered[index ++] = R[h][0] - paeth_predictor(0, R[h - 1][0], 0);
		filtered[index ++] = G[h][0] - paeth_predictor(0, G[h - 1][0], 0);
		filtered[index ++] = B[h][0] - paeth_predictor(0, B[h - 1][0], 0);
		for(uint32_t w = 1; w < Width; ++w){
			filtered[index ++] = R[h][w] - paeth_predictor(R[h][w - 1], R[h - 1][w], R[h - 1][w - 1]);
			filtered[index ++] = G[h][w] - paeth_predictor(G[h][w - 1], G[h - 1][w], G[h - 1][w - 1]);
			filtered[index ++] = B[h][w] - paeth_predictor(B[h][w - 1], B[h - 1][w], B[h - 1][w - 1]);
		}
		return false;
	}

	bool (PNG::*f_funcs[5])(const uint32_t, std::vector<uint8_t> &)
	 = {&PNG::f_None, &PNG::f_Sub, &PNG::f_Up, &PNG::f_Ave, &PNG::f_Paeth};

	uint64_t abs_sum(const std::vector<uint8_t> & vec){
		uint64_t result = 0;
		for(const uint8_t & i : vec) result += abs(static_cast<int8_t>(i));
		return result;
	}

	std::vector<uint8_t> filtered_stream_Deflate(){
		std::vector<uint8_t> IDATstream(filtered_stream.size() + 1000);
		z_stream z; z.zalloc = Z_NULL; z.zfree = Z_NULL; z.opaque = Z_NULL;
		assert(deflateInit2(&z, Z_BEST_COMPRESSION, Z_DEFLATED, 15, 8, Z_RLE) == Z_OK);
		z.next_in = filtered_stream.data();
		z.avail_in = filtered_stream.size();
		z.next_out = IDATstream.data();
		z.avail_out = IDATstream.size();
		int ret;
		do{
			ret = deflate(&z, Z_FINISH);
			assert(ret == Z_OK || ret == Z_STREAM_END);
		} while(ret != Z_STREAM_END);
		deflateEnd(&z);
		IDATstream.resize(z.total_out);
		return IDATstream;
	}

};

#endif
