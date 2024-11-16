#include "PNG_opt.hpp"
#include "BMP.hpp"
#include "FILE.hpp"
#include "dataset.hpp"

std::vector<uint32_t> PNGsizes_entoropy_off;
std::vector<uint32_t> PNGsizes_entoropy_on;

void entoropy_on(PNG_opt & png){
	png.filterer();
	PNGsizes_entoropy_off.push_back(png.getFileSize());
}

void entoropy_off(PNG_opt & png){
	png.filterer_opt();
	PNGsizes_entoropy_on.push_back(png.getFileSize());
}

std::array<std::array<uint32_t, 256>, 256> RG_freque;
std::array<std::array<uint32_t, 256>, 256> RB_freque;
std::array<std::array<uint32_t, 256>, 256> GB_freque;

void RGB_compare(PNG_opt & png){
	png.filterer_opt(true, false);
	for(uint32_t h = 0; h < png.Height; ++h){
		for(uint32_t w = 0; w < png.Width; ++w){
			RG_freque[255 - png.ImageData.R[h][w]][png.ImageData.G[h][w]] ++;
			RB_freque[255 - png.ImageData.R[h][w]][png.ImageData.B[h][w]] ++;
			GB_freque[255 - png.ImageData.G[h][w]][png.ImageData.B[h][w]] ++;
		}
	}
}

IMAGE RGB_freque_image(const std::array<std::array<uint32_t, 256>, 256> & freque){
	IMAGE img(256, 256);
	for(uint16_t y = 0; y < 256; ++y){
		for(uint16_t x = 0; x < 256; ++x){
			uint8_t value = std::min(255, int(log2(freque[y][x]) * 10));
			img.R[y][x] = value;
			img.G[y][x] = value;
			img.B[y][x] = value;
		}
	}
	return img;
}

int main(){
	void (*funcs[])(PNG_opt &) = {
		// &entoropy_off, &entoropy_on,
		&RGB_compare
	};

	int cnt = 1;
	for(const std::string & dataset : dataset_paths){
		puts(dataset.c_str());

		// PNGsizes_entoropy_off.clear();
		// PNGsizes_entoropy_on.clear();

		for(auto & i : RG_freque) i.fill(0);
		for(auto & i : RB_freque) i.fill(0);
		for(auto & i : GB_freque) i.fill(0);


		dataset_allPNG(funcs, 1, dataset);

		/*
		std::ofstream out_entoropy("../csv/entoropy_" + std::to_string(cnt) + ".csv");
		for(uint32_t i = 0; i < PNGsizes_entoropy_off.size(); ++i){
			out_entoropy << PNGsizes_entoropy_off[i] << ","
			 << PNGsizes_entoropy_on[i] << std::endl;
		}
		out_entoropy.close();
		*/

		/*
		std::ofstream out_RG("../csv/RG_" + std::to_string(cnt) + ".csv");
		for(auto & i : RG_freque){
			for(auto & j : i){
				out_RG << j << ",";
			}
			out_RG << std::endl;
		}
		out_RG.close();
		std::ofstream out_RB("../csv/RB_" + std::to_string(cnt) + ".csv");
		for(auto & i : RB_freque){
			for(auto & j : i){
				out_RB << j << ",";
			}
			out_RB << std::endl;
		}
		out_RB.close();
		std::ofstream out_GB("../csv/GB_" + std::to_string(cnt) + ".csv");
		for(auto & i : GB_freque){
			for(auto & j : i){
				out_GB << j << ",";
			}
			out_GB << std::endl;
		}
		out_GB.close();
		*/
		PNG(RGB_freque_image(RG_freque)).write("../out/RG_freque_" + std::to_string(cnt) + ".png");
		PNG(RGB_freque_image(RB_freque)).write("../out/RB_freque_" + std::to_string(cnt) + ".png");
		PNG(RGB_freque_image(GB_freque)).write("../out/GB_freque_" + std::to_string(cnt) + ".png");


		cnt ++;
	}
	return 0;
}
