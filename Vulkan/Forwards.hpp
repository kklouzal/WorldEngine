#pragma once
#define WIN32_LEAN_AND_MEAN

#include "volk.h"
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

#include <KNet.hpp>

#define VMA_IMPLEMENTATION
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_INLINE
#define GLM_FORCE_INTRINSICS
#define GLM_FORCE_SSE2
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#include "TinyGLTF.hpp"

#include <include\cef_app.h>
#include <include\cef_client.h>
#include <include\cef_render_handler.h>

#include "btBulletDynamicsCommon.h"
//#include "Bullet_DebugDraw.hpp"
#include "BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"
#include "BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h"
#include "BulletDynamics/Dynamics/btSimulationIslandManagerMt.h"
#include "BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h"
#include "BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h"

#include "LuaScripting.hpp"

#include "imgui.h"

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

//
//	Include OZZ Headers
#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/local_to_model_job.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/animation/runtime/skeleton_utils.h"

#include "ozz/animation/offline/additive_animation_builder.h"
#include "ozz/animation/offline/animation_builder.h"
#include "ozz/animation/offline/animation_optimizer.h"
#include "ozz/animation/offline/raw_animation.h"
#include "ozz/animation/offline/raw_skeleton.h"
#include "ozz/animation/offline/skeleton_builder.h"

#include "ozz/base/maths/box.h"
#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/simd_quaternion.h"
#include "ozz/base/maths/soa_transform.h"
#include "ozz/base/maths/vec_float.h"

#include "ozz/base/containers/vector.h"
#include "ozz/base/io/archive.h"
#include "ozz/base/io/stream.h"

//#include "ozz_utils.h"
//s#include "ozz_mesh.h"

#include <ozz/options/options.h>

// Texture properties
constexpr auto FB_DIM = 2048;
constexpr auto TEX_DIM = 2048;
constexpr auto SHADOWMAP_DIM = 2048;
constexpr auto SHADOWMAP_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT;
constexpr auto TEX_FILTER = VK_FILTER_LINEAR;
constexpr auto LIGHT_COUNT = 6;

//	Vertex definition
#include "Vertex.hpp"

// Offscreen frame buffer properties

class EventReceiver;
namespace Gwen { namespace Renderer { class Vulkan; } }

//
//	Deferred Rendering Uniform Buffer Object
struct DLight {
	glm::vec4 position;
	glm::vec4 target;
	glm::vec4 color;
	glm::mat4 viewMatrix;
};
struct DComposition {
	DLight lights[LIGHT_COUNT];
	glm::i32 debugDisplayTarget = 0;
};
struct DShadow {
	glm::mat4 mvp[LIGHT_COUNT];
	glm::mat4 instancePos[1024];
};

//
//	Camera Push Constant (can only hold a maximum of 2 mat4's which is 8 vec4's)
struct CameraPushConstant {
	glm::mat4 view_proj{};
	glm::vec4 pos;
};
//
//	Model Uniform Buffer Object
struct UniformBufferObject {
	glm::mat4 model{};
	//ozz::math::Float4x4 bones[32]{};
};

//
struct InstanceData {
	glm::mat4 model;
};

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
	uint32_t Width = 0;
	uint32_t Height = 0;
	std::vector<unsigned char> Pixels = {};

	//	ToDo: vector.swap image(p)
	TextureObject(VkDevice Device, VmaAllocator Allocator) : _Device(Device), _Allocator(Allocator) {}
	~TextureObject() {
		vkDestroyImageView(_Device, ImageView, nullptr);
		vmaDestroyImage(_Allocator, Image, Allocation);
	}
};

void setImageLayout(
	VkCommandBuffer cmdbuffer,
	VkImage image,
	VkImageLayout oldImageLayout,
	VkImageLayout newImageLayout,
	VkImageSubresourceRange subresourceRange,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask)
{
	// Create an image barrier object
	VkImageMemoryBarrier imageMemoryBarrier = vks::initializers::imageMemoryBarrier();
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		// Image layout is undefined (or does not matter)
		// Only valid as initial layout
		// No flags required, listed only for completeness
		imageMemoryBarrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image is a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image is a depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image is a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image is a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image is read by a shader
		// Make sure any shader reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (newImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image will be used as a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (imageMemoryBarrier.srcAccessMask == 0)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	// Put barrier inside setup command buffer
	vkCmdPipelineBarrier(
		cmdbuffer,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);
}

void setImageLayout(
	VkCommandBuffer cmdbuffer,
	VkImage image,
	VkImageAspectFlags aspectMask,
	VkImageLayout oldImageLayout,
	VkImageLayout newImageLayout,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask)
{
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = aspectMask;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;
	setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
}

glm::mat4 to_mat4(const ozz::math::Float4x4& m) {
	glm::mat4 result{};
	for (uint8_t i{ 0 }; i < 4; ++i)
		ozz::math::StorePtr(m.cols[i], &result[i].x);
	return result;
}

class Plane
{

public:
	Plane() {}

	Plane(const glm::vec4& abcd)
		: normal(abcd.x, abcd.y, abcd.z), d(abcd.w) {}

	void normalize()
	{
		float mag = glm::length(normal);
		normal /= mag;
		d /= mag;
	}

	glm::vec3 normal;
	float d;        // distance from origin
};

class Frustum
{
public:
	Frustum(const glm::mat4& mat, bool normalize_planes = true)
		// create frustum from  matrix
		// if extracted from projection matrix only, planes will be in eye-space
		// if extracted from view*projection, planes will be in world space
		// if extracted from model*view*projection planes will be in model space
	{
		// create non-normalized clipping planes
		planes[0] = Plane(mat[3] - mat[0]);       // right
		planes[1] = Plane(mat[3] + mat[0]);       // left
		planes[2] = Plane(mat[3] - mat[1]);       // top
		planes[3] = Plane(mat[3] + mat[1]);       // bottom
		planes[4] = Plane(mat[3] - mat[2]);       // far
		planes[5] = Plane(mat[3] + mat[2]);       // near
		// normalize the plane equations, if requested
		if (normalize_planes)
		{
			planes[0].normalize();
			planes[1].normalize();
			planes[2].normalize();
			planes[3].normalize();
			planes[4].normalize();
			planes[5].normalize();
		}
	}

	Plane planes[6];        // plane normals point into frustum
};