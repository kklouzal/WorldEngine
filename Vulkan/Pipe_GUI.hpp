#pragma once

namespace Pipeline {
	struct GUI : public PipelineObject
	{
		GUI(VkPipelineCache PipelineCache) : PipelineObject()
		{
			//
			//	DescriptorSetLayout
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
			};
			VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
			if (vkCreateDescriptorSetLayout(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			{
				#ifdef _DEBUG
				throw std::runtime_error("failed to create descriptor set layout!");
				#endif
			}

			//
			//	Pipeline Layout
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
			if (vkCreatePipelineLayout(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			{
				#ifdef _DEBUG
				throw std::runtime_error("failed to create pipeline layout!");
				#endif
			}

			//
			//	Graphics Pipeline
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
			VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
			VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_TRUE);
			// Enable blending
			blendAttachmentState.blendEnable = VK_TRUE;
			blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
			blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			//
			VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
			VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
			VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
			VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
			std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_SCISSOR };
			VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

			std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {};

			VkPipelineShaderStageCreateInfo vertShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = createShaderModule(readFile("shaders/Vertex_GUI.vert.spv"));
			vertShaderStageInfo.pName = "main";
			shaderStages[0] = vertShaderStageInfo;
			VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = createShaderModule(readFile("shaders/Fragment_GUI.frag.spv"));
			fragShaderStageInfo.pName = "main";
			shaderStages[1] = fragShaderStageInfo;

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)WorldEngine::VulkanDriver::WIDTH;
			viewport.height = (float)WorldEngine::VulkanDriver::HEIGHT;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			viewportState.pViewports = &viewport;

			rasterizationState.depthClampEnable = VK_FALSE;
			rasterizationState.rasterizerDiscardEnable = VK_FALSE;
			rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationState.lineWidth = 1.0f;
			rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizationState.depthBiasEnable = VK_FALSE;

			VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(pipelineLayout, WorldEngine::VulkanDriver::renderPass);
			pipelineCI.pInputAssemblyState = &inputAssemblyState;
			pipelineCI.pRasterizationState = &rasterizationState;
			pipelineCI.pColorBlendState = &colorBlendState;
			pipelineCI.pMultisampleState = &multisampleState;
			pipelineCI.pViewportState = &viewportState;
			pipelineCI.pDepthStencilState = &depthStencilState;
			pipelineCI.pDynamicState = &dynamicState;
			pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
			pipelineCI.pStages = shaderStages.data();
			pipelineCI.subpass = 3;	//	Subpass
			//
			rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;

			auto binding = Vertex::getBindingDescription_Simple();
			auto description = Vertex::getAttributeDescriptions_Simple();
			VkPipelineVertexInputStateCreateInfo vertexInputInfo = vks::initializers::pipelineVertexInputStateCreateInfo(binding, description);
			pipelineCI.pVertexInputState = &vertexInputInfo;

			vkCreateGraphicsPipelines(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, PipelineCache, 1, &pipelineCI, nullptr, &graphicsPipeline);
			//
			//	Cleanup Shader Modules
			vkDestroyShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, vertShaderStageInfo.module, nullptr);
			vkDestroyShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, fragShaderStageInfo.module, nullptr);
		}
		//
		//
		//	Create Descriptor
		//
		//
		DescriptorObject* createDescriptor(const TextureObject* Texture, const std::vector<VkBuffer>& UniformBuffers = std::vector<VkBuffer>()) {
			DescriptorObject* NewDescriptor = new DescriptorObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice);
			//
			//	Create Descriptor Pool
			std::vector<VkDescriptorPoolSize> poolSizes = {
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size())
			};
			VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size());
			if (vkCreateDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &descriptorPoolInfo, nullptr, &NewDescriptor->DescriptorPool) != VK_SUCCESS) {
				#ifdef _DEBUG
				throw std::runtime_error("failed to create descriptor pool!");
				#endif
			}
			//
			//	Create Descriptor Sets
			std::vector<VkDescriptorSetLayout> layouts(WorldEngine::VulkanDriver::swapChain.images.size(), descriptorSetLayout);
			VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(NewDescriptor->DescriptorPool, layouts.data(), (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size());
			NewDescriptor->DescriptorSets.resize(WorldEngine::VulkanDriver::swapChain.images.size());
			if (vkAllocateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &allocInfo, NewDescriptor->DescriptorSets.data()) != VK_SUCCESS) {
				#ifdef _DEBUG
				throw std::runtime_error("failed to allocate descriptor sets!");
				#endif
			}
			//
			//	Update individual sets
			for (size_t i = 0; i < WorldEngine::VulkanDriver::swapChain.images.size(); i++)
			{
				VkDescriptorImageInfo textureImage =
					vks::initializers::descriptorImageInfo(
						Sampler,
						Texture->ImageView,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				std::vector<VkWriteDescriptorSet> writeDescriptorSets;
				writeDescriptorSets = {
					// Binding 1 : texture
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &textureImage)
				};
				vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
			}

			return NewDescriptor;
		}

		DescriptorObject* createRawDescriptor(const VkImageView Image, const VkSampler Sampler) {
			DescriptorObject* NewDescriptor = new DescriptorObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice);

			std::array<VkDescriptorPoolSize, 1> poolSizes = {};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[0].descriptorCount = static_cast<uint32_t>(WorldEngine::VulkanDriver::swapChain.images.size());

			VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<uint32_t>(WorldEngine::VulkanDriver::swapChain.images.size());

			if (vkCreateDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &poolInfo, nullptr, &NewDescriptor->DescriptorPool) != VK_SUCCESS) {
#ifdef _DEBUG
				throw std::runtime_error("failed to create descriptor pool!");
#endif
			}

			std::vector<VkDescriptorSetLayout> layouts(WorldEngine::VulkanDriver::swapChain.images.size(), descriptorSetLayout);
			VkDescriptorSetAllocateInfo allocInfoDS = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			allocInfoDS.descriptorPool = NewDescriptor->DescriptorPool;
			allocInfoDS.descriptorSetCount = static_cast<uint32_t>(WorldEngine::VulkanDriver::swapChain.images.size());
			allocInfoDS.pSetLayouts = layouts.data();

			NewDescriptor->DescriptorSets.resize(WorldEngine::VulkanDriver::swapChain.images.size());
			if (vkAllocateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &allocInfoDS, NewDescriptor->DescriptorSets.data()) != VK_SUCCESS) {
#ifdef _DEBUG
				throw std::runtime_error("failed to allocate descriptor sets!");
#endif
			}

			for (size_t i = 0; i < WorldEngine::VulkanDriver::swapChain.images.size(); i++) {

				VkDescriptorImageInfo imageInfo = {};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = Image;
				imageInfo.sampler = Sampler;

				std::array<VkWriteDescriptorSet, 1> descriptorWrites = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

				descriptorWrites[0].dstSet = NewDescriptor->DescriptorSets[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pImageInfo = &imageInfo;

				vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}

			return NewDescriptor;
		}
	};
}