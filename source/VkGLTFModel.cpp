#include "VkGLTFModel.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tiny_gltf.h>

const std::string model_path = projectRoot + std::string("assets/gltf/outhere_space_buddy/scene.gltf");

glm::mat4 VkGLTFModel::Node::getLocalMatrix()
{
	return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
}

VkGLTFModel::~VkGLTFModel()
{
};

void VkGLTFModel::setup() {
	loadGLTFFile(model_path);
}

void VkGLTFModel::cleanup() {
	vkDestroyBuffer(rhi->device, vertexBuffer, nullptr);
	vkFreeMemory(rhi->device, vertexBufferMemory, nullptr);
	vkDestroyBuffer(rhi->device, indexBuffer, nullptr);
	vkFreeMemory(rhi->device, indexBufferMemory, nullptr);

	for (ImageProptie imgProp : imgProps)
	{
		vkDestroyImageView(rhi->device, imgProp.view, nullptr);
		vkDestroyImage(rhi->device, imgProp.image, nullptr);
		vkDestroySampler(rhi->device, imgProp.sampler, nullptr);
		vkFreeMemory(rhi->device, imgProp.memory, nullptr);
	}

	for (Skin skin : skins)
	{
		vkDestroyBuffer(rhi->device, skin.buffer, nullptr);
		vkFreeMemory(rhi->device, skin.bufferMemory, nullptr);
	}
}

void VkGLTFModel::updateUniformBuffer() {
}

void VkGLTFModel::recordCommandBuffer() {
}

void VkGLTFModel::loadImages(tinygltf::Model& input)
{
	imgProps.resize(input.images.size());
	for (size_t i = 0; i < input.images.size(); i++)
	{
		tinygltf::Image& glTFImage = input.images[i];
		unsigned char* buffer = nullptr;
		VkDeviceSize   bufferSize = 0;
		bool           deleteBuffer = false;
		if (glTFImage.component == 3)
		{
			bufferSize = glTFImage.width * glTFImage.height * 4;
			buffer = new unsigned char[bufferSize];
			unsigned char* rgba = buffer;
			unsigned char* rgb = &glTFImage.image[0];
			for (size_t i = 0; i < glTFImage.width * glTFImage.height; ++i)
			{
				memcpy(rgba, rgb, sizeof(unsigned char) * 3);
				rgba += 4;
				rgb += 3;
			}
			deleteBuffer = true;
		}
		else
		{
			buffer = &glTFImage.image[0];
			bufferSize = glTFImage.image.size();
		}

		auto& imgProp = imgProps[i];
		ImageData imageData{ glTFImage.width,glTFImage.height,buffer };
		rhi->createTextureImage(imageData, imgProp.image, imgProp.memory);
		rhi->createTextureImageView(imgProp.image, imgProp.view);
		rhi->createTextureSampler(imgProp.sampler);

		if (deleteBuffer)
		{
			delete[] buffer;
		}
	}
};

void VkGLTFModel::loadTextures(tinygltf::Model& input)
{
	texrureImageIndices.resize(input.textures.size());
	for (size_t i = 0; i < input.textures.size(); i++)
	{
		texrureImageIndices[i] = input.textures[i].source;
	}
};

void VkGLTFModel::loadMaterials(tinygltf::Model& input)
{
	materialProps.resize(input.materials.size());
	for (size_t i = 0; i < input.materials.size(); i++)
	{
		auto& mtlProp = materialProps[i];
		tinygltf::Material glTFMaterial = input.materials[i];
		if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end())
		{
			mtlProp.baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
		}
		if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end())
		{
			auto textureIdx = glTFMaterial.values["baseColorTexture"].TextureIndex();
			auto imageIdx = texrureImageIndices[i];
			mtlProp.baseColorImg = imgProps[imageIdx];
		}
		if (glTFMaterial.values.find("metallicRoughnessTexture") != glTFMaterial.values.end())
		{
			auto textureIdx = glTFMaterial.values["metallicRoughnessTexture"].TextureIndex();
			auto imageIdx = texrureImageIndices[i];
			mtlProp.metallicRoughnessImg = imgProps[imageIdx];
		}
		auto textureIdx = glTFMaterial.normalTexture.index;
		auto imageIdx = texrureImageIndices[i];
		mtlProp.metallicRoughnessImg = imgProps[imageIdx];
	}
};

