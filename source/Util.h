#pragma once
#include <vector>
#include <iostream>
#include <fstream>

extern char const* vk_layer_path;
extern char const* shader_path;
extern char const* model_path;
extern char const* texture_path;

struct ImageData {
	uint32_t width;
	uint32_t height;
	void* data;
};

class Util {
public:
	static std::vector<char> readShader(const std::string& filename);
	static std::vector<char> readFile(const std::string& filename);
	static ImageData stbimgLoad(const char* path);
	static void stbimgFree(ImageData imageData);
};
