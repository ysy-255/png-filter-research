#include <fstream>
#include <vector>
#include <iostream>
#include <filesystem>

#include "PNG.hpp"
#include "optimalPNG.hpp"

unsigned char withinU8(const int value){
	return (unsigned char) (value < 0 ? 0 : value > 255 ? 255 : value);
}

int filters[4 + 4 + 10 + 10][7] = {
	{0,  0,  0,  0,  0,  0,  0}, // none
	{0,  1,  0,  0,  0,  0,  0}, // sub
	{0,  0,  1,  0,  0,  0,  0}, // up
	{1,  1,  1,  0,  0,  0,  0}, // average

	{0,  0,  0,  0,  1,  0,  0}, // 5
	{0,  1,  1,  0, -1,  0,  0}, // 6
	{1,  2,  1,  0, -1,  0,  0}, // 7
	{1,  1,  2,  0, -1,  0,  0}, // 8
	// ここまで lossless JPEG
	

	{0,  1,  1,  0, -1,  0,  0},
	{2,  3,  3,  0, -2,  0,  0},
	{2,  2,  2,  0,  0, -1,  1},
	{0,  0,  1,  0,  0,  0,  0},
	{1,  2,  1,  0, -1,  0,  0},
	{2,  4,  1, -1, -1,  0,  1},
	{0,  1,  0,  0,  0,  0,  0},
	{2,  3,  3,  0, -2, -1,  1},
	{2,  2,  2,  0, -1,  0,  1},
	{2,  3,  2,  0, -1,  0,  0},
	{2,  3,  2, -1,  0, -1,  1},
	{2,  4,  2, -1, -1,  0,  0}
};

int filters_R[4][11] = {
	{0,  0,  0,  1,  0,  0, -1,  1,  0,  0,  0},
	{1,  0,  1,  1,  0, -1, -1,  2,  0,  0,  0},
	{2, -3,  4,  3,  0,  0,  0,  0,  0,  0,  0},
	{2,  0,  1,  3,  0, -1, -3,  4,  0,  0,  0}
};

int filters_G[4][10] = {
	{2,  0,  0,  0, -2,  3,  3,  0,  0,  0},
	{2, -1,  0,  1, -2,  4,  2, -1,  0,  1},
	{0,  0,  0,  0,  0,  1,  0,  0,  0,  0},
	{2,  0,  0,  0, -1,  2,  3,  0,  0,  0}
};

int filters_B[4][12] = {
	{2,  0,  0,  0,  0,  0, -3, -1,  4,  0,  3,  1},
	{2,  0,  0,  0,  0,  0, -2, -1,  3,  0,  3,  1},
	{2,  1, -1,  0,  0, -1,  1, -2,  2,  0,  0,  4},
	{2,  0, -1,  0,  1,  0, -1,  0,  1, -2,  4,  2}
};


bool UseEntropy_rate = false;
bool UseJPEGlossless = false;
bool UseAddFilters = false;
bool UseGx_diff = false;
bool UseRGBFilters = false;
int Filter_num;

