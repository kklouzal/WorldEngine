#pragma once

#include "lodepng.h"

struct PipelineObject {
	VulkanDriver* _Driver;

	VkSampler Sampler = VK_NULL_HANDLE;
	VkSampler DeferredSampler = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline = VK_NULL_HANDLE;

	virtual ~PipelineObject() {
		printf("\tDestroy Base PipelineObject\n");
		for (auto Tex : _Textures) {
			delete Tex.second;
		}
		for (auto Tex : _Textures2) {
			delete Tex;
		}
		vkDestroyPipeline(_Driver->_VulkanDevice->logicalDevice, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(_Driver->_VulkanDevice->logicalDevice, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(_Driver->_VulkanDevice->logicalDevice, descriptorSetLayout, nullptr);
		vkDestroySampler(_Driver->_VulkanDevice->logicalDevice, Sampler, nullptr);
		vkDestroySampler(_Driver->_VulkanDevice->logicalDevice, DeferredSampler, nullptr);
	}

	virtual TextureObject* createTextureImage(const std::string& File) = 0;
	virtual TextureObject* createTextureImage2(tinygltf::Image& ImgData) { return nullptr; };

	void EmptyCache() {
		for (auto Tex : _Textures) {
			delete Tex.second;
		}
		_Textures.clear();
		for (auto Tex : _Textures2) {
			delete Tex;
		}
		_Textures2.clear();
	}

protected:

	std::unordered_map<std::string, TextureObject*> _Textures;
	std::deque<TextureObject*> _Textures2;

	PipelineObject(VulkanDriver* Driver) : _Driver(Driver)
	{
		//
		//	Image Sampler For Entire Pipeline
		VkSamplerCreateInfo samplerInfo1 = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo1.magFilter = VK_FILTER_LINEAR;
		samplerInfo1.minFilter = VK_FILTER_LINEAR;
		samplerInfo1.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo1.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo1.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo1.anisotropyEnable = VK_TRUE;
		samplerInfo1.maxAnisotropy = 16;
		samplerInfo1.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo1.unnormalizedCoordinates = VK_FALSE;
		samplerInfo1.compareEnable = VK_FALSE;
		samplerInfo1.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo1.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo1.mipLodBias = 0.0f;
		samplerInfo1.minLod = 0.0f;
		samplerInfo1.maxLod = 0.0f;
		if (vkCreateSampler(_Driver->_VulkanDevice->logicalDevice, &samplerInfo1, nullptr, &Sampler) != VK_SUCCESS) {
#ifdef _DEBUG
			throw std::runtime_error("failed to create texture sampler!");
#endif
		}

		VkSamplerCreateInfo samplerInfo2 = vks::initializers::samplerCreateInfo();
		samplerInfo2.magFilter = VK_FILTER_NEAREST;
		samplerInfo2.minFilter = VK_FILTER_NEAREST;
		samplerInfo2.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo2.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo2.addressModeV = samplerInfo2.addressModeU;
		samplerInfo2.addressModeW = samplerInfo2.addressModeU;
		samplerInfo2.mipLodBias = 0.0f;
		samplerInfo2.maxAnisotropy = 1.0f;
		samplerInfo2.minLod = 0.0f;
		samplerInfo2.maxLod = 1.0f;
		samplerInfo2.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		if (vkCreateSampler(_Driver->_VulkanDevice->logicalDevice, &samplerInfo2, nullptr, &DeferredSampler) != VK_SUCCESS) {
			#ifdef _DEBUG
			throw std::runtime_error("failed to create texture sampler!");
			#endif
		}
	}

	//
	//	Helper Functions
	std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
#ifdef _DEBUG
			throw std::runtime_error("failed to open file!");
#endif
		}

		const size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	VkShaderModule createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(_Driver->_VulkanDevice->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
#ifdef _DEBUG
			throw std::runtime_error("failed to create shader module!");
#endif
		}

		return shaderModule;
	}
};

namespace Pipeline {
	struct Default;
	struct GUI;
}