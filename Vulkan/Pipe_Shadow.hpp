#pragma once

namespace Pipeline {
	struct Shadow : public PipelineObject
	{
		std::vector<VkDescriptorSet> DescriptorSets = {};
		VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
		//
		//	Per-Frame Shadow Rendering Uniform Buffer Objects
		DShadow uboShadow;										//	Doesnt Need Cleanup
		std::vector<VkBuffer> uboShadowBuff = {};				//	Cleaned Up
		std::vector<VmaAllocation> uboShadowAlloc = {};			//	Cleaned Up
		//
		VkViewport viewport;
		VkRect2D scissor;
		std::array<VkClearValue, 4> clearValues = {};
		//
		std::vector<VkRenderPassBeginInfo> renderPass = {};

		~Shadow()
		{
			//
			for (int i = 0; i < uboShadowBuff.size(); i++)
			{
				vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, uboShadowBuff[i], uboShadowAlloc[i]);
			}
			//
			vkDestroyDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, DescriptorPool, nullptr);
		}

		Shadow(VkPipelineCache PipelineCache)
			: PipelineObject()
		{
			//
			viewport = vks::initializers::viewport((float)SHADOWMAP_DIM, (float)SHADOWMAP_DIM, 0.0f, 1.0f);
			scissor = vks::initializers::rect2D(SHADOWMAP_DIM, SHADOWMAP_DIM, 0, 0);
			clearValues[0].depthStencil = { 1.0f, 0 };
			//
			//
			//	DescriptorSetLayout
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				//	Binding 1: Geometry Uniform Buffer
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_GEOMETRY_BIT, 0)
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
			//
			//	Load shader files
			VkPipelineShaderStageCreateInfo vertShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = createShaderModule(readFile("shaders/shadow.vert.spv"));
			vertShaderStageInfo.pName = "main";
			shaderStages[0] = vertShaderStageInfo;
			VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
			fragShaderStageInfo.module = createShaderModule(readFile("shaders/shadow.geom.spv"));
			fragShaderStageInfo.pName = "main";
			shaderStages[1] = fragShaderStageInfo;
			//
			pipelineCI.pStages = shaderStages.data();
			pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
			//
			// Shadow pass doesn't use any color attachments
			colorBlendState.attachmentCount = 0;
			colorBlendState.pAttachments = nullptr;
			// Cull front faces
			rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
			depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			// Enable depth bias
			rasterizationState.depthBiasEnable = VK_TRUE;
			// Add depth bias to dynamic state, so we can change it at runtime
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
			dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);
			//	Bind vertex input
			auto binding = Vertex::getBindingDescription();
			auto description = Vertex::getAttributeDescriptions_Shadow();
			VkPipelineVertexInputStateCreateInfo vertexInputInfo = vks::initializers::pipelineVertexInputStateCreateInfo(binding, description);
			pipelineCI.pVertexInputState = &vertexInputInfo;
			//
			pipelineCI.renderPass = WorldEngine::VulkanDriver::frameBuffers.shadow->renderPass;
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
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t)SwapChainCount * 4)
			};
			VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, (uint32_t)SwapChainCount);
			VK_CHECK_RESULT(vkCreateDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &descriptorPoolInfo, nullptr, &DescriptorPool));
			//
			//	Create and Update individual Descriptor sets and uniform buffers
			DescriptorSets.resize(SwapChainCount);
			uboShadowBuff.resize(SwapChainCount);
			uboShadowAlloc.resize(SwapChainCount);
			renderPass.resize(SwapChainCount);
			for (size_t i = 0; i < SwapChainCount; i++)
			{
				VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(DescriptorPool, &descriptorSetLayout, 1);
				VK_CHECK_RESULT(vkAllocateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &allocInfo, &DescriptorSets[i]));

				//
				//	Uniform Buffer Creation
				VkBufferCreateInfo uniformBufferInfo = vks::initializers::bufferCreateInfo(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(DShadow));
				uniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				VmaAllocationCreateInfo uniformAllocInfo = {};
				uniformAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				uniformAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
				vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &uniformBufferInfo, &uniformAllocInfo, &uboShadowBuff[i], &uboShadowAlloc[i], nullptr);
				//
				VkDescriptorBufferInfo bufferInfo_shadow = {};
				bufferInfo_shadow.buffer = uboShadowBuff[i];
				bufferInfo_shadow.offset = 0;
				bufferInfo_shadow.range = sizeof(DShadow);

				std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
					vks::initializers::writeDescriptorSet(DescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferInfo_shadow),
				};
				vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

				//
				//	Render Pass Info
				renderPass[i] = vks::initializers::renderPassBeginInfo();
				renderPass[i].renderPass = WorldEngine::VulkanDriver::frameBuffers.shadow->renderPass;
				renderPass[i].framebuffer = WorldEngine::VulkanDriver::frameBuffers.shadow->framebuffers[i];
				renderPass[i].renderArea.extent.width = SHADOWMAP_DIM;
				renderPass[i].renderArea.extent.height = SHADOWMAP_DIM;
				renderPass[i].clearValueCount = 1;
				renderPass[i].pClearValues = clearValues.data();
			}
		}

		//
		//	TODO: Implement staging buffer
		void UploadBuffersToGPU(const size_t& CurFrame)
		{
			memcpy(uboShadowAlloc[CurFrame]->GetMappedData(), &uboShadow, sizeof(uboShadow));
		}

		void ResetCommandPools(std::vector <VkCommandBuffer>& primaryCommandBuffers_Shadow, std::vector<TriangleMesh*>& MeshCache)
		{
			for (int i = 0; i < primaryCommandBuffers_Shadow.size(); i++)
			{
				VkCommandBufferBeginInfo cmdBufInfo_shadow = vks::initializers::commandBufferBeginInfo();
				VK_CHECK_RESULT(vkBeginCommandBuffer(primaryCommandBuffers_Shadow[i], &cmdBufInfo_shadow));

				vkCmdSetViewport(primaryCommandBuffers_Shadow[i], 0, 1, &viewport);
				vkCmdSetScissor(primaryCommandBuffers_Shadow[i], 0, 1, &scissor);
				vkCmdSetDepthBias(primaryCommandBuffers_Shadow[i], WorldEngine::VulkanDriver::depthBiasConstant, 0.0f, WorldEngine::VulkanDriver::depthBiasSlope);
				vkCmdBeginRenderPass(primaryCommandBuffers_Shadow[i], &renderPass[i], VK_SUBPASS_CONTENTS_INLINE);
				vkCmdBindPipeline(primaryCommandBuffers_Shadow[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);


				// hacky instancing
				vkCmdBindDescriptorSets(primaryCommandBuffers_Shadow[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &DescriptorSets[i], 0, nullptr);

				size_t indexPos = 0;
				for (auto& Mesh : MeshCache)
				{
					//
					//	Only draw shadow casing meshes
					if (!Mesh->bCastsShadows) {
						continue;
					}
					//
					VkDeviceSize offsets[] = { 0 };
					vkCmdBindVertexBuffers(primaryCommandBuffers_Shadow[i], 0, 1, &Mesh->vertexBuffer, offsets);
					vkCmdBindIndexBuffer(primaryCommandBuffers_Shadow[i], Mesh->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
					uint32_t indexSize = indexSize = Mesh->_GLTF->Indices.size();
					size_t instanceCount = 0;
					for (auto& Instance : Mesh->instanceData)
					{
						Mesh->instanceData_Shadow[instanceCount] = &uboShadow.instancePos[indexPos + instanceCount];// = &Instance.model;
						instanceCount++;
					}
					vkCmdDrawIndexed(primaryCommandBuffers_Shadow[i], indexSize, instanceCount, 0, 0, indexPos);
					indexPos += instanceCount;
				}
				// end hacky instancing

				//
				//	End shadow pass
				vkCmdEndRenderPass(primaryCommandBuffers_Shadow[i]);
				VK_CHECK_RESULT(vkEndCommandBuffer(primaryCommandBuffers_Shadow[i]));
			}
		}
	};
}