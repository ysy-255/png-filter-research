#ifndef opt_PNG_H
#define opt_PNG_H

#include "PNG.hpp"

float myratePredictor(std::vector<unsigned char> & vec){
	float result = 0;
	unsigned int P[256] = {0};
	std::vector<unsigned char>::iterator offset = vec.begin();
	std::vector<unsigned char>::iterator last = vec.end();
	while(offset != last) ++P[*offset ++];
	for(int x = 0; x < 256; x++){
		if(P[x] > 0){
			result += P[x] * std::log2(P[x]);
		}
	}
	return result;
}

#endif
