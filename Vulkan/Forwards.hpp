#pragma once
#define WIN32_LEAN_AND_MEAN
#pragma warning( disable : 4714 )
#pragma warning( disable : 4267 )

#include <Gwen/Gwen.h>
#include <Gwen/Skins/TexturedBase.h>
#include <Gwen/Input/Windows.h>
#include <Gwen/Controls.h>
#include <Gwen/Controls/WindowControl.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_INLINE
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#include "TinyGLTF.hpp"

#define _D_CORE_DLL
#define _D_NEWTON_DLL
#define _D_COLLISION_DLL
#include <ndNewton.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "LuaScripting.hpp"

#include <functional>
#include <algorithm>
#include <optional>
#include <set>
#include <array>
#include <fstream>
#include <deque>
#include <string>

//
//	Include Vulkan Helpers
#include <iostream>
#include <vector>
#include "VulkanInitializers.hpp"
#include "VulkanDevice.hpp"
#include "VulkanFrameBuffer.hpp"
#include "VulkanSwapChain.hpp"

// Texture properties
#define TEX_DIM 2048
#define TEX_FILTER VK_FILTER_LINEAR
#define SHADOWMAP_DIM 2048
#define SHADOWMAP_FORMAT VK_FORMAT_D32_SFLOAT_S8_UINT
#define LIGHT_COUNT 6

// Offscreen frame buffer properties
#define FB_DIM TEX_DIM

class EventReceiver;
namespace Gwen { namespace Renderer { class Vulkan; } }

//
//	Deferred Rendering Uniform Buffer Object
struct DLight {
	glm::vec4 position;
	glm::vec4 color;
	glm::f32 radius;
};
struct DComposition {
	DLight lights[6];
	glm::i32 debugDisplayTarget = 0;
};

//
//	Camera Push Constant
struct CameraPushConstant {
	glm::mat4 view{};
	glm::mat4 proj{};
	glm::vec3 pos{};
};
//
//	Model Uniform Buffer Object
struct UniformBufferObject {
	glm::mat4 model{};
	glm::mat4 bones[128]{};
};

//
//	Vertex
struct Vertex {
	glm::vec3 pos{ 0, 0, 0 };
	glm::vec4 color{ 0, 0, 0, 0 };
	glm::vec2 texCoord{ 0, 0 };
	glm::vec3 normal{ 0, 0, 0 };

	glm::vec4 Bones{ 0, 0, 0, 0 };
	glm::vec4 Weights{ 0, 0, 0, 0 };
	glm::vec4 Tangents{ 0, 0, 0, 0 };

	static std::vector<VkVertexInputBindingDescription> getBindingDescription() {
		std::vector<VkVertexInputBindingDescription> bindingDescription = {};
		bindingDescription.emplace_back();
		bindingDescription[0].binding = 0;
		bindingDescription[0].stride = sizeof(Vertex);
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {};

		attributeDescriptions.emplace_back();
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;		//	Position
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions.emplace_back();
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Color
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		attributeDescriptions.emplace_back();
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;			//	UV
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		attributeDescriptions.emplace_back();
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;		//	Normal
		attributeDescriptions[3].offset = offsetof(Vertex, normal);

		attributeDescriptions.emplace_back();
		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Joint
		attributeDescriptions[4].offset = offsetof(Vertex, Bones);

		attributeDescriptions.emplace_back();
		attributeDescriptions[5].binding = 0;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;	//	Weight
		attributeDescriptions[5].offset = offsetof(Vertex, Weights);

		attributeDescriptions.emplace_back();
		attributeDescriptions[6].binding = 0;
		attributeDescriptions[6].location = 6;
		attributeDescriptions[6].format = VK_FORMAT_R32G32B32_SFLOAT;	//	Tangent
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
	VkDevice _Device = VK_NULL_HANDLE;
	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> DescriptorSets = {};

	DescriptorObject(VkDevice Device) : _Device(Device) {}
	~DescriptorObject() {
		vkDestroyDescriptorPool(_Device, DescriptorPool, nullptr);
	}
};

struct TextureObject {
	VkDevice _Device = VK_NULL_HANDLE;
	VmaAllocator _Allocator = VMA_NULL;
	VmaAllocation Allocation = VMA_NULL;
	VkImage Image = VK_NULL_HANDLE;
	VkImageView ImageView = VK_NULL_HANDLE;
	unsigned int Width = 0;
	unsigned int Height = 0;
	std::vector<unsigned char> Pixels = {};
	bool Empty = true;

	//	ToDo: vector.swap image(p)
	TextureObject(VkDevice Device, VmaAllocator Allocator) : _Device(Device), _Allocator(Allocator) {}
	~TextureObject() {
		if (!Empty) {
			vkDestroyImageView(_Device, ImageView, nullptr);
			vmaDestroyImage(_Allocator, Image, Allocation);
		}
	}
};