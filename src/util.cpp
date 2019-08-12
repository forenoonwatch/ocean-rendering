#include "util.hpp"

#include <fstream>
#include <cctype>

void Util::split(std::vector<std::string>& elems, const std::string& s, char delim) {
    const char* cstr = s.c_str();
    size_t strLength = (size_t)s.length();
    size_t start = 0;
    size_t end = 0;
        
    while (end <= strLength) {
        while (end <= strLength) {
            if (cstr[end] == delim) {
                break;
			}

            ++end;
        }
            
        elems.push_back(s.substr(start, end - start));

        start = end + 1;
        end = start;
    }
}

std::vector<std::string> Util::split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	Util::split(elems, s, delim);

	return elems;
}

std::string Util::getFilePath(const std::string& fileName) {
	const char* cstr = fileName.c_str();
	size_t strLength = (size_t)fileName.length();
	size_t end = strLength - 1;
	
	while (end != 0) {
		if (cstr[end] == '/' || cstr[end] == '\\') {
			break;
		}

		--end;
	}

	if (end == 0) {
		return fileName;
	}
	
	return fileName.substr(0, end + 1);
}

std::string Util::getFileExtension(const std::string& fileName) {
	const char* cstr = fileName.c_str();
	size_t strLength = (size_t)fileName.length();
	size_t end = strLength - 1;

	while (end > 0 && !std::isalnum(cstr[end])) {
		--end;
	}

	size_t start = end;

	while (start > 0) {
		if (cstr[start] == '.') {
			return fileName.substr(start + 1, end - start);
		}
		else if (cstr[start] == '/' || cstr[start] == '\\') {
			return "";
		}

		--start;
	}

	return "";
}

bool Util::resolveFileLinking(std::stringstream& out, const std::string& fileName,
		const std::string& linkKeyword) {
	std::ifstream file;
	file.open(fileName.c_str());

	std::string filePath = getFilePath(fileName);
	std::string line;

	if (file.is_open()) {
		while (file.good()) {
			std::getline(file, line);
			
			if (line.find(linkKeyword) == std::string::npos) {
				out << line << "\n";
			}
			else {
				std::string linkFileName = Util::split(line, ' ')[1];
				linkFileName = linkFileName.substr(1, linkFileName.length() - 2);

				resolveFileLinking(out, filePath + linkFileName,
						linkKeyword);
				out << "\n";
			}
		}
	}
	else {
		return false;
	}

	return true;
}
