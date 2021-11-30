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

	PipelineObject(VulkanDriver* Driver) : _Driver(Driver) {}

	//
	//	Create Descriptor Set Layout
	void createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo &LayoutInfo) {
		if (vkCreateDescriptorSetLayout(_Driver->_VulkanDevice->logicalDevice, &LayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
#ifdef _DEBUG
			throw std::runtime_error("failed to create descriptor set layout!");
#endif
		}
	}

	//
	//	Create Pipeline Layout & Graphics Pipeline
	void createPipeline(
		const VkPipelineShaderStageCreateInfo shaderStages[],
		const VkPipelineVertexInputStateCreateInfo &vertexInputInfo,
		const VkPipelineInputAssemblyStateCreateInfo &inputAssembly,
		const VkPipelineViewportStateCreateInfo &viewportState,
		const VkPipelineRasterizationStateCreateInfo &rasterizer,
		const VkPipelineMultisampleStateCreateInfo &multisampling,
		const VkPipelineDepthStencilStateCreateInfo &depthStencil,
		const VkPipelineColorBlendStateCreateInfo &colorBlending,
		const VkPipelineDynamicStateCreateInfo &dynamicState) {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (vkCreatePipelineLayout(_Driver->_VulkanDevice->logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
#ifdef _DEBUG
			throw std::runtime_error("failed to create pipeline layout!");
#endif
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = _Driver->renderPass;
		pipelineInfo.subpass = 0;

		if (vkCreateGraphicsPipelines(_Driver->_VulkanDevice->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
#ifdef _DEBUG
			throw std::runtime_error("failed to create graphics pipeline!");
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
	struct Skinned;
}