#pragma once

namespace Pipeline {
	struct Composition : public PipelineObject
	{
		std::vector<VkDescriptorSet> DescriptorSets = {};
		VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;

		~Composition()
		{
			//
			vkDestroyDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, DescriptorPool, nullptr);
		}

		Composition(VkPipelineCache PipelineCache)
			: PipelineObject()
		{
			//
			//
			//	DescriptorSetLayout
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				//	Binding 0 : Position/Color texture target
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
				//	Binding 1 : Normal texture target
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
				//	Binding 2 : Albedo texture target
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
				//	Binding 3 : Fragment UBO
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
				//	Binding 4: Shadow map
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4)
			};
			VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout));

			//
			//	Pipeline Layout
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
			VkPushConstantRange push_constant;
			push_constant.offset = 0;
			push_constant.size = sizeof(CameraPushConstant);
			push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pipelineLayoutCreateInfo.pPushConstantRanges = &push_constant;
			pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
			VK_CHECK_RESULT(vkCreatePipelineLayout(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

			//
			//	Graphics Pipeline
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
			VkPipelineRasterizationStateCreateInfo rasterizationState = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
			VkPipelineColorBlendAttachmentState blendAttachmentState = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
			VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);
			VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
			VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);
			VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT, 0);
			std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

			std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = {};

			VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(pipelineLayout, WorldEngine::VulkanDriver::renderPass);
			pipelineCI.pInputAssemblyState = &inputAssemblyState;
			pipelineCI.pRasterizationState = &rasterizationState;
			pipelineCI.pColorBlendState = &colorBlendState;
			pipelineCI.pMultisampleState = &multisampleState;
			pipelineCI.pViewportState = &viewportState;
			pipelineCI.pDepthStencilState = &depthStencilState;
			pipelineCI.pDynamicState = &dynamicState;
			//
			//	Load shader files
			VkPipelineShaderStageCreateInfo vertShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = createShaderModule(readFile("shaders/deferred.vert.spv"));
			vertShaderStageInfo.pName = "main";
			shaderStages[0] = vertShaderStageInfo;
			VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = createShaderModule(readFile("shaders/deferred.frag.spv"));
			fragShaderStageInfo.pName = "main";
			shaderStages[1] = fragShaderStageInfo;
			//
			pipelineCI.pStages = shaderStages.data();
			pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
			//
			rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
			//	Empty vertex input state, vertices are generated by the vertex shader
			VkPipelineVertexInputStateCreateInfo emptyInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
			pipelineCI.pVertexInputState = &emptyInputState;
			//	Create composite graphics pipeline
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, PipelineCache, 1, &pipelineCI, nullptr, &graphicsPipeline));
			//	Cleanup Shader Modules
			vkDestroyShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, vertShaderStageInfo.module, nullptr);
			vkDestroyShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, fragShaderStageInfo.module, nullptr);
			//
			//
			//	Descriptors
			const size_t SwapChainCount = WorldEngine::VulkanDriver::swapChain.images.size();
			//
			//	Create Descriptor Pool
			std::vector<VkDescriptorPoolSize> poolSizes = {
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 3),
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 4)
			};
			VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, (uint32_t)SwapChainCount);
			VK_CHECK_RESULT(vkCreateDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &descriptorPoolInfo, nullptr, &DescriptorPool));
			//
			//	Create and Update individual Descriptor sets and uniform buffers
			DescriptorSets.resize(SwapChainCount);
			for (size_t i = 0; i < SwapChainCount; i++)
			{
				VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(DescriptorPool, &descriptorSetLayout, 1);
				VK_CHECK_RESULT(vkAllocateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &allocInfo, &DescriptorSets[i]));
				// Image descriptors for the offscreen color attachments
				VkDescriptorImageInfo texDescriptorPosition =
					vks::initializers::descriptorImageInfo(
						DeferredSampler,
						WorldEngine::VulkanDriver::frameBuffers.deferred->attachments[0].view,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				VkDescriptorImageInfo texDescriptorNormal =
					vks::initializers::descriptorImageInfo(
						DeferredSampler,
						WorldEngine::VulkanDriver::frameBuffers.deferred->attachments[1].view,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				VkDescriptorImageInfo texDescriptorAlbedo =
					vks::initializers::descriptorImageInfo(
						DeferredSampler,
						WorldEngine::VulkanDriver::frameBuffers.deferred->attachments[2].view,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				VkDescriptorBufferInfo bufferInfo_composition = {};
				bufferInfo_composition.buffer = WorldEngine::VulkanDriver::uboCompositionBuff[i];
				bufferInfo_composition.offset = 0;
				bufferInfo_composition.range = sizeof(DComposition);

				VkDescriptorImageInfo texDescriptorShadowMap =
					vks::initializers::descriptorImageInfo(
						WorldEngine::VulkanDriver::frameBuffers.shadow->sampler,
						WorldEngine::VulkanDriver::frameBuffers.shadow->attachments[0].view,
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

				std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
					// Binding 2 : Position texture target
					vks::initializers::writeDescriptorSet(DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &texDescriptorPosition),
					// Binding 3 : Normals texture target
					vks::initializers::writeDescriptorSet(DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &texDescriptorNormal),
					// Binding 4 : Albedo texture target
					vks::initializers::writeDescriptorSet(DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &texDescriptorAlbedo),
					// Binding 5 : Fragment shader uniform buffer
					vks::initializers::writeDescriptorSet(DescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3, &bufferInfo_composition),
					// Binding 6: Shadow map
					vks::initializers::writeDescriptorSet(DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &texDescriptorShadowMap)
				};
				vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

			}
		}
	};
}