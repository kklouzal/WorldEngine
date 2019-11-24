#pragma once


struct PipelineObject {
	VulkanDriver* _Driver;

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline = VK_NULL_HANDLE;

	~PipelineObject() {
		for (auto Tex : Textures) {
			delete Tex;
		}
		vkDestroyPipeline(_Driver->device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(_Driver->device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(_Driver->device, descriptorSetLayout, nullptr);
	}
protected:

	std::vector<TextureObject*> Textures;

	PipelineObject(VulkanDriver* Driver) : _Driver(Driver) {}

	//
	//	Create Descriptor Set Layout
	void createDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo LayoutInfo) {
		if (vkCreateDescriptorSetLayout(_Driver->device, &LayoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
#ifdef _DEBUG
			throw std::runtime_error("failed to create descriptor set layout!");
#endif
		}
	}

	//
	//	Create Pipeline Layout & Graphics Pipeline
	void createPipeline(
		VkPipelineShaderStageCreateInfo shaderStages[],
		VkPipelineVertexInputStateCreateInfo vertexInputInfo,
		VkPipelineInputAssemblyStateCreateInfo inputAssembly,
		VkPipelineViewportStateCreateInfo viewportState,
		VkPipelineRasterizationStateCreateInfo rasterizer,
		VkPipelineMultisampleStateCreateInfo multisampling,
		VkPipelineDepthStencilStateCreateInfo depthStencil,
		VkPipelineColorBlendStateCreateInfo colorBlending,
		VkPipelineDynamicStateCreateInfo dynamicState) {
		VkPipelineLayoutCreateInfo pipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (vkCreatePipelineLayout(_Driver->device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
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

		if (vkCreateGraphicsPipelines(_Driver->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
#ifdef _DEBUG
			throw std::runtime_error("failed to create graphics pipeline!");
#endif
		}
	}

	//
	//
	//	Helper Functions
	//
	//
	std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
#ifdef _DEBUG
			throw std::runtime_error("failed to open file!");
#endif
		}

		size_t fileSize = (size_t)file.tellg();
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
		if (vkCreateShaderModule(_Driver->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
#ifdef _DEBUG
			throw std::runtime_error("failed to create shader module!");
#endif
		}

		return shaderModule;
	}
};

namespace Pipeline {
	struct GUI;
	struct Default;
}