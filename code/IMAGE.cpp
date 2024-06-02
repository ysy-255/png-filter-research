#include <vector>
#include <cmath>

struct IMAGE{
	std::vector<std::vector<unsigned char>>R;
	std::vector<std::vector<unsigned char>>G;
	std::vector<std::vector<unsigned char>>B;
	unsigned int width;
	unsigned int height;
	IMAGE() = default;
	IMAGE(unsigned int w, unsigned int h):
		width(w),
		height(h),
		R(height, std::vector<unsigned char>(width)),
		G(height, std::vector<unsigned char>(width)),
		B(height, std::vector<unsigned char>(width))
	{}
};

float image_compare(const IMAGE &image1, const IMAGE &image2){
	long long gap =0;
	for(int i =0; i < image1.height; i++){
		for(int j =0; j < image1.width; j++){
			gap += std::pow(std::abs(image1.R[i][j] - image2.R[i][j]), 2);
			gap += std::pow(std::abs(image1.G[i][j] - image2.G[i][j]), 2);
			gap += std::pow(std::abs(image1.B[i][j] - image2.B[i][j]), 2);
		}
	}
	return std::pow(gap / (image1.width * image1.height * 3), 0.5);
}

void image2imos(IMAGE &image, IMAGE &reserve){
	int R,G,B;
	for(int i = image.height -1; i >=0; i--){
		for(int j = image.width -1; j >=0; j--){
			R = (image.R[i][j] - (i >0 ? image.R[i -1][j] : 0)) - ((j >0 ? image.R[i][j -1] : 0) - (i >0 && j >0 ? image.R[i -1][j -1] : 0));
			G = (image.G[i][j] - (i >0 ? image.G[i -1][j] : 0)) - ((j >0 ? image.G[i][j -1] : 0) - (i >0 && j >0 ? image.G[i -1][j -1] : 0));
			B = (image.B[i][j] - (i >0 ? image.B[i -1][j] : 0)) - ((j >0 ? image.B[i][j -1] : 0) - (i >0 && j >0 ? image.B[i -1][j -1] : 0));
			R >=0 ? R *= 2 : R = -R * 2 -1;
			G >=0 ? G *= 2 : G = -G * 2 -1;
			B >=0 ? B *= 2 : B = -B * 2 -1;
			image.R[i][j] = R % 256 ;
			image.G[i][j] = G % 256 ;
			image.B[i][j] = B % 256 ;
			reserve.R[i][j] = R >> 8;
			reserve.G[i][j] = G >> 8;
			reserve.B[i][j] = B >> 8;
		}
	}
	return;
}