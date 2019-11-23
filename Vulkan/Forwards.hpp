#pragma once

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
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}

struct PipelineObject {
	VulkanDriver* _Driver;

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline = VK_NULL_HANDLE;

	PipelineObject(VulkanDriver* Driver) : _Driver(Driver) {}

	~PipelineObject() {
		vkDestroyPipeline(_Driver->device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(_Driver->device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(_Driver->device, descriptorSetLayout, nullptr);
	}

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
};

struct TextureObject {

	VmaAllocation Allocation = VMA_NULL;
	VkImage Image = VK_NULL_HANDLE;
	VkImageView ImageView = VK_NULL_HANDLE;
	VkSampler Sampler = VK_NULL_HANDLE;
	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> DescriptorSets = {};
	int Width;
	int Height;
	stbi_uc* Pixels;

	VulkanDriver* _Driver;

	TextureObject(VulkanDriver* Driver, int W, int H, stbi_uc* P) : _Driver(Driver), Width(W), Height(H), Pixels(P) {}
	~TextureObject() {
		vkDestroySampler(_Driver->device, Sampler, nullptr);
		vkDestroyImageView(_Driver->device, ImageView, nullptr);
		vmaDestroyImage(_Driver->allocator, Image, Allocation);
		vkDestroyDescriptorPool(_Driver->device, DescriptorPool, nullptr);
		stbi_image_free(Pixels);
	}
};