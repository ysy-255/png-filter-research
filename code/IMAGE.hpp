#ifndef IMAGE_H
#define IMAGE_H


#include <vector>
#include <cmath>


struct IMAGE{
	std::vector<std::vector<unsigned char>>R;
	std::vector<std::vector<unsigned char>>G;
	std::vector<std::vector<unsigned char>>B;
	unsigned int height;
	unsigned int width;
	IMAGE() = default;
	IMAGE(unsigned int height, unsigned int width):
		width(width),
		height(height),
		R(height, std::vector<unsigned char>(width)),
		G(height, std::vector<unsigned char>(width)),
		B(height, std::vector<unsigned char>(width))
	{}
};

float image_compare(const IMAGE &image1, const IMAGE &image2){
	long long gap = 0;
	for(int i = 0; i < image1.height; i++){
		for(int j = 0; j < image1.width; j++){
			gap += std::pow(std::abs(image1.R[i][j] - image2.R[i][j]), 2);
			gap += std::pow(std::abs(image1.G[i][j] - image2.G[i][j]), 2);
			gap += std::pow(std::abs(image1.B[i][j] - image2.B[i][j]), 2);
		}
	}
	return std::pow(gap / (image1.height * image1.width * 3), 0.5);
}

#define NEAREST_NEIGHBOR 0

IMAGE image_enlarge(
		IMAGE & image,
		float scaleY,
		float scaleX,
		unsigned char type // この関数の直前にdefineしたもの
	){
	if(image.height * scaleY > 65535 || image.height * scaleY > 65535) return {};
	IMAGE result(static_cast<int>(image.height * scaleY), static_cast<int>(image.width * scaleX));
	switch(type){
		case NEAREST_NEIGHBOR:{
			for(int y = 0; y < result.height; y++){
				for(int x = 0; x < result.width; x++){
					int ypoint = static_cast<int>((2 * y + 1) / scaleY) / 2;
					int xpoint = static_cast<int>((2 * x + 1) / scaleX) / 2;
					result.R[y][x] = image.R[ypoint][xpoint];
					result.G[y][x] = image.G[ypoint][xpoint];
					result.B[y][x] = image.B[ypoint][xpoint];
				}
			}
			break;
		}
	}
	return result;
}

#endif
