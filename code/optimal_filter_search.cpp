#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <filesystem>

#include "PNG.hpp"
#include "optimalPNG.hpp"

std::vector<std::vector<int>> a;


// ベクトル列の集合を各列が重複しないよう作成する
void f(int depth, int m /*マイナスのベクトルの合計*/, int p /*最終的な合計*/, int now, std::vector<int> vec, bool tom/* マイナスのベクトルを追加するとき */){
	// 再帰の終了条件
	if(depth == 1){
		// 残りの数が最後の要素以上なら追加
		if(p - now >= vec.back()){
			vec.push_back(p - now);
			std::sort(vec.begin() + 1, vec.end()); // 後に辞書順全探索するためにソート

			// 全て2で割りきれるものは追加しない
			bool flag = false;
			for(int i : vec) flag |= (i % 2 != 0);
			if(!flag) return;

			a.push_back(vec);
		}
		return;
	}

	// プラスのベクトルのみでフィルター列を作成するとき
	if(m == 0){
		// 最後の要素以上の数を追加し再帰処理
		for(int i = vec.back(); i + now < p; i++){
			vec.push_back(i);
			f(depth - 1, m, p, now + i, vec, false);
			vec.pop_back();
		}
		return;
	}
	// マイナスのベクトルを追加するとき
	if(tom){
		if(vec.back() < m - now) return; // これ以上マイナスのベクトルを追加できない
		if(depth == 2){
			vec.push_back(m - now); // 次にプラスのベクトルを追加して終わらなければならないので残りの数を追加
			f(depth - 1, m, p, m, vec, false); // 追加するベクトルをプラスに変更して再帰
		}
		else{
			// マイナスのベクトルを追加する再帰
			for(int i = vec.back(); i >= m - now; i--){
				vec.push_back(i);
				f(depth - 1, m, p, now + i, vec, !(i == m - now)/*追加してマイナスのsumを達成出来たらプラスに変更*/);
				vec.pop_back();
			}
		}
	}
	else{
		// プラスのベクトルを追加する再帰
		for(int i = std::max(vec.back(), 1); i + now < p; i++){
			vec.push_back(i);
			f(depth - 1, m, p, now + i, vec, false);
			vec.pop_back();
		}
	}
	return;
}

int predictsum(std::vector<int> & a, std::vector<std::vector<unsigned char>> & channel, int & h, int & w){
	int i1 = channel[h  ][w-1] * a[1];
	int i2 = channel[h-1][w]   * a[2];
	int i3 = channel[h  ][w-2] * a[3];
	int i4 = channel[h-1][w-1] * a[4];
	int i5 = channel[h-2][w  ] * a[5];
	int i6 = channel[h-1][w+1] * a[6];
	return (i1 + i2 + i3 + i4 + i5 + i6) >> a[7];
}

void choosebigger(std::vector<long long> & a, std::vector<long long> & b, std::vector<long long> & c, int width){
	for(int w = 0; w < width; w++){
		c[w] = std::max(a[w], b[w]);
	}
	return;
}

bool is_bigger(std::vector<long long> & a, std::vector<long long> & b, int width){
	long long asum = 0, bsum = 0;
	for(int w = 0; w < width; w++){
		asum += a[w];
		bsum += b[w];
	}
	return asum > bsum;
}
std::vector<int> chooser(std::vector<std::vector<long long>> & vec, int time){
	int height = vec.size();
	int width = vec.back().size();
	std::vector<std::vector<std::vector<long long>>> dp(time + 1, std::vector<std::vector<long long>>(height, std::vector<long long>(width, 0)));
	std::vector<std::vector<char>> change(time, std::vector<char>(height, 'y'));
	
	for(int t = 1; t <= time; t++){
		for(int h = 0; h < height; h++){
			choosebigger(dp[t - 1][h], vec[h], dp[t][h], width);
			if(h > 0){
				if(is_bigger(dp[t][h - 1], dp[t][h], width)){
					dp[t][h] = dp[t][h - 1];
					change[t - 1][h] = 'n';
				}
			}
		}
	}
	std::vector<int> result(time);
	for(int t = time - 1, h = height - 1; t >= 0 && h >= 0; t --){
		while(change[t][h] == 'n') h--;
		result[t] = h;
	}
	return result;
}

std::vector<std::vector<int>> a2;

