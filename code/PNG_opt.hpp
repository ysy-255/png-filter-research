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
			uint64_t reverse_entoropy_max = 0;
			for(uint8_t i = 0; i < filter_num; ++i){
				if((this->*filter_funcs_opt[i + filter_begin])(h, filtered_array[i])){
					continue;
				}
				uint64_t reverse_entoropy = normalized_reverse_entoropy(filtered_array[i]);
				if(reverse_entoropy > reverse_entoropy_max){
					reverse_entoropy_max = reverse_entoropy;
					filter_methods[h] = i + filter_begin;
				}
			}
			std::vector<uint8_t> & choosed_filtered_array = filtered_array[filter_methods[h] - filter_begin];
			if(change_raw){
				size_t index_t = 1;
				for(uint32_t w = 0; w < Width; ++w){
					ImageData.R[h][w] = choosed_filtered_array[index_t ++];
					ImageData.G[h][w] = choosed_filtered_array[index_t ++];
					ImageData.B[h][w] = choosed_filtered_array[index_t ++];
				}
			}
			if(write_filtered_stream){
				std::copy(choosed_filtered_array.begin(), choosed_filtered_array.end(), &filtered_stream[index]);
			}
			index += channel_num * Width + 1;
		}
	}
	

	private:

	bool (PNG_opt::*filter_funcs_opt[5])(const uint32_t, std::vector<uint8_t> &)
	 = {&PNG_opt::filter_None, &PNG_opt::filter_Sub, &PNG_opt::filter_Up, &PNG_opt::filter_Average, &PNG_opt::filter_Paeth};

	uint64_t normalized_reverse_entoropy(const std::vector<uint8_t> & vec){
		uint32_t cnt[0x100] = {0};
		for(const uint8_t i : vec) cnt[i] ++;
		uint64_t result = 0;
		for(const uint32_t i : cnt){
			if(i == 0) continue;
			result += (uint64_t)i * log2(i);
		}
		return result;
	}

};

#endif
