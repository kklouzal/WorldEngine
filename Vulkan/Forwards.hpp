#pragma once

//
//	Uniform Buffer Object
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

//
//	Vertex
struct Vertex {
	glm::vec3 pos;
	glm::vec4 color;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription() {
		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

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
		vkDestroyDescriptorPool(_Driver->device, DescriptorPool, nullptr);
	}
};

struct TextureObject {
	VulkanDriver* _Driver;
	VmaAllocation Allocation = VMA_NULL;
	VkImage Image = VK_NULL_HANDLE;
	VkImageView ImageView = VK_NULL_HANDLE;
	VkSampler Sampler = VK_NULL_HANDLE;
	int Width;
	int Height;
	stbi_uc* Pixels;

	TextureObject(VulkanDriver* Driver, int W, int H, stbi_uc* P) : _Driver(Driver), Width(W), Height(H), Pixels(P) {}
	~TextureObject() {
		vkDestroySampler(_Driver->device, Sampler, nullptr);
		vkDestroyImageView(_Driver->device, ImageView, nullptr);
		vmaDestroyImage(_Driver->allocator, Image, Allocation);
		stbi_image_free(Pixels);
	}
};

#include "PipelineObject.hpp"
