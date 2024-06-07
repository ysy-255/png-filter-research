#ifndef FILE_H
#define FILE_H


#include <fstream>
#include <iostream>
#include <vector>
#include <cstring>


unsigned short UV2_US(
		const std::vector<unsigned char> & vec,
		const unsigned int & position,
		const bool & isLittleEndian
	){
	unsigned short value;
	std::memcpy(&value, &vec[position], sizeof(unsigned short));
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		const bool systemIsLittleEndian = true;
	#else
		const bool systemIsLittleEndian = false;
	#endif
	if(isLittleEndian != systemIsLittleEndian){
		value = (value >> 8) | (value << 8);
	}
	return value;
}

unsigned int   UV4_UI(
		const std::vector<unsigned char> & vec,
		const unsigned int & position,
		const bool & isLittleEndian
	){
	unsigned int   value;
	std::memcpy(&value, &vec[position], sizeof(unsigned int));
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		const bool systemIsLittleEndian = true;
	#else
		const bool systemIsLittleEndian = false;
	#endif
	if(isLittleEndian != systemIsLittleEndian){
		value = 
		((value >> 24 ) & 0x000000FF) |
		((value >>  8 ) & 0x0000FF00) |
		((value <<  8 ) & 0x00FF0000) |
		((value << 24 ) & 0xFF000000);
	}
	return value;
}

void US_write(
		const unsigned short & value,
		std::ofstream & out,
		const bool & isLittleEndian){
	if(isLittleEndian){
		out.put(static_cast<unsigned char>(value >> 0));
		out.put(static_cast<unsigned char>(value >> 8));
	}
	else{
		out.put(static_cast<unsigned char>(value >> 8));
		out.put(static_cast<unsigned char>(value >> 0));
	}
	return;
}

void UI_write(
		const unsigned int   & value,
		std::ofstream & out,
		const bool & isLittleEndian){
	if(isLittleEndian){
		out.put(static_cast<unsigned char>(value >>  0));
		out.put(static_cast<unsigned char>(value >>  8));
		out.put(static_cast<unsigned char>(value >> 16));
		out.put(static_cast<unsigned char>(value >> 24));
	}
	else{
		out.put(static_cast<unsigned char>(value >> 24));
		out.put(static_cast<unsigned char>(value >> 16));
		out.put(static_cast<unsigned char>(value >>  8));
		out.put(static_cast<unsigned char>(value >>  0));
	}
	return;
}


#endif
