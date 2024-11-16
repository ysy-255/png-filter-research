#ifndef IMAGE_H
#define IMAGE_H

#include <bits/types.h>
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>

#include <vector>


class IMAGE{
	public:
	uint32_t width = 0;
	uint32_t height = 0;
	std::vector<std::vector<uint8_t>> R;
	std::vector<std::vector<uint8_t>> G;
	std::vector<std::vector<uint8_t>> B;
	IMAGE(){};
	IMAGE(uint32_t W, uint32_t H):
		width(W),
		height(H),
		R(H, std::vector<uint8_t>(W, 0)),
		G(H, std::vector<uint8_t>(W, 0)),
		B(H, std::vector<uint8_t>(W, 0))
	{}
};

#endif