void VkGLTFModel::loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, VkGLTFModel::Node* parent, uint32_t nodeIndex, std::vector<uint32_t>& indexBuffer, std::vector<VkGLTFModel::Vertex>& vertexBuffer)
{
	VkGLTFModel::Node* node = new VkGLTFModel::Node{};
	node->matrix = glm::mat4(1.0f);
	node->parent = parent;
	node->index = nodeIndex;
	node->skin = inputNode.skin;
	nodeCnt++;

	if (inputNode.translation.size() == 3)
	{
		node->translation = glm::vec3(glm::make_vec3(inputNode.translation.data()));
	}
	if (inputNode.rotation.size() == 4)
	{
		glm::quat q = glm::make_quat(inputNode.rotation.data());
		node->rotation = glm::mat4(q);
	}
	if (inputNode.scale.size() == 3)
	{
		node->scale = glm::vec3(glm::make_vec3(inputNode.scale.data()));
	}
	if (inputNode.matrix.size() == 16)
	{
		node->matrix = glm::make_mat4x4(inputNode.matrix.data());
	};

	if (inputNode.children.size() > 0)
	{
		for (size_t i = 0; i < inputNode.children.size(); i++)
		{
			loadNode(input.nodes[inputNode.children[i]], input, node, inputNode.children[i], indexBuffer, vertexBuffer);
		}
	}
	if (inputNode.mesh > -1)
	{
		const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
		for (size_t i = 0; i < mesh.primitives.size(); i++)
		{
			const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
			uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
			uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
			uint32_t indexCount = 0;
			{
				const float* positionBuffer = nullptr;
				const float* normalsBuffer = nullptr;
				const float* tangentsBuffer = nullptr;
				const float* texCoordsBuffer = nullptr;
				const uint16_t* jointIndicesBuffer = nullptr;
				const float* jointWeightsBuffer = nullptr;
				size_t vertexCount = 0;
				bool hasSkin = false;
				if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
					positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					vertexCount = accessor.count;
				}
				if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
					normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				if (glTFPrimitive.attributes.find("TANGENT") != glTFPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TANGENT")->second];
					const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
					tangentsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
					texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				if (glTFPrimitive.attributes.find("JOINTS_0") != glTFPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("JOINTS_0")->second];
					const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
					jointIndicesBuffer = reinterpret_cast<const uint16_t*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				if (glTFPrimitive.attributes.find("WEIGHTS_0") != glTFPrimitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("WEIGHTS_0")->second];
					const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
					jointWeightsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
				}
				hasSkin = (jointIndicesBuffer && jointWeightsBuffer);
				for (size_t v = 0; v < vertexCount; v++)
				{
					Vertex vert{};
					vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
					vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
					vert.tangent = glm::normalize(glm::vec4(tangentsBuffer ? glm::make_vec4(&tangentsBuffer[v * 4]) : glm::vec4(0.0f)));
					vert.uv = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec3(0.0f);
					vert.color = glm::vec3(1.0f);
					vertexBuffer.push_back(vert);
				}
			}
			{
				const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
				const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
				indexCount += static_cast<uint32_t>(accessor.count);
				switch (accessor.componentType)
				{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
				{
					const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
					for (size_t index = 0; index < accessor.count; index++)
					{
						indexBuffer.push_back(buf[index] + vertexStart);
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
				{
					const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
					for (size_t index = 0; index < accessor.count; index++)
					{
						indexBuffer.push_back(buf[index] + vertexStart);
					}
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
				{
					const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
					for (size_t index = 0; index < accessor.count; index++)
					{
						indexBuffer.push_back(buf[index] + vertexStart);
					}
					break;
				}
				default:
					std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
					return;
				}
			}
			Primitive primitive{};
			primitive.firstIndex = firstIndex;
			primitive.indexCount = indexCount;
			primitive.materialPropertie = materialProps[glTFPrimitive.material];
			node->primitives.push_back(primitive);
		}
	}

	if (parent)
	{
		parent->children.push_back(node);
	}
	else
	{
		nodes.push_back(node);
	}
}

void VkGLTFModel::loadSkins(tinygltf::Model& input)
{
	skins.resize(input.skins.size());

	for (size_t i = 0; i < input.skins.size(); i++)
	{
		tinygltf::Skin glTFSkin = input.skins[i];
		skins[i].name = glTFSkin.name;
		skins[i].skeletonRoot = findNode(glTFSkin.skeleton);
		for (int jointIndex : glTFSkin.joints)
		{
			Node* node = findNode(jointIndex);
			if (node)
			{
				skins[i].joints.push_back(node);
			}
		}
		if (glTFSkin.inverseBindMatrices > -1)
		{
			const tinygltf::Accessor& accessor = input.accessors[glTFSkin.inverseBindMatrices];
			const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
			const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
			skins[i].inverseBindMatrices.resize(accessor.count);
			memcpy(skins[i].inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
			auto bufferSize = sizeof(glm::mat4) * skins[i].inverseBindMatrices.size();
			rhi->createBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				skins[i].buffer, skins[i].bufferMemory);

			vkMapMemory(rhi->device, skins[i].bufferMemory, 0, bufferSize, 0, &skins[i].bufferMapped);
		}
	}
}

void VkGLTFModel::loadAnimations(tinygltf::Model& input)
{
	animations.resize(input.animations.size());
	for (size_t i = 0; i < input.animations.size(); i++)
	{
		tinygltf::Animation glTFAnimation = input.animations[i];
		animations[i].name = glTFAnimation.name;
		animations[i].samplers.resize(glTFAnimation.samplers.size());
		for (size_t j = 0; j < glTFAnimation.samplers.size(); j++)
		{
			tinygltf::AnimationSampler glTFSampler = glTFAnimation.samplers[j];
			AnimationSampler& dstSampler = animations[i].samplers[j];
			dstSampler.interpolation = glTFSampler.interpolation;
			{
				const tinygltf::Accessor& accessor = input.accessors[glTFSampler.input];
				const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				const float* buf = static_cast<const float*>(dataPtr);
				for (size_t index = 0; index < accessor.count; index++)
				{
					dstSampler.inputs.push_back(buf[index]);
				}
				for (auto input : animations[i].samplers[j].inputs)
				{
					if (input < animations[i].start)
					{
						animations[i].start = input;
					};
					if (input > animations[i].end)
					{
						animations[i].end = input;
					}
				}
			}
			{
				const tinygltf::Accessor& accessor = input.accessors[glTFSampler.output];
				const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];
				const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
				switch (accessor.type)
				{
				case TINYGLTF_TYPE_VEC3:
				{
					const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						dstSampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
					}
					break;
				}
				case TINYGLTF_TYPE_VEC4:
				{
					const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						dstSampler.outputsVec4.push_back(buf[index]);
					}
					break;
				}
				default:
				{
					std::cout << "unknown type" << std::endl;
					break;
				}
				}
			}
		}
		animations[i].channels.resize(glTFAnimation.channels.size());
		for (size_t j = 0; j < glTFAnimation.channels.size(); j++)
		{
			tinygltf::AnimationChannel glTFChannel = glTFAnimation.channels[j];
			AnimationChannel& dstChannel = animations[i].channels[j];
			dstChannel.path = glTFChannel.target_path;
			dstChannel.samplerIndex = glTFChannel.sampler;
			dstChannel.node = findNode(glTFChannel.target_node);
		}
	}
}

