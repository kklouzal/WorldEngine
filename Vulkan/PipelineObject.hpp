#pragma once

#include "lodepng.h"

struct PipelineObject {
	VulkanDriver* _Driver;

	VkSampler Sampler = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline = VK_NULL_HANDLE;

	~PipelineObject() {
		printf("\tDestroy Base PipelineObject\n");
		for (auto Tex : _Textures) {
			delete Tex.second;
		}
		vkDestroyPipeline(_Driver->_VulkanDevice->logicalDevice, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(_Driver->_VulkanDevice->logicalDevice, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(_Driver->_VulkanDevice->logicalDevice, descriptorSetLayout, nullptr);
		vkDestroySampler(_Driver->_VulkanDevice->logicalDevice, Sampler, nullptr);
	}

	virtual DescriptorObject* createDescriptor(const TextureObject* Texture, const std::vector<VkBuffer>& UniformBuffers) = 0;
	virtual TextureObject* createTextureImage(const std::string& File) = 0;

	void EmptyCache() {
		for (auto Tex : _Textures) {
			delete Tex.second;
		}
		_Textures.clear();
	}

protected:

	std::unordered_map<std::string, TextureObject*> _Textures;

	PipelineObject(VulkanDriver* Driver) : _Driver(Driver)
	{
		//
		//	Image Sampler For Entire Pipeline
		VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		if (vkCreateSampler(_Driver->_VulkanDevice->logicalDevice, &samplerInfo, nullptr, &Sampler) != VK_SUCCESS) {
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