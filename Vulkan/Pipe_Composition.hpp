#pragma once

namespace Pipeline {
	struct Composition : public PipelineObject
	{
		std::vector<VkDescriptorSet> DescriptorSets = {};
		VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
		//
		std::array<VkClearValue, 6> clearValues;
		VkViewport viewport;
		VkRect2D scissor;
		//
		std::vector<VkRenderPassBeginInfo> renderPass = {};

		~Composition()
		{
			//
			vkDestroyDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, DescriptorPool, nullptr);
		}

		Composition(VkPipelineCache PipelineCache)
			: PipelineObject()
		{
			viewport = vks::initializers::viewport((float)WorldEngine::VulkanDriver::WIDTH, (float)WorldEngine::VulkanDriver::HEIGHT, 0.0f, 1.0f);
			scissor = vks::initializers::rect2D(WorldEngine::VulkanDriver::WIDTH, WorldEngine::VulkanDriver::HEIGHT, 0, 0);
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
			clearValues[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
			clearValues[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
			clearValues[3].color = { 0.0f, 0.0f, 0.0f, 0.0f };
			clearValues[4].depthStencil = { 1.0f, 0 };
			clearValues[5].depthStencil = { 1.0f, 0 };
			//
			//
			//	DescriptorSetLayout
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				//	Binding 0 : Position input attachment
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
				//	Binding 1 : Normal input attachment
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
				//	Binding 2 : Albedo input attachment
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
				//	Binding 3: Shadow map
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
				//	Binding 4 : Fragment UBO
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 4)
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
			pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
			pipelineCI.pStages = shaderStages.data();
			pipelineCI.subpass = 2;	//	Subpass
			//
			rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
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
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 1),
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 4),
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 4)
			};
			VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, (uint32_t)SwapChainCount);
			VK_CHECK_RESULT(vkCreateDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &descriptorPoolInfo, nullptr, &DescriptorPool));
			//
			//	Create and Update individual Descriptor sets and uniform buffers
			DescriptorSets.resize(SwapChainCount);
			renderPass.resize(SwapChainCount);
			for (size_t i = 0; i < SwapChainCount; i++)
			{
				VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(DescriptorPool, &descriptorSetLayout, 1);
				VK_CHECK_RESULT(vkAllocateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &allocInfo, &DescriptorSets[i]));
				// Image descriptors for the offscreen color attachments
				VkDescriptorImageInfo texDescriptorPosition =
					vks::initializers::descriptorImageInfo(
						DeferredSampler,
						WorldEngine::VulkanDriver::attachments/*[i]*/.position.view,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				VkDescriptorImageInfo texDescriptorNormal =
					vks::initializers::descriptorImageInfo(
						DeferredSampler,
						WorldEngine::VulkanDriver::attachments/*[i]*/.normal.view,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				VkDescriptorImageInfo texDescriptorAlbedo =
					vks::initializers::descriptorImageInfo(
						DeferredSampler,
						WorldEngine::VulkanDriver::attachments/*[i]*/.albedo.view,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				VkDescriptorImageInfo texDescriptorShadowMap =
					vks::initializers::descriptorImageInfo(
						ShadowSampler,
						WorldEngine::VulkanDriver::attachments.shadow.view,
						VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

				VkDescriptorBufferInfo bufferInfo_composition = {};
				bufferInfo_composition.buffer = WorldEngine::VulkanDriver::uboCompositionBuff[i];
				bufferInfo_composition.offset = 0;
				bufferInfo_composition.range = sizeof(DComposition);

				std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
					// Binding 0 : Position texture target
					vks::initializers::writeDescriptorSet(DescriptorSets[i], VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0, &texDescriptorPosition),
					// Binding 1 : Normals texture target
					vks::initializers::writeDescriptorSet(DescriptorSets[i], VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1, &texDescriptorNormal),
					// Binding 2 : Albedo texture target
					vks::initializers::writeDescriptorSet(DescriptorSets[i], VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2, &texDescriptorAlbedo),
					// Binding 3: Shadow map
					vks::initializers::writeDescriptorSet(DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &texDescriptorShadowMap),
					// Binding 4 : Fragment shader uniform buffer
					vks::initializers::writeDescriptorSet(DescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &bufferInfo_composition)
				};
				vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

				//
				//	Render Pass Info
				renderPass[i] = vks::initializers::renderPassBeginInfo();
				renderPass[i].renderPass = WorldEngine::VulkanDriver::renderPass;
				renderPass[i].renderArea.offset = { 0, 0 };
				renderPass[i].renderArea.extent = { WorldEngine::VulkanDriver::WIDTH, WorldEngine::VulkanDriver::HEIGHT };
				renderPass[i].clearValueCount = static_cast<uint32_t>(clearValues.size());;
				renderPass[i].pClearValues = clearValues.data();
				renderPass[i].framebuffer = WorldEngine::VulkanDriver::frameBuffers_Main[i];
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
				inheritanceInfo.subpass = 2;
				//
				//	Secondary CommandBuffer Begin Info
				VkCommandBufferBeginInfo commandBufferBeginInfo = vks::initializers::commandBufferBeginInfo();
				commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
				commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				//
				//	Begin recording state
				VK_CHECK_RESULT(vkBeginCommandBuffer(CommandBuffers[i], &commandBufferBeginInfo));
				//
				//	Draw our combined image view over the entire screen
				vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
				vkCmdSetViewport(CommandBuffers[i], 0, 1, &WorldEngine::VulkanDriver::viewport_Deferred);
				vkCmdSetScissor(CommandBuffers[i], 0, 1, &WorldEngine::VulkanDriver::scissor_Deferred);
				vkCmdBindDescriptorSets(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &DescriptorSets[i], 0, nullptr);
				vkCmdDraw(CommandBuffers[i], 3, 1, 0, 0);
				//
				//	End recording state
				VK_CHECK_RESULT(vkEndCommandBuffer(CommandBuffers[i]));
			}
		}
	};
}