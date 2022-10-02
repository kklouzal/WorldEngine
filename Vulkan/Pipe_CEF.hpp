#pragma once

namespace Pipeline {
	struct CEF : public PipelineObject
	{
		std::vector<VkDescriptorSet> DescriptorSets_Composition = {};
		VkDescriptorPool DescriptorPool_Composition = VK_NULL_HANDLE;
		//
		VkViewport viewport;
		VkRect2D scissor;

		~CEF()
		{
			vkDestroyDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, DescriptorPool_Composition, nullptr);
		}

		CEF(VkPipelineCache PipelineCache)
			: PipelineObject()
		{
			viewport = vks::initializers::viewport((float)WorldEngine::VulkanDriver::WIDTH, (float)WorldEngine::VulkanDriver::HEIGHT, 0.0f, 1.0f);
			scissor = vks::initializers::rect2D(WorldEngine::VulkanDriver::WIDTH, WorldEngine::VulkanDriver::HEIGHT, 0, 0);
			//
			//	DescriptorSetLayout
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				//	Binding 0 : CEF Image Sampler
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
			};
			VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &descriptorLayout, nullptr, &descriptorSetLayout));

			//
			//	Pipeline Layout
			VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptorSetLayout, 1);
			VK_CHECK_RESULT(vkCreatePipelineLayout(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout));

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
			pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
			pipelineCI.pStages = shaderStages.data();
			pipelineCI.subpass = 3;	//	Subpass
			//
			rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
			//
			//	Load shader files
			VkPipelineShaderStageCreateInfo vertShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = createShaderModule(readFile("shaders/cef.vert.spv"));
			vertShaderStageInfo.pName = "main";
			shaderStages[0] = vertShaderStageInfo;
			VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = createShaderModule(readFile("shaders/cef.frag.spv"));
			fragShaderStageInfo.pName = "main";
			shaderStages[1] = fragShaderStageInfo;
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
			//	CEF Descriptor
			//
			//	Create Descriptor Pool
			std::vector<VkDescriptorPoolSize> poolSizes = {
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 4)
			};
			VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size());
			VK_CHECK_RESULT(vkCreateDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &descriptorPoolInfo, nullptr, &DescriptorPool_Composition));
			//
			//	Create and Update individual Descriptor sets
			DescriptorSets_Composition.resize(WorldEngine::VulkanDriver::swapChain.images.size());
			for (size_t i = 0; i < WorldEngine::VulkanDriver::swapChain.images.size(); i++)
			{
				VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(DescriptorPool_Composition, &descriptorSetLayout, 1);
				VK_CHECK_RESULT(vkAllocateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &allocInfo, &DescriptorSets_Composition[i]));
				// Image descriptors for the offscreen color attachments
				VkDescriptorImageInfo texDescriptorCEF =
					vks::initializers::descriptorImageInfo(
						WorldEngine::CEF::sampler,
						WorldEngine::CEF::CEFTex->ImageView,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
					vks::initializers::writeDescriptorSet(DescriptorSets_Composition[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &texDescriptorCEF),
				};
				vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
			}
		}

		void ResetCommandPools(std::vector<VkCommandBuffer>& CommandBuffers)
		{
			for (size_t i = 0; i < CommandBuffers.size(); i++)
			{
				//
				//	Secondary CommandBuffer Inheritance Info
				VkCommandBufferInheritanceInfo inheritanceInfo = vks::initializers::commandBufferInheritanceInfo();
				inheritanceInfo.renderPass = WorldEngine::VulkanDriver::renderPass;
				inheritanceInfo.framebuffer = WorldEngine::VulkanDriver::frameBuffers_Main[i];
				inheritanceInfo.subpass = 3;
				//
				//	Secondary CommandBuffer Begin Info
				VkCommandBufferBeginInfo commandBufferBeginInfo = vks::initializers::commandBufferBeginInfo();
				commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
				//
				//	Begin recording state
				VK_CHECK_RESULT(vkBeginCommandBuffer(CommandBuffers[i], &commandBufferBeginInfo));
				vkCmdSetViewport(CommandBuffers[i], 0, 1, &viewport);
				vkCmdSetScissor(CommandBuffers[i], 0, 1, &scissor);
				//
				//	Draw CEF fullscreen triangle
				vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
				vkCmdBindDescriptorSets(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &DescriptorSets_Composition[i], 0, nullptr);
				vkCmdDraw(CommandBuffers[i], 3, 1, 0, 0);
				//
				//	End recording state
				VK_CHECK_RESULT(vkEndCommandBuffer(CommandBuffers[i]));
			}
		}
	};
}