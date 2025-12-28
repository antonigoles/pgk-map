#pragma once
#include <string>
#include <vector>
#include <Engine/Core/OpenGL.hpp>

namespace Engine {
	unsigned int count_char_in_string(const std::string& str, const char& c);
	
	std::string join_string(const std::vector<std::string>& str, const std::string& connector);
	
	void split_string(const std::string& str, const char& delimiter, std::vector<std::string>& out);
	
	bool isStringFloat(const std::string& str);
	
	float parseInt(const std::vector<std::string>& tokens, int offset);

	float parseFloat(const std::vector<std::string>& tokens, int offset);

	glm::vec3 parseVec3(const std::vector<std::string>& tokens, int offset);

	std::vector<float> parseVectorFlag(
		const std::vector<std::string>& tokens, 
		std::string flag, 
		std::vector<float> d_value
	);

	std::string replace_variable(const std::string& str, const std::string& variable, const std::string& value);
}