#ifndef dataset_H
#define dataset_H

#include <filesystem>

#include "PNG_opt.hpp"
#include "FILE.hpp"

bool dataset_allPNG(void (*funcs[])(PNG_opt &), const uint32_t func_num, std::string dataset_path){
	auto dataset = get_allfile(dataset_path);
	for(auto & x : dataset){
		PNG_opt png;
		if(png.read(x)) continue;
		else{
			for(uint32_t i = 0; i < func_num; ++i){
				PNG_opt png_t = png;
				(funcs[i])(png);
			}
		}
	}
	return 0;
}

std::vector<std::string> dataset_paths = {
	"/mnt/c/test images/afreightdata",
	"/mnt/c/test images/C00C00",
	"/mnt/c/test images/screenshot",
	"/mnt/c/test images/X4"
};

#endif
