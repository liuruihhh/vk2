#pragma once
#include <string>
#include <algorithm>
#include <cfloat>
#include <limits>
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>
#include "VkRHI.h"
#include "VkGLTFModel.h"

class VkGLTFModel {
public:

	struct ImageProptie {
		VkImage					image;
		VkImageLayout			layout;
		VkDeviceMemory			memory;
		VkImageView				view;
		uint32_t				width, height;
		uint32_t				miplvs;
		uint32_t				layerCnt;
		VkDescriptorImageInfo	descriptor;
		VkSampler				sampler;
		VkDescriptorSet			descriptorSet;
	};

	struct MaterialPropertie {
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		ImageProptie* baseColorImg;
		ImageProptie* normalImg;
		ImageProptie* metallicRoughnessImg;
	};

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec4 tangent;
		glm::vec2 uv;
		glm::vec3 color;
		glm::vec4 jointIndices;
		glm::vec4 jointWeights;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions;
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, normal);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, tangent);

			attributeDescriptions[3].binding = 0;
			attributeDescriptions[3].location = 3;
			attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[3].offset = offsetof(Vertex, uv);

			attributeDescriptions[4].binding = 0;
			attributeDescriptions[4].location = 4;
			attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[4].offset = offsetof(Vertex, color);

			attributeDescriptions[5].binding = 0;
			attributeDescriptions[5].location = 5;
			attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[5].offset = offsetof(Vertex, jointIndices);

			attributeDescriptions[6].binding = 0;
			attributeDescriptions[6].location = 6;
			attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attributeDescriptions[6].offset = offsetof(Vertex, jointWeights);

			return attributeDescriptions;
		}
	};

	struct Primitive
	{
		uint32_t firstIndex;
		uint32_t indexCount;
		MaterialPropertie  materialPropertie;
	};

	struct Node
	{
		Node* parent;
		uint32_t				index;
		std::vector<Node*>		children;
		std::vector<Primitive>	primitives;
		glm::vec3				translation{};
		glm::vec3				scale{ 1.0f };
		glm::quat				rotation{};
		int32_t					skin = -1;
		glm::mat4				matrix;
		glm::mat4				getLocalMatrix();
	};

	struct Skin {
		std::string				name;
		Node* skeletonRoot = nullptr;
		std::vector<glm::mat4>	inverseBindMatrices;
		std::vector<Node*>		joints;

		std::vector <VkBuffer>					jointMatricesbuffers;
		std::vector <VkDeviceMemory>			jointMatricesBufferMemories;
		std::vector<void*>						jointMatricesBufferMappeds;
		std::vector <VkDescriptorBufferInfo>	jointMatricesbufferDescriptors;
		std::vector <VkDescriptorSet>			jointMatricesDescriptorSets;
	};

	struct AnimationSampler
	{
		std::string            interpolation;
		std::vector<float>     inputs;
		std::vector<glm::vec4> outputsVec4;
	};

	struct AnimationChannel
	{
		std::string path;
		Node* node;
		uint32_t    samplerIndex;
	};

	struct Animation
	{
		std::string                   name;
		std::vector<AnimationSampler> samplers;
		std::vector<AnimationChannel> channels;
		float                         start = FLT_MAX;
		float                         end = FLT_MIN;
		float                         currentTime = 0.0f;
	};

	struct UniformBufferObject
	{
		glm::mat4 view;
		glm::mat4 proj;
		glm::vec4 viewPos;
		glm::vec4 lightPos;
	};

	struct PushConstObject {
		glm::mat4 model;
		glm::vec4 baseColorFactor;
	};

	VkRHI* rhi;

	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	std::vector<void*> uniformBuffersMapped;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<uint32_t>			texrureImageIndices;
	std::vector<MaterialPropertie>	materialProps;
	std::vector<ImageProptie>		imgProps;
	std::vector<Node*>				nodes;
	std::vector<Skin>				skins;
	std::vector<Animation>			animations;
	uint32_t						nodeCnt = 0;
	uint32_t						activeAnimation = 0;

	~VkGLTFModel();
	void		setup();
	void		cleanup();
	void		drawFrame(float delta, float time);
	void		updateUniformBuffer(float delta, float time);
	void		recordCommandBuffer();
	void		loadImages(tinygltf::Model& input);
	void		loadTextures(tinygltf::Model& input);
	void		loadMaterials(tinygltf::Model& input);
	void		loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, VkGLTFModel::Node* parent, uint32_t nodeIndex,
		std::vector<uint32_t>& indexBuffer, std::vector<VkGLTFModel::Vertex>& vertexBuffer);
	void		loadSkins(tinygltf::Model& input);
	void		loadAnimations(tinygltf::Model& input);
	Node* findNode(Node* parent, uint32_t index);
	Node* findNode(uint32_t index);
	glm::mat4	getNodeMatrix(VkGLTFModel::Node* node);
	void		updateJoints(VkGLTFModel::Node* node);
	void		updateAnimation(float deltaTime);
	void		drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkGLTFModel::Node* node);
	void		draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout);
	void		loadGLTFFile(std::string filename);
	void		createRenderPass();
	void		createDescriptorSetLayout();
	void		createGraphicsPipeline();
	void		createUniformBuffers();
	void		createDescriptorPool();
	void		createDescriptorSets();
};