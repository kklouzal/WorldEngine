#pragma once

//
//	Uniform Buffer Object
struct UniformBufferObject {
	glm::mat4 model{};
	glm::mat4 view{};
	glm::mat4 proj{};
	glm::mat4 bones[128]{};
};

//
//	Vertex
struct Vertex {
	glm::vec3 pos{};
	glm::vec4 color{};
	glm::vec2 texCoord{};
	glm::vec3 normal{};

	glm::vec4 Bones{};
	glm::vec4 Weights{};
	glm::vec4 Tangents{};

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 7> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 7> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;		//	Position
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Color
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;			//	UV
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;		//	Normal
		attributeDescriptions[3].offset = offsetof(Vertex, normal);

		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Joint
		attributeDescriptions[4].offset = offsetof(Vertex, Bones);

		attributeDescriptions[5].binding = 0;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Weight
		attributeDescriptions[5].offset = offsetof(Vertex, Weights);

		attributeDescriptions[6].binding = 0;
		attributeDescriptions[6].location = 6;
		attributeDescriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Tangent
		attributeDescriptions[6].offset = offsetof(Vertex, Tangents);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && texCoord == other.texCoord;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec4>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct DescriptorObject {
	VulkanDriver* _Driver;
	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> DescriptorSets = {};

	DescriptorObject(VulkanDriver* Driver) : _Driver(Driver) {}
	~DescriptorObject() {
		vkDestroyDescriptorPool(_Driver->_VulkanDevice->logicalDevice, DescriptorPool, nullptr);
	}
};

struct TextureObject {
	VulkanDriver* _Driver = nullptr;
	VmaAllocation Allocation = VMA_NULL;
	VkImage Image = VK_NULL_HANDLE;
	VkImageView ImageView = VK_NULL_HANDLE;
	unsigned int Width = 0;
	unsigned int Height = 0;
	std::vector<unsigned char> Pixels = {};
	bool Empty = true;

	//	ToDo: vector.swap image(p)
	TextureObject(VulkanDriver* Driver) : _Driver(Driver) {}
	~TextureObject() {
		if (!Empty) {
			vkDestroyImageView(_Driver->_VulkanDevice->logicalDevice, ImageView, nullptr);
			vmaDestroyImage(_Driver->allocator, Image, Allocation);
		}
	}
};

#include "PipelineObject.hpp"
