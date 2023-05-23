#include <vector>
#include <iostream>
#include <fstream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <vulkan/vulkan.h>
#include <cassert>
#include <unordered_map>
#include "Util.h"

std::string macros2Path(std::string macros) {
	size_t pos = 0;
	while ((pos = macros.find("\\", pos)) != std::string::npos)
	{
		macros.replace(pos, 1, "/");
		pos++;
	}
	return macros;
};

const std::string projectRoot = macros2Path(std::string(PROJECT_ROOT));
const std::string vk_layer_path = projectRoot + std::string("3rd/vulkanSDK/Bin");
const std::string shader_path = projectRoot + std::string("assets/shaders/spv/");
const std::string texture_path = projectRoot + std::string("assets/textures/viking_room.png");

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

ImageData Util::stbimgLoad(const char* path)
{
	int width, height, channels;
	stbi_uc* pixels = stbi_load(texture_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}
	ImageData img{ static_cast<uint32_t>(width) ,static_cast<uint32_t>(height) ,pixels };
	return img;
};

void Util::stbimgFree(ImageData imageData)
{
	stbi_image_free(imageData.data);
};