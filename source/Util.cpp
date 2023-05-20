#include <vector>
#include <iostream>
#include <fstream>
#include "Util.h"

char const* vk_layer_path = "D:/project/VK/vk2/3rd/vulkanSDK/Bin";
char const* shader_path = "D:/project/VK/vk2/assets/shaders/spv/";
char const* model_path = "D:/project/VK/vk2/assets/models/viking_room.obj";
char const* texture_path = "D:/project/VK/vk2/assets/textures/viking_room.png";

std::vector<char> Util::readShader(const std::string& filename) {
	std::string filePath = shader_path;
	filePath += filename;
	auto shaderCode = readFile(filePath);
	return shaderCode;
}

std::vector<char> Util::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}