VkGLTFModel::Node* VkGLTFModel::findNode(Node* parent, uint32_t index)
{
	Node* nodeFound = nullptr;
	if (parent->index == index)
	{
		return parent;
	}
	for (auto& child : parent->children)
	{
		nodeFound = findNode(child, index);
		if (nodeFound)
		{
			break;
		}
	}
	return nodeFound;
}

VkGLTFModel::Node* VkGLTFModel::findNode(uint32_t index)
{
	Node* nodeFound = nullptr;
	for (auto& node : nodes)
	{
		nodeFound = findNode(node, index);
		if (nodeFound)
		{
			break;
		}
	}
	return nodeFound;
}

glm::mat4 VkGLTFModel::getNodeMatrix(VkGLTFModel::Node* node)
{
	glm::mat4 nodeMatrix = node->getLocalMatrix();
	VkGLTFModel::Node* currentParent = node->parent;
	while (currentParent)
	{
		nodeMatrix = currentParent->getLocalMatrix() * nodeMatrix;
		currentParent = currentParent->parent;
	}
	return nodeMatrix;
}

void VkGLTFModel::updateJoints(VkGLTFModel::Node* node)
{
	if (node->skin > -1)
	{
		glm::mat4 inverseTransform = glm::inverse(getNodeMatrix(node));
		Skin skin = skins[node->skin];
		size_t numJoints = (uint32_t)skin.joints.size();
		std::vector<glm::mat4> jointMatrices(numJoints);
		for (size_t i = 0; i < numJoints; i++)
		{
			jointMatrices[i] = getNodeMatrix(skin.joints[i]) * skin.inverseBindMatrices[i];
			jointMatrices[i] = inverseTransform * jointMatrices[i];
		}
		memcpy(skin.bufferMapped, jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4));
	}

	for (auto& child : node->children)
	{
		updateJoints(child);
	}
}

