#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GlFW/glfw3.h>
#include <glm/glm.hpp>
class VkGLTFModel {
	VkDevice device;
	VkQueue copyQueue;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexMemory;

	struct MaterialProperties {
		uint32_t baseColorTexIdx;
		uint32_t normalTexIdx;
		uint32_t metallicRoughnessTexIdx;
	};
};