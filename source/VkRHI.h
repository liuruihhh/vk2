#pragma once
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>
#include <array>
#include <sstream>
#include <windows.h>
#include <algorithm>
#include <set>
#include <fstream>
#include <unordered_map>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GlFW/glfw3.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Util.h"

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

extern const int MAX_FRAMES_IN_FLIGHT;

class VkRHI {
public:
#pragma region class_member
	bool framebufferResized;
	VkDevice device;
	GLFWwindow* window;
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	VkRenderPass renderPass;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	//VkDescriptorSetLayout descriptorSetLayout;
	//VkPipelineLayout pipelineLayout;
	//VkPipeline graphicsPipeline;
	VkCommandPool commandPool;
	//std::vector<Vertex> vertices;
	//std::vector<uint32_t> indices;
	//VkBuffer vertexBuffer;
	//VkDeviceMemory vertexBufferMemory;
	//VkBuffer indexBuffer;
	//VkDeviceMemory indexBufferMemory;
	//std::vector<VkBuffer> uniformBuffers;
	//std::vector<VkDeviceMemory> uniformBuffersMemory;
	//std::vector<void*> uniformBuffersMapped;
	//VkDescriptorPool descriptorPool;
	//std::vector<VkDescriptorSet> descriptorSets;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	uint32_t mipLevels;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishSemaphores;
	std::vector<VkFence> inFlightFences;
	uint32_t currentFrame = 0;
	uint32_t currentImageIdx = 0;
#pragma endregion class_member
public:
	void run();
	void initWindow();
	void initVulkan();
	void mainLoop();
	void beginDrawFrame();
	void endDrawFrame();
	void cleanup();
	void cleanupSwapCain();
	void createInstance();
	void createSurface();
	void createSwapChain();
	void recreateSwapChain();
	void createSwapChainImageViews();
	//void createRenderPass();
	//void createDescriptorSetLayout();
	//void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createFramebuffers();
	void createColorResources();
	void createDepthResources();
	VkFormat findDepthFormat();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	bool hasStencilComponent(VkFormat format);
	void createTextureImage(ImageData imageData, VkImage& image, VkDeviceMemory& imageMemory);
	void createTextureImageView(VkImage& image, VkImageView& imageView);
	void createTextureSampler(VkSampler& sampler);
	void createImage(uint32_t width, uint32_t hight, uint32_t mipLevels, VkSampleCountFlagBits sampleCount, VkFormat format, VkImageTiling tiling,
	VkImageUsageFlags usage, VkMemoryPropertyFlags memProperties, VkImage& image, VkDeviceMemory& imageMemory);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectMask, uint32_t mipLevels);
	void generateMipmaps(VkImage image, VkFormat format, uint32_t texW, uint32_t texH, uint32_t mipLevels);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void createCommandPool();
	void createVertexBuffer(VkDeviceSize bufferSize, void* verticesData, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
	void createIndexBuffer(VkDeviceSize bufferSize, void* indicesData, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory);
	//void createVertexBuffer();
	//void createIndexBuffer();
	//void createUniformBuffers();
	//void createDescriptorPool();
	//void createDescriptorSets();
	//void updateUniformBuffer(uint32_t currentImage);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProperties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createCommandBuffers();
	void createSyncObjs();
	//void recordCommandBuffer();
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector< VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicDevice);
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void createLogicalDevice();
	void setupDebugMessenger();
	void pickPhysicalDevice();
	VkSampleCountFlagBits getMaxUsableSampleCount();
	bool isDeviceSuitable(VkPhysicalDevice physicalDevice);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);
	int rateDeviceSuitability(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
	static VKAPI_ATTR VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			  VkDebugUtilsMessageTypeFlagsEXT messageType,
			  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			  void* pUserData);
};