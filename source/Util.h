#pragma once
#include <vector>
#include <iostream>
#include <fstream>

extern const std::string vk_layer_path;
extern const std::string shader_path;
extern const std::string model_path;
extern const std::string texture_path;

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