std::vector<unsigned char> mychooser(const IMAGE & data, const int h){
	int Filter_num = 5;
	int H = data.height;
	int W = data.width;
	if(UseJPEGlossless) Filter_num += 4;
	if(UseAddFilters) Filter_num += 12;
	std::vector<std::vector<unsigned char>> results(Filter_num + UseRGBFilters, std::vector<unsigned char>(W * 3 + 1));
	std::vector<std::vector<unsigned char>> results_R(4, std::vector<unsigned char>(W));
	std::vector<std::vector<unsigned char>> results_G(4, std::vector<unsigned char>(W));
	std::vector<std::vector<unsigned char>> results_B(4, std::vector<unsigned char>(W));
	for(int i = 0; i < Filter_num; i++) results[i][0] = i;
	unsigned long long predictedRate[Filter_num + UseRGBFilters];
	struct pixels{
		                        int uu = 0;
		            int ul = 0; int  u = 0; int ur = 0;
		int ll = 0; int  l = 0; int  x = 0;
	};
	pixels R, G, B;
	if(h > 0){
		R.u = data.R[h - 1][0];
		G.u = data.G[h - 1][0];
		B.u = data.B[h - 1][0];
	}
	for(int w = 0; w < W; w++){
		if(h >= 2){
			R.uu = data.R[h - 2][w];
			G.uu = data.G[h - 2][w];
			B.uu = data.B[h - 2][w];
		}
		if(h >= 1 && w < W - 1){
			R.ur = data.R[h - 1][w + 1];
			G.ur = data.G[h - 1][w + 1];
			B.ur = data.B[h - 1][w + 1];
		}
		R.x = data.R[h][w];
		G.x = data.G[h][w];
		B.x = data.B[h][w];
		unsigned int offset = 0;
		std::vector<unsigned char> Rx_hat(Filter_num);
		std::vector<unsigned char> Gx_hat(Filter_num);
		std::vector<unsigned char> Bx_hat(Filter_num);
		for(int Type = 0; Type < 4; ++Type, ++offset){
			int Rpa1 = R.l * filters[Type][1], Gpa1 = G.l * filters[Type][1], Bpa1 = B.l * filters[Type][1],
			    Rpa2 = R.u * filters[Type][2], Gpa2 = G.u * filters[Type][2], Bpa2 = B.u * filters[Type][2];
			Rx_hat[offset] = withinU8((Rpa1 + Rpa2) >> filters[Type][0]);
			Gx_hat[offset] = withinU8((Gpa1 + Gpa2) >> filters[Type][0]);
			Bx_hat[offset] = withinU8((Bpa1 + Bpa2) >> filters[Type][0]);
		}
		Rx_hat[offset] = PaethPredictor(R.l, R.u, R.ul);
		Gx_hat[offset] = PaethPredictor(G.l, G.u, G.ul);
		Bx_hat[offset] = PaethPredictor(B.l, B.u, B.ul);
		offset ++;
		if(UseJPEGlossless){
			for(int Type = 4; Type < 8; ++Type, ++offset){
				int Rpa1 = R.l  * filters[Type][1], Gpa1 = G.l  * filters[Type][1], Bpa1 = B.l  * filters[Type][1],
				    Rpa2 = R.u  * filters[Type][2], Gpa2 = G.u  * filters[Type][2], Bpa2 = B.u  * filters[Type][2],
				    Rpa4 = R.ul * filters[Type][4], Gpa4 = G.ul * filters[Type][4], Bpa4 = B.ul * filters[Type][4];
				Rx_hat[offset] = withinU8((Rpa1 + Rpa2 + Rpa4) >> filters[Type][0]);
				Gx_hat[offset] = withinU8((Gpa1 + Gpa2 + Gpa4) >> filters[Type][0]);
				Bx_hat[offset] = withinU8((Bpa1 + Bpa2 + Bpa4) >> filters[Type][0]);
			}
		}
		if(UseAddFilters){
			for(int Type = 8; Type < 20; ++Type, ++offset){
				int Rpa1 = R.l  * filters[Type][1], Gpa1 = G.l  * filters[Type][1], Bpa1 = B.l  * filters[Type][1],
				    Rpa2 = R.u  * filters[Type][2], Gpa2 = G.u  * filters[Type][2], Bpa2 = B.u  * filters[Type][2],
				    Rpa3 = R.ll * filters[Type][3], Gpa3 = G.ll * filters[Type][3], Bpa3 = B.ll * filters[Type][3],
				    Rpa4 = R.ul * filters[Type][4], Gpa4 = G.ul * filters[Type][4], Bpa4 = B.ul * filters[Type][4],
				    Rpa5 = R.uu * filters[Type][5], Gpa5 = G.uu * filters[Type][5], Bpa5 = B.uu * filters[Type][5],
				    Rpa6 = R.ur * filters[Type][6], Gpa6 = G.ur * filters[Type][6], Bpa6 = B.ur * filters[Type][6];
				Rx_hat[offset] = withinU8((Rpa1 + Rpa2 + Rpa3 + Rpa4 + Rpa5 + Rpa6) >> filters[Type][0]);
				Gx_hat[offset] = withinU8((Gpa1 + Gpa2 + Gpa3 + Gpa4 + Gpa5 + Gpa6) >> filters[Type][0]);
				Bx_hat[offset] = withinU8((Bpa1 + Bpa2 + Bpa3 + Bpa4 + Bpa5 + Bpa6) >> filters[Type][0]);
			}
		}
		if(UseGx_diff){
			for(offset = 0; offset < Filter_num; ++offset){
				int Gx_def = G.x - Gx_hat[offset];
				Rx_hat[offset] = withinU8(Rx_hat[offset] + Gx_def);
				Bx_hat[offset] = withinU8(Bx_hat[offset] + Gx_def);
			}
		}
		for(offset = 0; offset < Filter_num; ++offset){
			results[offset][w * 3 + 1] = R.x - Rx_hat[offset];
			results[offset][w * 3 + 2] = G.x - Gx_hat[offset];
			results[offset][w * 3 + 3] = B.x - Bx_hat[offset];
		}
		if(UseRGBFilters){
			for(int Type = 0; Type < 4; ++Type){
				results_R[Type][w] = (unsigned char)R.x - withinU8((
					R.ul * filters_R[Type][1] +
				    R.u  * filters_R[Type][2] +
				    R.l  * filters_R[Type][3] +
					G.ul * filters_R[Type][4] +
					G.u  * filters_R[Type][5] +
					G.l  * filters_R[Type][6] +
					G.x  * filters_R[Type][7] +
					B.ul * filters_R[Type][8] +
					B.u  * filters_R[Type][9] +
					B.l  * filters_R[Type][10]) >> filters_R[Type][0]);
				results_G[Type][w] = (unsigned char)G.x - withinU8((
					R.ul * filters_G[Type][1] +
				    R.u  * filters_G[Type][2] +
				    R.l  * filters_G[Type][3] +
					G.ul * filters_G[Type][4] +
					G.u  * filters_G[Type][5] +
					G.l  * filters_G[Type][6] +
					B.ul * filters_G[Type][7] +
					B.u  * filters_G[Type][8] +
					B.l  * filters_G[Type][9]) >> filters_G[Type][0]);
				results_B[Type][w] = (unsigned char)B.x - withinU8((
					R.ul * filters_B[Type][1] +
				    R.u  * filters_B[Type][2] +
				    R.l  * filters_B[Type][3] +
					R.x  * filters_B[Type][4] +
					G.ul * filters_B[Type][5] +
					G.u  * filters_B[Type][6] +
					G.l  * filters_B[Type][7] +
					G.x  * filters_B[Type][8] +
					B.ul * filters_B[Type][9] +
					B.u  * filters_B[Type][10] +
					B.l  * filters_B[Type][11]) >> filters_B[Type][0]);
			}
		}
		R.ll = R.l , G.ll = G.l , B.ll = B.l ;
		R.l  = R.x , G.l  = G.x , B.l  = B.x ;
		R.ul = R.u , G.ul = G.u , B.ul = B.u ;
		R.u  = R.ur, G.u  = G.ur, B.u  = B.ur;
	}
	if(UseRGBFilters){
		unsigned long long R_value = 0, G_value = 0, B_value = 0;
		unsigned char R_element = 0, G_element = 0, B_element = 0;
		for(int Type = 0; Type < 4; ++Type){
			long long R_rate = tilde_entropy_rate(results_R[Type]);
			long long G_rate = tilde_entropy_rate(results_G[Type]);
			long long B_rate = tilde_entropy_rate(results_B[Type]);
			if(R_rate > R_value) R_value = R_rate, R_element = Type;
			if(G_rate > G_value) G_value = G_rate, G_element = Type;
			if(B_rate > B_value) B_value = B_rate, B_element = Type;
		}
		results[Filter_num][0] = (R_element + (G_element << 2) + (B_element << 4)) + Filter_num;
		for(int w = 0; w < W; ++w){
			results[Filter_num][w * 3 + 1] = results_R[R_element][w];
			results[Filter_num][w * 3 + 2] = results_G[G_element][w];
			results[Filter_num][w * 3 + 3] = results_B[B_element][w];
		}
		Filter_num ++;
	}
	unsigned long long max_value = 0;
	unsigned char max_element = 0;
	if(UseEntropy_rate){
		for(int i = 0; i < Filter_num; i++){
			predictedRate[i] = tilde_entropy_rate(results[i]);
			// std::cout << i << ": " << predictedRate[i] << ", ";
			if(predictedRate[i] > max_value){
				max_value = predictedRate[i];
				max_element = i;
			}
		}
	}
	else{
		max_value = LONG_LONG_MAX;
		for(int i = 0; i < Filter_num; i++){
			predictedRate[i] = ratePredictor(results[i]);
			// std::cout << i << ": " << predictedRate[i] << ", ";
			if(predictedRate[i] < max_value){
				max_value = predictedRate[i];
				max_element = i;
			}
		}
	}
	return results[max_element];
}

