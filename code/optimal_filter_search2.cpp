#include <fstream>
#include <filesystem>
#include <algorithm>

#include "PNG.hpp"
#include "optimalPNG.hpp"
#include "temp_tests.cpp"

const unsigned char WindowSize_R = 10;
const unsigned char WindowSize_G = 9;
const unsigned char WindowSize_B = 11;


std::vector<std::vector<char>> a_R(0, std::vector<char>(WindowSize_R));
std::vector<std::vector<char>> a_G(0, std::vector<char>(WindowSize_G));
std::vector<std::vector<char>> a_B(0, std::vector<char>(WindowSize_B));


void f(
	unsigned char depth,
	const unsigned char & WindowSize,
	std::vector<std::vector<char>> & a,
	std::vector<char> & vec,
	char num,
	const char & to,
	const char & min
){
	if(depth == WindowSize){
		if(vec[depth - 2] <= to - num){
			vec[depth - 1] = to - num;
			a.push_back(vec);
			return;
		}
	}
	else{
		++ depth;
		for(int add = std::max(vec[depth - 3], char(min - num)); num + add <= to; ++add){
			vec[depth - 2] = add;
			f(depth, WindowSize, a, vec, num + add, to, min);
		}
	}
	return;
}

void f_helper(
	const unsigned char & WindowSize,
	std::vector<std::vector<char>> & a,
	std::vector<std::vector<unsigned char>> & test,
	const char & to,
	const char & min
){
	std::vector<std::vector<char>> a_temp;
	for(char first = 0; first >= min; --first){
		std::vector<char> vec(WindowSize);
		vec[0] = first;
		f(2, WindowSize, a_temp, vec, first, to, min);
	}
	const unsigned int a_temp_size = a_temp.size();
	for(unsigned int i = 0; i < a_temp_size; i++){
		unsigned char flag = 0;
		for(unsigned char w = 0; w < WindowSize; w++){
			flag |= a_temp[i][w];
		}
		if((flag & 1) == 0){
			unsigned char shift = __builtin_ctz(flag);
			for(unsigned char w = 0; w < WindowSize; w++){
				a_temp[i][w] >>= shift;
			}
		}
	}
	unsigned int test_num = test.size();
	for(unsigned int i = 0; i < a_temp_size; i++){
		char a_i_sum = 0;
		for(unsigned char w = 0; w < WindowSize; w++){
			a_i_sum += a_temp[i][w];
		}
		unsigned char a_i_shift = __builtin_ctz(a_i_sum);
		a_temp[i].push_back(a_i_shift);
		do{
			bool flag = true;
			for(int t = 0; t < test_num; t++){
				int predict1 = 0;
				for(int w = 0; w < WindowSize; w++){
					predict1 += a_temp[i][w] * test[t][w];
				}
				predict1 >>= a_i_shift;
				flag &= test[t][WindowSize] <= predict1;
				flag &= test[t][WindowSize + 1] >= predict1;
				if(!flag) break;
			}
			if(flag) a.push_back(a_temp[i]);
		}while(std::next_permutation(a_temp[i].begin(), a_temp[i].end() - 1));
	}
	return;
}

std::vector<unsigned int> chooser(std::vector<std::vector<long long>> & vec, const int time){
	const int height = vec.size();
	const int width = vec.back().size();
	#define V(T) std::vector<T>
	#define VVV(T, z, y, x, v, name) V(V(V(T))) name(z, V(V(T))(y, V(T)(x, v)));
	VVV(long long, time + 1, height, width, 0, dp);
	char change[time][height] = {'y'};
	
	for(int t = 1; t <= time; t++){
		for(int h = 0; h < height; h++){
			for(int w = 0; w < width; w++){
				dp[t][h][w] = std::max(dp[t - 1][h][w], vec[h][w]);
			}
			if(h > 0){
				long long asum = 0, bsum = 0;
				for(int w = 0; w < width; w++){
					asum += dp[t][h - 1][w];
					bsum += dp[t][h][w];
				}
				if(asum > bsum){
					for(int w = 0; w < width; w++){
						dp[t][h][w] = dp[t][h - 1][w];
					}
					change[t - 1][h] = 'n';
				}
			}
		}
	}
	std::vector<unsigned int> result(time);
	for(int t = time - 1, h = height - 1; t >= 0 && h >= 0; t--){
		while(change[t][h] == 'n') h--;
		result[t] = h;
	}
	return result;
}