void VkGLTFModel::updateAnimation(float deltaTime)
{
	if (activeAnimation > static_cast<uint32_t>(animations.size()) - 1)
	{
		std::cout << "No animation with index " << activeAnimation << std::endl;
		return;
	}
	Animation& animation = animations[activeAnimation];
	animation.currentTime += deltaTime;
	if (animation.currentTime > animation.end)
	{
		animation.currentTime -= animation.end;
	}

	for (auto& channel : animation.channels)
	{
		AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
		for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
		{
			if (sampler.interpolation != "LINEAR")
			{
				std::cout << "This sample only supports linear interpolations\n";
				continue;
			}
			if ((animation.currentTime >= sampler.inputs[i]) && (animation.currentTime <= sampler.inputs[i + 1]))
			{
				float a = (animation.currentTime - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
				if (channel.path == "translation")
				{
					channel.node->translation = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
				}
				if (channel.path == "rotation")
				{
					glm::quat q1;
					q1.x = sampler.outputsVec4[i].x;
					q1.y = sampler.outputsVec4[i].y;
					q1.z = sampler.outputsVec4[i].z;
					q1.w = sampler.outputsVec4[i].w;

					glm::quat q2;
					q2.x = sampler.outputsVec4[i + 1].x;
					q2.y = sampler.outputsVec4[i + 1].y;
					q2.z = sampler.outputsVec4[i + 1].z;
					q2.w = sampler.outputsVec4[i + 1].w;

					channel.node->rotation = glm::normalize(glm::slerp(q1, q2, a));
				}
				if (channel.path == "scale")
				{
					channel.node->scale = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], a);
				}
			}
		}
	}
	for (auto& node : nodes)
	{
		updateJoints(node);
	}
}

void VkGLTFModel::drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkGLTFModel::Node* node)
{
	if (node->primitives.size() > 0)
	{
		glm::mat4 nodeMatrix = node->matrix;
		VkGLTFModel::Node* currentParent = node->parent;
		while (currentParent)
		{
			nodeMatrix = currentParent->matrix * nodeMatrix;
			currentParent = currentParent->parent;
		}
		vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);
		if (node->skin >= 0)
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &skins[node->skin].descriptorSet, 0, nullptr);
		}
		for (VkGLTFModel::Primitive& primitive : node->primitives)
		{
			if (primitive.indexCount > 0)
			{
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 2, 1, &primitive.materialPropertie.baseColorImg.descriptorSet, 0, nullptr);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 3, 1, &primitive.materialPropertie.normalImg.descriptorSet, 0, nullptr);
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 4, 1, &primitive.materialPropertie.metallicRoughnessImg.descriptorSet, 0, nullptr);
				vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
			}
		}
	}
	for (auto& child : node->children)
	{
		drawNode(commandBuffer, pipelineLayout, child);
	}
}

void VkGLTFModel::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
{
	VkDeviceSize offsets[1] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	for (auto& node : nodes)
	{
		drawNode(commandBuffer, pipelineLayout, node);
	}
}

void VkGLTFModel::loadGLTFFile(std::string filename) {
	tinygltf::Model    glTFInput;
	tinygltf::TinyGLTF gltfContext;
	std::string        error, warning;

	bool fileLoaded = gltfContext.LoadASCIIFromFile(&glTFInput, &error, &warning, filename);
	if (!fileLoaded) {
		std::cerr << "loadGLTFFile fail:" << std::endl;
	}
	std::vector<uint32_t>	indices;
	std::vector<Vertex>		vertices;
	loadImages(glTFInput);
	loadTextures(glTFInput);
	loadMaterials(glTFInput);
	const tinygltf::Scene& scene = glTFInput.scenes[0];
	for (size_t i = 0; i < scene.nodes.size(); i++)
	{
		const tinygltf::Node node = glTFInput.nodes[scene.nodes[i]];
		loadNode(node, glTFInput, nullptr, scene.nodes[i], indices, vertices);
	}
	loadSkins(glTFInput);
	loadAnimations(glTFInput);
	for (auto node : nodes)
	{
		updateJoints(node);
	}

	size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
	size_t indexBufferSize = indices.size() * sizeof(uint32_t);

	rhi->createVertexBuffer(vertexBufferSize, vertices.data(), vertexBuffer, vertexBufferMemory);
	rhi->createIndexBuffer(indexBufferSize, indices.data(), indexBuffer, indexBufferMemory);
}