std::vector<unsigned char> myfilterer(PNG & data, bool change){
	unsigned int streamWidth = data.Width * 3 + 1;
	std::vector<unsigned char> datastream(streamWidth * data.Height);
	for(unsigned int h = 0; h < data.Height; h++){
		data.ImageData.R[h].push_back(0);
		data.ImageData.G[h].push_back(0);
		data.ImageData.B[h].push_back(0);
		std::memcpy(datastream.data() + streamWidth * h, mychooser(data.ImageData, h).data(), streamWidth);
		data.methods[h] = datastream[streamWidth * h];
	}
	if(change){
		unsigned int offset = 0;
		for(unsigned int h = 0; h < data.Height; h++){
			++offset;
			for(unsigned int w = 0; w < data.Width; w++){
				data.ImageData.R[h][w] = datastream[offset ++];
				data.ImageData.G[h][w] = datastream[offset ++];
				data.ImageData.B[h][w] = datastream[offset ++];
			}
		}
	}
	return datastream;
}

int main(){
	std::ofstream result("result.csv");
	for(int i = 100; i < 200; i++){
		PNG image;
		std::string path = std::to_string(i);
		while(path.size() < 4)path = '0' + path;
		path = "../../../../mnt/c/test images/X4/" + path + "x4.png";
		readPNG(path, image); // ここに画像のパス
		for(int n = 0; n < 8; n++){
			UseEntropy_rate = ((n & 1) > 0);
			UseJPEGlossless = ((n & 2) > 0);
			UseGx_diff = ((n & 4) > 0);
			std::vector<unsigned char> datastream = myfilterer(image, false);
			result << getPNGsize(image.Width, image.Height, datastream) << ',';
		}
		result << '\n';
		std::cerr << float(i - 100) / 2  << "% ";
	}
	int count = 0;
	auto directry = std::filesystem::directory_iterator("/mnt/c/test images/screenshot/");
	for(auto & i : directry){
		if(++count % 5 == 0){
			PNG image;
			readPNG(i.path().string(), image);
			for(int n = 0; n < 8; n++){
				UseEntropy_rate = ((n & 1) > 0);
				UseJPEGlossless = ((n & 2) > 0);
				UseGx_diff = ((n & 4) > 0);
				std::vector<unsigned char> datastream = myfilterer(image, false);
				result << getPNGsize(image.Width, image.Height, datastream) << ',';
			}
			result << '\n';
			std::cerr << float(count / 5) / 2 + 50.0 << "% ";
		}
		if(count == 500) break;
	}
}