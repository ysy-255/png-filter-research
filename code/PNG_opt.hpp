#ifndef PNG_opt_H
#define PNG_opt_H

#include "PNG.hpp"
#include <math.h>

class PNG_opt : public PNG{
	public:

	PNG_opt(){};
	PNG_opt(const std::string & path) {read(path);}

	inline const std::vector<uint32_t> filter_methods_cnt(){
		std::vector<uint32_t> result(5);
		for(auto & i : filter_methods){
			result[i] ++;
		}
		return result;
	}

	size_t getFileSize(){
		size_t result = 0;
		result += 8;
		result += 4 + PNG_IHDR_SIZE + 4;
		result += 4 + filtered_stream_Deflate().size() + 4;
		result += 4 + PNG_IEND_SIZE + 4;
		return result;
	}

	void filterer_opt(const uint8_t filter_begin = 1, const uint8_t filter_end = 5, const bool change_raw = false, const bool write_filtered_stream = true){
		const uint8_t filter_num = filter_end - filter_begin;
		size_t index = 0;
		if(write_filtered_stream){
			filtered_stream.resize((channel_num * Width + 1) * Height);
		}
		// Filter_method = 1;
		filter_methods.resize(Height);
		std::vector<uint8_t> filtered_array[filter_num];
		for(uint8_t i = 0; i < filter_num; ++i){
			filtered_array[i].resize(Width * 3 + 1);
		}
		for(uint32_t h = 0; h < Height; ++h){
			int64_t entropy_min = INT64_MAX;
			for(uint8_t i = 0; i < filter_num; ++i){
				if((this->*f_funcs_opt[i + filter_begin])(h, filtered_array[i])){
					continue;
				}
				int64_t entropy = simplified_entropy(filtered_array[i]);
				if(entropy < entropy_min){
					entropy_min = entropy;
					filter_methods[h] = i + filter_begin;
				}
			}
			std::vector<uint8_t> & choosed_filtered_array = filtered_array[filter_methods[h] - filter_begin];
			if(change_raw){
				size_t index_t = 1;
				for(uint32_t w = 0; w < Width; ++w){
					R[h][w] = choosed_filtered_array[index_t ++];
					G[h][w] = choosed_filtered_array[index_t ++];
					B[h][w] = choosed_filtered_array[index_t ++];
				}
			}
			if(write_filtered_stream){
				std::copy(choosed_filtered_array.begin(), choosed_filtered_array.end(), &filtered_stream[index]);
			}
			index += channel_num * Width + 1;
		}
	}
	

	private:

	bool f_N_RGBdecorr_a(const uint32_t h, std::vector<uint8_t> & filtered){
		size_t index = 0;
		filtered[index ++] = 5;
		uint8_t G_diff;
		for(uint32_t w = 0; w < Width; ++w){
			G_diff = G[h][w];
			filtered[index ++] = R[h][w] - G_diff;
			filtered[index ++] = G_diff;
			filtered[index ++] = B[h][w] - G_diff;
		}
		return false;
	}
	bool f_S_RGBdecorr_a(const uint32_t h, std::vector<uint8_t> & filtered){
		size_t index = 0;
		filtered[index ++] = 6;
		uint8_t G_diff = G[h][0];
		filtered[index ++] = R[h][0] - G_diff;
		filtered[index ++] = G_diff;
		filtered[index ++] = B[h][0] - G_diff;
		for(uint32_t w = 1; w < Width; ++w){
			G_diff = G[h][w] - G[h][w - 1];
			filtered[index ++] = R[h][w] - R[h][w - 1] - G_diff;
			filtered[index ++] = G_diff;
			filtered[index ++] = B[h][w] - B[h][w - 1] - G_diff;
		}
		return false;
	}
	bool f_U_RGBdecorr_a(const uint32_t h, std::vector<uint8_t> & filtered){
		if(h == 0) return true;
		size_t index = 0;
		filtered[index ++] = 7;
		uint8_t G_diff;
		for(uint32_t w = 0; w < Width; ++w){
			G_diff = G[h][w] - G[h - 1][w];
			filtered[index ++] = R[h][w] - R[h - 1][w] - G_diff;
			filtered[index ++] = G_diff;
			filtered[index ++] = B[h][w] - B[h - 1][w] - G_diff;
		}
		return false;
	}
	bool f_A_RGBdecorr_a(const uint32_t h, std::vector<uint8_t> & filtered){
		if(h == 0) return true;
		size_t index = 0;
		filtered[index ++] = 8;
		uint8_t G_diff = G[h][0] - (G[h - 1][0] >> 1);
		filtered[index ++] = R[h][0] - (R[h - 1][0] >> 1) - G_diff;
		filtered[index ++] = G_diff;
		filtered[index ++] = B[h][0] - (B[h - 1][0] >> 1) - G_diff;
		for(uint32_t w = 1; w < Width; ++w){
			G_diff = G[h][w] - ((G[h][w - 1] + G[h - 1][w]) >> 1);
			filtered[index ++] = R[h][w] - ((R[h][w - 1] + R[h - 1][w]) >> 1) - G_diff;
			filtered[index ++] = G_diff;
			filtered[index ++] = B[h][w] - ((B[h][w - 1] + B[h - 1][w]) >> 1) - G_diff;
		}
		return false;
	}
	bool f_P_RGBdecorr_a(const uint32_t h, std::vector<uint8_t> & filtered){
		if(h == 0) return true;
		size_t index = 0;
		filtered[index ++] = 9;
		uint8_t G_diff = G[h][0] - paeth_predictor(0, G[h - 1][0], 0);
		filtered[index ++] = R[h][0] - paeth_predictor(0, R[h - 1][0], 0) - G_diff;
		filtered[index ++] = G_diff;
		filtered[index ++] = B[h][0] - paeth_predictor(0, B[h - 1][0], 0) - G_diff;
		for(uint32_t w = 1; w < Width; ++w){
			G_diff = G[h][w] - paeth_predictor(G[h][w - 1], G[h - 1][w], G[h - 1][w - 1]);
			filtered[index ++] = R[h][w] - paeth_predictor(R[h][w - 1], R[h - 1][w], R[h - 1][w - 1]) - G_diff;
			filtered[index ++] = G_diff;
			filtered[index ++] = B[h][w] - paeth_predictor(B[h][w - 1], B[h - 1][w], B[h - 1][w - 1]) - G_diff;
		}
		return false;
	}

	bool (PNG_opt::*f_funcs_opt[10])(const uint32_t, std::vector<uint8_t> &)
	 = {&PNG_opt::f_None, &PNG_opt::f_Sub, &PNG_opt::f_Up, &PNG_opt::f_Ave, &PNG_opt::f_Paeth,
	    &PNG_opt::f_N_RGBdecorr_a, &PNG_opt::f_S_RGBdecorr_a, &PNG_opt::f_U_RGBdecorr_a, &PNG_opt::f_A_RGBdecorr_a, &PNG_opt::f_P_RGBdecorr_a};

	int64_t simplified_entropy(const std::vector<uint8_t> & vec){
		uint32_t cnt[0x100] = {0};
		for(const uint8_t i : vec) cnt[i] ++;
		double result = 0;
		for(const double i : cnt){
			if(i == 0) continue;
			result += i * log2(i);
		}
		return -result;
	}

};

#endif
