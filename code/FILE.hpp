#ifndef FILE_H
#define FILE_H

#include <bits/types.h>
#include <bits/stdint-intn.h>
#include <bits/stdint-uintn.h>

#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <algorithm>

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	const bool SYSTEM_LITTLE_ENDIAN = true;
#else
	const bool SYSTEM_LITTLE_ENDIAN = false;
#endif

inline std::vector<uint8_t> filepath2data(const std::string & path){
	std::ifstream file_ifstream(path, std::ios::binary | std::ios::ate);
	if(!file_ifstream.is_open()) return {};
	size_t file_size = file_ifstream.tellg();
	file_ifstream.seekg(0);
	std::vector<uint8_t> result(file_size);
	file_ifstream.read(reinterpret_cast<char*>(result.data()), file_size);
	return result;
}

inline std::vector<std::string> get_allfile(const std::string & folder_path){
	std::vector<std::string> result;
	for(auto & x : std::filesystem::directory_iterator(folder_path)){
		if(x.is_regular_file()){
			result.push_back(x.path().string());
		}
	}
	return result;
}

uint16_t VU8_U16(
		const std::vector<uint8_t> & vec,
		size_t & index,
		const bool is_little_endian // 3412
	){
	uint16_t value = 0;
	size_t newindex = index + 2;
	if(is_little_endian == SYSTEM_LITTLE_ENDIAN){
		std::copy(&vec[index], &vec[newindex], reinterpret_cast<uint8_t*>(&value));
	}
	else{
		std::reverse_copy(&vec[index], &vec[newindex], reinterpret_cast<uint8_t*>(&value));
	}
	index = newindex;
	return value;
}

uint32_t VU8_U32(
		const std::vector<uint8_t> & vec,
		size_t & index,
		const bool is_little_endian
	){
	uint32_t value = 0;
	size_t newindex = index + 4;
	if(is_little_endian == SYSTEM_LITTLE_ENDIAN){
		std::copy(&vec[index], &vec[newindex], reinterpret_cast<uint8_t*>(&value));
	}
	else{
		std::reverse_copy(&vec[index], &vec[newindex], reinterpret_cast<uint8_t*>(&value));
	}
	index = newindex;
	return value;
}

template<typename T>
void U16_write_stream(
		const uint16_t value,
		std::basic_ostream<T> & out,
		const bool is_little_endian
	){
	if(is_little_endian){
		out.put(static_cast<T>(value >> 0));
		out.put(static_cast<T>(value >> 8));
	}
	else{
		out.put(static_cast<T>(value >> 8));
		out.put(static_cast<T>(value >> 0));
	}
	return;
}

template<typename T>
void U32_write_stream(
		const uint32_t value,
		std::basic_ostream<T> & out,
		const bool is_little_endian
	){
	if(is_little_endian){
		out.put(static_cast<T>(value >>  0));
		out.put(static_cast<T>(value >>  8));
		out.put(static_cast<T>(value >> 16));
		out.put(static_cast<T>(value >> 24));
	}
	else{
		out.put(static_cast<T>(value >> 24));
		out.put(static_cast<T>(value >> 16));
		out.put(static_cast<T>(value >>  8));
		out.put(static_cast<T>(value >>  0));
	}
	return;
}

void U16_write_VU8(
		const uint16_t value,
		std::vector<uint8_t> & vec,
		size_t & index,
		const bool is_little_endian
	){
	if(is_little_endian){
		vec[index ++] = static_cast<uint8_t>(value >> 0);
		vec[index ++] = static_cast<uint8_t>(value >> 8);
	}
	else{
		vec[index ++] = static_cast<uint8_t>(value >> 8);
		vec[index ++] = static_cast<uint8_t>(value >> 0);
	}
	return;
}

void U32_write_VU8(
		const uint32_t value,
		std::vector<uint8_t> & vec,
		size_t & index,
		const bool isLittleEndian
	){
	if(isLittleEndian){
		vec[index ++] = static_cast<uint8_t>(value >>  0);
		vec[index ++] = static_cast<uint8_t>(value >>  8);
		vec[index ++] = static_cast<uint8_t>(value >> 16);
		vec[index ++] = static_cast<uint8_t>(value >> 24);
	}
	else{
		vec[index ++] = static_cast<uint8_t>(value >> 24);
		vec[index ++] = static_cast<uint8_t>(value >> 16);
		vec[index ++] = static_cast<uint8_t>(value >>  8);
		vec[index ++] = static_cast<uint8_t>(value >>  0);
	}
	return;
}


#endif
