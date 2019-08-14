#pragma once

#include "common.hpp"

#include <algorithm>
#include <string>
#include <sstream>
#include <vector>

namespace Util {
	inline std::string toLower(const std::string& str) {
		std::string res = str;
		std::transform(res.begin(), res.end(), res.begin(), ::tolower);

		return res;
	}

	inline bool startsWith(const std::string& s0, const std::string& s1) {
		return s0.compare(0, s1.length(), s1);
	}

	void split(std::vector<std::string>& elems, const std::string& s, char delim);
	std::vector<std::string> split(const std::string& s, char delim);

	std::string getFilePath(const std::string& fileName);
	std::string getFileExtension(const std::string& fileName);

	bool resolveFileLinking(std::stringstream& out, const std::string& fileName,
			const std::string& linkKeyword);

	template <typename T>
	inline T reverseBits(T v) {
		T r = v;
		uint32 s = sizeof(T) * 8 - 1;

		for (v >>= 1; v; v >>= 1) {
			r <<= 1;
			r |= v & 1;
			--s;
		}

		r <<= s;

		return r;
	}

	template <typename T>
	inline T rotateLeft(T v, int32 shift) {
		return (v << shift) | (v >> (sizeof(T) * 8 - shift));
	}

	template <typename T>
	inline T rotateRight(T v, int32 shift) {
		return (v >> shift) | (v << (sizeof(T) * 8 - shift));
	}
};
