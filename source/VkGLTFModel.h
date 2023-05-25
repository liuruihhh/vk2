#pragma once
#include <GlFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include "VkRHI.h"
class VkGLTFModel {
	VkRHI* rhi;

	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

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
		ImageProptie baseColorImg;
		ImageProptie normalImg;
		ImageProptie metallicRoughnessImg;
	};

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec4 tangent;
		glm::vec2 uv;
		glm::vec3 color;
		glm::vec4 jointIndices;
		glm::vec4 jointWeights;
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
		VkBuffer				buffer;
		VkDeviceMemory			bufferMemory;
		void* bufferMapped;
		VkDescriptorBufferInfo	bufferDescriptor;
		VkDescriptorSet			descriptorSet;
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
		float                         start = std::numeric_limits<float>::max();
		float                         end = std::numeric_limits<float>::min();
		float                         currentTime = 0.0f;
	};

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
	void		updateUniformBuffer();
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
};