void worker(std::vector<IMAGE> & images, std::string outPath){
	int a_R_size = a_R.size();
	int a_G_size = a_G.size();
	int a_B_size = a_B.size();
	std::vector<std::vector<long long>> rate_R(a_R_size, std::vector<long long>(0));
	std::vector<std::vector<long long>> rate_G(a_G_size, std::vector<long long>(0));
	std::vector<std::vector<long long>> rate_B(a_B_size, std::vector<long long>(0));
	unsigned int image_num = images.size();
	for(int i = 0; i < image_num; i++){
		std::cerr << i << "% ";
		int height = images[i].height;
		int width = images[i].width;
		int dh = height / 10 + 1;
		for(int h = 1; h < height; h += dh){
			std::vector<std::vector<unsigned char>> diff_R(a_R_size, std::vector<unsigned char>(width - 1));
			std::vector<std::vector<unsigned char>> diff_G(a_G_size, std::vector<unsigned char>(width - 1));
			std::vector<std::vector<unsigned char>> diff_B(a_B_size, std::vector<unsigned char>(width - 1));
			for(int w = 1; w < width; w++){
				for(int n = 0; n < a_R_size; n++){
					int r1 = images[i].R[h-1][w-1] * a_R[n][0];
					int r2 = images[i].R[h-1][ w ] * a_R[n][1];
					int r3 = images[i].R[ h ][w-1] * a_R[n][2];
					int g1 = images[i].G[h-1][w-1] * a_R[n][3];
					int g2 = images[i].G[h-1][ w ] * a_R[n][4];
					int g3 = images[i].G[ h ][w-1] * a_R[n][5];
					int g4 = images[i].G[ h ][ w ] * a_R[n][6];
					int b1 = images[i].B[h-1][w-1] * a_R[n][7];
					int b2 = images[i].B[h-1][ w ] * a_R[n][8];
					int b3 = images[i].B[ h ][w-1] * a_R[n][9];
					int R_hat = (r1 + r2 + r3 + g1 + g2 + g3 + g4 + b1 + b2 + b3) >> a_R[n][10];
					diff_R[n][w - 1] = images[i].R[h][w] - R_hat;
				}
				for(int n = 0; n < a_G_size; n++){
					int r1 = images[i].R[h-1][w-1] * a_G[n][0];
					int r2 = images[i].R[h-1][ w ] * a_G[n][1];
					int r3 = images[i].R[ h ][w-1] * a_G[n][2];
					int g1 = images[i].G[h-1][w-1] * a_G[n][3];
					int g2 = images[i].G[h-1][ w ] * a_G[n][4];
					int g3 = images[i].G[ h ][w-1] * a_G[n][5];
					int b1 = images[i].B[h-1][w-1] * a_G[n][6];
					int b2 = images[i].B[h-1][ w ] * a_G[n][7];
					int b3 = images[i].B[ h ][w-1] * a_G[n][8];
					int G_hat = (r1 + r2 + r3 + g1 + g2 + g3 + b1 + b2 + b3) >> a_G[n][9];
					diff_G[n][w - 1] = images[i].G[h][w] - G_hat;
				}
				for(int n = 0; n < a_B_size; n++){
					int r1 = images[i].R[h-1][w-1] * a_B[n][0];
					int r2 = images[i].R[h-1][ w ] * a_B[n][1];
					int r3 = images[i].R[ h ][w-1] * a_B[n][2];
					int r4 = images[i].R[ h ][ w ] * a_B[n][3];
					int g1 = images[i].G[h-1][w-1] * a_B[n][4];
					int g2 = images[i].G[h-1][ w ] * a_B[n][5];
					int g3 = images[i].G[ h ][w-1] * a_B[n][6];
					int g4 = images[i].G[ h ][ w ] * a_B[n][7];
					int b1 = images[i].B[h-1][w-1] * a_B[n][8];
					int b2 = images[i].B[h-1][ w ] * a_B[n][9];
					int b3 = images[i].B[ h ][w-1] * a_B[n][10];
					int B_hat = (r1 + r2 + r3 + r4 + g1 + g2 + g3 + g4 + b1 + b2 + b3) >> a_B[n][11];
					diff_B[n][w - 1] = images[i].B[h][w] - B_hat;
				}
			}
			for(int n = 0; n < a_R_size; n++){
				rate_R[n].push_back(myratePredictor(diff_R[n]));
			}
			for(int n = 0; n < a_G_size; n++){
				rate_G[n].push_back(myratePredictor(diff_G[n]));
			}
			for(int n = 0; n < a_B_size; n++){
				rate_B[n].push_back(myratePredictor(diff_B[n]));
			}
		}
	}
	std::ofstream out_R(outPath + "_R" + ".csv");
	std::ofstream out_G(outPath + "_G" + ".csv");
	std::ofstream out_B(outPath + "_B" + ".csv");
	const int time = 4;
	std::vector<unsigned int> dp_R = chooser(rate_R, time);
	std::vector<unsigned int> dp_G = chooser(rate_G, time);
	std::vector<unsigned int> dp_B = chooser(rate_B, time);
	for(int t = 0; t < time; t++){
		out_R << int(a_R[dp_R[t]][WindowSize_R]) << ' ';
		out_G << int(a_G[dp_G[t]][WindowSize_G]) << ' ';
		out_B << int(a_B[dp_B[t]][WindowSize_B]) << ' ';
		for(int n = 0; n < WindowSize_R; n++){
			out_R << int(a_R[dp_R[t]][n]) << ' ';
		}
		out_R << std::endl;
		for(int n = 0; n < WindowSize_G; n++){
			out_G << int(a_G[dp_G[t]][n]) << ' ';
		}
		out_G << std::endl;
		for(int n = 0; n < WindowSize_B; n++){
			out_B << int(a_B[dp_B[t]][n]) << ' ';
		}
		out_B << std::endl;
	}
	std::cout << std::endl;
	return;
}

int main(){
	f_helper(WindowSize_R, a_R, test_R, 4, -3);
	printf("%d\n", int(a_R.size()));
	f_helper(WindowSize_G, a_G, test_G, 4, -3);
	printf("%d\n", int(a_G.size()));
	f_helper(WindowSize_B, a_B, test_B, 4, -3);
	printf("%d\n", int(a_B.size()));
	std::vector<IMAGE> images = {}; int count = 0; // スクリーンショット10枚
	auto directry = std::filesystem::directory_iterator("/mnt/c/test images/screenshot/"); // (Cドライブ上のスクリーンショット)
	for(auto & i : directry){
		PNG png;
		readPNG(i.path().string(), png);
		images.push_back(png.ImageData);
		if(++count == 100)break;
	}
	count = 0;
	for(int l = 1; l <= 100; l++){
		std::string S = std::to_string(l);
		while(S.size() < 4)S = '0' + S;
		PNG png;
		readPNG("../../../../mnt/c/test images/X4/" + S + "x4.png", png);
		images.push_back(png.ImageData);
		if(++count == 100)break;
	}
	worker(images, "../out/filter");
	return 0;
}