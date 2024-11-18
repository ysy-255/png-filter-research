#include <valarray>
#include <utility>

#include "PNG_opt.hpp"
#include "BMP.hpp"
#include "FILE.hpp"
#include "dataset.hpp"

template<typename T>
inline std::valarray<T> vector2valarray(const std::vector<T> & vec){
	return std::valarray<T>(vec.data(), vec.size());
}

template<typename T>
inline T valarray_ave(const std::valarray<T> & arr){
	return arr.sum() / arr.size();
}

std::vector<double> filesizes_normal;
std::vector<double> filesizes_entropy;

void filesize_normal(PNG_opt & png){
	png.filterer();
	filesizes_normal.push_back(png.getFileSize());
}

void filesize_entropy(PNG_opt & png){
	png.filterer_opt();
	filesizes_entropy.push_back(png.getFileSize());
}

void filesize_out(const std::vector<std::pair<std::string, std::vector<double>&>> data, const int cnt){
	int data_size = data.size();
	std::vector<std::valarray<double>> filesizes_arr;
	std::vector<std::valarray<double>> filesizes_ratio;
	for(int i = 0; i < data_size; ++i){
		filesizes_arr.push_back(vector2valarray(data[i].second));
		filesizes_ratio.push_back(filesizes_arr.back() / filesizes_arr.front());
	}

	std::ofstream csv_filesizes("../csv/filesizes/" + std::to_string(cnt) + ".csv");
	csv_filesizes << std::fixed << std::setprecision(4);
	for(uint32_t i = 0; i < data.front().second.size(); ++i){
		csv_filesizes << uint32_t(filesizes_arr[0][i]);
		for(int j = 1; j < data_size; ++j){
			csv_filesizes
			 << ',' << uint32_t(filesizes_arr[j][i])
			 << ',' << filesizes_ratio[j][i];
		}
		csv_filesizes << '\n';
	}
	csv_filesizes.close();

	std::ofstream result_filesizes("../result/filesizes/" + std::to_string(cnt) + ".txt");
	result_filesizes << std::fixed << std::setprecision(4);
	for(int i = 1; i < data_size; ++i){
		std::string line(data[i].first.length() + data[0].first.length() + 9, '-'), line2 = line;
		for(int i = 1; i < int(line2.size()); i += 2) line2[i] = ' ';
		result_filesizes
		 << "\n-- " << data[i].first << " / " << data[0].first << " --\n"
		 << "総圧縮率　: " << filesizes_arr[i].sum() / filesizes_arr[0].sum() << '\n'
		 << "最良圧縮率: " << filesizes_ratio[i].min() << '\n'
		 << "平均圧縮率: " << valarray_ave(filesizes_ratio[i]) << '\n'
		 << "最悪圧縮率: " << filesizes_ratio[i].max() << '\n'
		 << line2 << '\n'
		 << "max: " << uint32_t(filesizes_arr[i].max()) << " / " << uint32_t(filesizes_arr[0].max()) << '\n'
		 << "ave: " << uint32_t(valarray_ave(filesizes_arr[i])) << " / " << uint32_t(valarray_ave(filesizes_arr[0])) << '\n'
		 << "min: " << uint32_t(filesizes_arr[i].min()) << " / " << uint32_t(filesizes_arr[0].min()) << '\n'
		 << "all: " << uint64_t(filesizes_arr[i].sum()) << " / " << uint64_t(filesizes_arr[0].sum()) << '\n'
		 << line << '\n';
	}
	result_filesizes.close();
	return;
}


std::array<std::array<uint32_t, 256>, 256> freq_table_RG_entropy;
std::array<std::array<uint32_t, 256>, 256> freq_table_RB_entropy;
std::array<std::array<uint32_t, 256>, 256> freq_table_GB_entropy;

void RGB_compare(PNG_opt & png){
	png.filterer_opt(1, 4, true, false);
	for(uint32_t h = 0; h < png.Height; ++h){
		for(uint32_t w = 0; w < png.Width; ++w){
			freq_table_RG_entropy[255 - png.R[h][w]][png.G[h][w]] ++;
			freq_table_RB_entropy[255 - png.R[h][w]][png.B[h][w]] ++;
			freq_table_GB_entropy[255 - png.G[h][w]][png.B[h][w]] ++;
		}
	}
}

IMAGE freq2image(const std::array<std::array<uint32_t, 256>, 256> & freq_table){
	IMAGE img(256, 256);
	for(uint16_t y = 0; y < 256; ++y){
		for(uint16_t x = 0; x < 256; ++x){
			uint8_t value = std::min(255, int(log2(freq_table[y][x]) * 10));
			img.R[y][x] = value;
			img.G[y][x] = value;
			img.B[y][x] = value;
		}
	}
	return img;
}

void freq_out(const std::array<std::array<uint32_t, 256>, 256> & freq_table, const std::string & freq_type, const std::string & method, const int cnt){
	std::ofstream out_csv("../csv/" + freq_type + '/' + method + '_' + std::to_string(cnt) + ".csv");
	for(const auto & i : freq_table){
		for(const auto & j : i){
			out_csv << j << ',';
		}
		out_csv << '\n';
	}
	out_csv.close();

	std::string out_img("../result/" + freq_type + '/' + method + '_' + std::to_string(cnt) + ".png");
	PNG(freq2image(freq_table)).write(out_img);
	
	return;
}

int main(){
	void (*funcs[])(PNG_opt &) = {
		&filesize_normal, &filesize_entropy,
		&RGB_compare
	};

	int cnt = 1;
	for(const std::string & dataset : dataset_paths){
		puts(dataset.c_str());

		filesizes_normal.clear();
		filesizes_entropy.clear();

		for(auto & i : freq_table_RG_entropy) i.fill(0);
		for(auto & i : freq_table_RB_entropy) i.fill(0);
		for(auto & i : freq_table_GB_entropy) i.fill(0);


		dataset_allPNG(funcs, 3, dataset);

		filesize_out({
			{"normal", filesizes_normal},
			{"entropy", filesizes_entropy}
		}, cnt);

		freq_out(freq_table_RG_entropy, "RG", "entropy", cnt);
		freq_out(freq_table_RB_entropy, "RB", "entropy", cnt);
		freq_out(freq_table_GB_entropy, "GB", "entropy", cnt);

		cnt ++;
	}
	return 0;
}