void prepare_a(){
	int _2 = 1; // 2の累乗(2で割り切れると誤差が出ないため、可逆圧縮には2の累乗が最適)
	for(int i = 0; i <= 2; i++){ // _2(最終的な合計)を変化させるループ
		for(int j = 0; j <= 3; j++){ // マイナスのベクトルの合計を変化させるループ
			f(6, -j, _2, 0, {0}, true); // 再帰
		}
		_2 *= 2;
	}
	for(int i = 0; i < a.size(); i++){
		int sum = 0;
		for(int _a : a[i]) sum += _a;
		a[i].push_back(__builtin_ctz(sum));
		do{
			if(a[i] != std::vector<int>{0,  0,  0,  0,  0,  0,  0,  0}
			&& a[i] != std::vector<int>{0,  1,  0,  0,  0,  0,  0,  0}
			&& a[i] != std::vector<int>{0,  0,  1,  0,  0,  0,  0,  0}
			&& a[i] != std::vector<int>{0,  1,  1,  0,  0,  0,  0,  1}
			&& a[i] != std::vector<int>{0,  0,  0,  0,  1,  0,  0,  0}
			&& a[i] != std::vector<int>{0,  1,  1,  0, -1,  0,  0,  0}
			&& a[i] != std::vector<int>{0,  2,  1, -1,  0,  0,  0,  1}
			&& a[i] != std::vector<int>{0,  1,  2, -1,  0,  0,  0,  1})
			{
				a2.push_back(a[i]);
			}
		}while(std::next_permutation(a[i].begin() + 1, a[i].end() - 1));
	}
	return;
}

void worker(std::vector<IMAGE> & images, std::string outPath){
	std::vector<std::vector<long long>> d(a2.size(), std::vector<long long>(0));
	for(int i = 0; i < 100; i++){
		std::cout << i << "% " << std::flush;
		int dh = images[i].height / 10 + 1;
		for(int h = 2; h < images[i].height; h += dh){
			std::vector<std::vector<unsigned char>> dif(a2.size(), std::vector<unsigned char>((images[i].width - 4) * 3));
			for(int w = 2; w + 2 < images[i].width; w++){
				for(int n = 0; n < a2.size(); n++){
					int rsum = predictsum(a2[n], images[i].R, h, w);
					int gsum = predictsum(a2[n], images[i].G, h, w);
					int bsum = predictsum(a2[n], images[i].B, h, w);
					dif[n][w * 3 - 6] = (unsigned char)(images[i].R[h][w] - (std::min(255, std::max(0, rsum))));
					dif[n][w * 3 - 5] = (unsigned char)(images[i].G[h][w] - (std::min(255, std::max(0, gsum))));
					dif[n][w * 3 - 4] = (unsigned char)(images[i].B[h][w] - (std::min(255, std::max(0, bsum))));
				}
			}
			for(int n = 0; n < a2.size(); n++){
				long long rate = entropy_rate(dif[n]);
				d[n].push_back(rate);
			}
		}
	}
	std::ofstream out(outPath);
	std::vector<int> dp_result = chooser(d, 10);
	for(int n = 0; n < 10; n++){
		out << a2[dp_result[n]][7] << ' ';
		for(int i = 1; i < 7; i++){
			out << a2[dp_result[n]][i] << ' ';
		}
		out << std::endl;
	}
	std::cout << std::endl;
	return;
}

int main(){
	prepare_a();
	std::vector<IMAGE> images_screen = {}; int count = 0; // スクリーンショット10枚
	auto directry = std::filesystem::directory_iterator("/mnt/c/test images/screenshot/"); // (Cドライブ上のスクリーンショット)
	for(auto & i : directry){
		PNG png;
		readPNG(i.path().string(), png);
		images_screen.push_back(png.ImageData);
		if(++count == 100)break;
	}
	std::vector<IMAGE> images_natural = {}; count = 0;
	for(int l = 1; l <= 100; l++){
		std::string S = std::to_string(l);
		while(S.size() < 4)S = '0' + S;
		PNG png;
		readPNG("../../../../mnt/c/test images/X4/" + S + "x4.png", png);
		images_natural.push_back(png.ImageData);
		if(++count == 100)break;
	}
	worker(images_screen, "../out/filter_screen.csv");
	worker(images_natural, "../out/filter_natural.csv");
	return 0;
}