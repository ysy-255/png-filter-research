#ifndef opt_PNG_H
#define opt_PNG_H

#include "PNG.hpp"

// 返される値が大きいほうがエントロピーが小さい
float entropy_rate(std::vector<unsigned char> & vec){
	float H_tilde = 0;
	unsigned int P[256] = {0};
	std::vector<unsigned char>::iterator offset = vec.begin();
	std::vector<unsigned char>::iterator last = vec.end();
	while(offset != last) ++P[*offset ++];
	for(int x = 0; x < 256; x++){
		if(P[x] > 0){
			H_tilde += P[x] * std::log2(P[x]);
		}
	}
	return H_tilde;
}

#endif


/*
H(S) = - \frac{1}{n} \sum_{X} P(X) \log_2 P(X)\\
\quad \space H(S) \cdot n^2 - n \log_2n = - \sum_{X} S(X) \log_2 S(X) \quad
∵P(X) = \frac{S(X)}{n}\\
\tilde{H}(S) = \sum_{X} S(X) \log_2 S(X)\qquad\,
*/
