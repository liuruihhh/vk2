#pragma once
#include <vector>
#include <iostream>
#include <fstream>

extern char const* vk_layer_path;
extern char const* shader_path;
extern char const* model_path;
extern char const* texture_path;

class Util {
public:
	static std::vector<char> readShader(const std::string& filename);
	static std::vector<char> readFile(const std::string& filename);
};
