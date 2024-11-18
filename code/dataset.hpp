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
	// CG画像 スクリーンショット
	"/mnt/c/test images/afreightdata",
		// https://homepages.inf.ed.ac.uk/amos/afreightdata.html [afreightim001.png - afreightim455.png]
	"/mnt/c/test images/screenshot",
		// my random 100 screenshots (including various sizes)

	// ピクトアーツさんの画像も使おうかと思っています  https://pictarts.com/



	// 自然画像(の保存にPNGが使われることはあまりないのでおまけみたいなものかもしれません)
	"/mnt/c/test images/C00C00",
		// https://testimages.org/sampling/ -> 8BIT -> RGB -> 1200x1200 -> C00C00 (all)
	"/mnt/c/test images/X4"
		// https://data.vision.ee.ethz.ch/cvl/DIV2K/ -> DIV2K/DIV2K_train_LR_bicubic/X4/ [0001x4.png - 0200x4.png]
};

#endif
