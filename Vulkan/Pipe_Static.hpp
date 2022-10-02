#pragma once

namespace Pipeline {
	struct Static : public PipelineObject
	{
		std::vector<TriangleMesh*> MeshCache;

		TriangleMesh* createMesh(const char* FileName, GLTFInfo* GLTFInfo_, bool bCastsShadows)
		{
			//
			//	Return mesh if already exists
			for (auto& Mesh : MeshCache) {
				if (Mesh->FileName == FileName) {
					return Mesh;
				}
			}
			//
			//	Create mesh if not exists
			TriangleMesh* Mesh = new TriangleMesh(this, FileName, GLTFInfo_, GLTFInfo_->DiffuseTex, GLTFInfo_->NormalTex, bCastsShadows, false);
			MeshCache.push_back(Mesh);
			return Mesh;
		}

		~Static()
		{
			for (auto& Mesh : MeshCache) {
				delete Mesh;
			}
		}

		Static(VkPipelineCache PipelineCache)
			: PipelineObject()
		{
			//
			//	DescriptorSetLayout
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				//	Binding 0 : Instancing SSBO
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
				//	Binding 1 : Camera UBO
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1),
				//	Binding 2 : Color texture target
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
				//	Binding 3 : Normal texture target
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3)
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
			pipelineCI.subpass = 1;	//	Subpass
			//
			rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
			//
			//	Load shader files
			VkPipelineShaderStageCreateInfo vertShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = createShaderModule(readFile("shaders/Vertex_Static.vert.spv"));
			vertShaderStageInfo.pName = "main";
			shaderStages[0] = vertShaderStageInfo;
			VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = createShaderModule(readFile("shaders/Fragment_Static.frag.spv"));
			fragShaderStageInfo.pName = "main";
			shaderStages[1] = fragShaderStageInfo;
			//	Bind vertex input
			auto binding = Vertex::getBindingDescription();
			auto description = Vertex::getAttributeDescriptions_Static();
			VkPipelineVertexInputStateCreateInfo vertexInputInfo = vks::initializers::pipelineVertexInputStateCreateInfo(binding, description);
			pipelineCI.pVertexInputState = &vertexInputInfo;
			//	Blend attachment states required for all color attachments
			std::array<VkPipelineColorBlendAttachmentState, 4> blendAttachmentStates = {
				vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
				vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
				vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
				vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
			};
			colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
			colorBlendState.pAttachments = blendAttachmentStates.data();
			//	Create offscreen graphics pipeline
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, PipelineCache, 1, &pipelineCI, nullptr, &graphicsPipeline));
			//	Cleanup Shader Modules
			vkDestroyShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, vertShaderStageInfo.module, nullptr);
			vkDestroyShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, fragShaderStageInfo.module, nullptr);
		}
		//
		//
		//	Create Descriptor
		//
		//
		DescriptorObject* createDescriptor(const TextureObject* TextureColor, const TextureObject* TextureNormal, const std::vector<VkBuffer>& StorageBuffers) {
			DescriptorObject* NewDescriptor = new DescriptorObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice);
			//
			//	Create Descriptor Pool
			std::vector<VkDescriptorPoolSize> poolSizes = {
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 16),
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 16)
			};
			VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size());
			VK_CHECK_RESULT(vkCreateDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &descriptorPoolInfo, nullptr, &NewDescriptor->DescriptorPool));
			//
			//	Create and Update individual Descriptor sets
			NewDescriptor->DescriptorSets.resize(WorldEngine::VulkanDriver::swapChain.images.size());
			for (size_t i = 0; i < WorldEngine::VulkanDriver::swapChain.images.size(); i++) {
				VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(NewDescriptor->DescriptorPool, &descriptorSetLayout, 1);
				VK_CHECK_RESULT(vkAllocateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &allocInfo, &NewDescriptor->DescriptorSets[i]));

				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = StorageBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = VK_WHOLE_SIZE;
				//
				VkDescriptorBufferInfo bufferInfo_camera = {};
				bufferInfo_camera.buffer = WorldEngine::SceneGraph::GetCamera()->uboCamBuff[i];
				bufferInfo_camera.offset = 0;
				bufferInfo_camera.range = sizeof(CameraUniformBuffer);

				VkDescriptorImageInfo textureImageColor =
					vks::initializers::descriptorImageInfo(
						Sampler,
						TextureColor->ImageView,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				VkDescriptorImageInfo textureImageNormal =
					vks::initializers::descriptorImageInfo(
						Sampler,
						TextureNormal->ImageView,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
					// Binding 0 : Instancing SSBO
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, &bufferInfo),
					// Binding 1 : Camera UBO
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &bufferInfo_camera),
					// Binding 2 : Color Texture
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &textureImageColor),
					// Binding 3 : Normal Texture
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &textureImageNormal)
				};

				vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
			}

			return NewDescriptor;
		}
		//
		//
		//	Update Descriptor
		//
		//
		void updateDescriptor(DescriptorObject* Descriptor, const TextureObject* TextureColor, const TextureObject* TextureNormal, const std::vector<VkBuffer>& StorageBuffers)
		{
			for (size_t i = 0; i < WorldEngine::VulkanDriver::swapChain.images.size(); i++) {
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = StorageBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = VK_WHOLE_SIZE;
				//
				VkDescriptorBufferInfo bufferInfo_camera = {};
				bufferInfo_camera.buffer = WorldEngine::SceneGraph::GetCamera()->uboCamBuff[i];
				bufferInfo_camera.offset = 0;
				bufferInfo_camera.range = sizeof(CameraUniformBuffer);

				VkDescriptorImageInfo textureImageColor =
					vks::initializers::descriptorImageInfo(
						Sampler,
						TextureColor->ImageView,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				VkDescriptorImageInfo textureImageNormal =
					vks::initializers::descriptorImageInfo(
						Sampler,
						TextureNormal->ImageView,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
					// Binding 0 : Instancing SSBO
					vks::initializers::writeDescriptorSet(Descriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, &bufferInfo),
					// Binding 1 : Camera UBO
					vks::initializers::writeDescriptorSet(Descriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &bufferInfo_camera),
					// Binding 2 : Color Texture
					vks::initializers::writeDescriptorSet(Descriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &textureImageColor),
					// Binding 3 : Normal Texture
					vks::initializers::writeDescriptorSet(Descriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &textureImageNormal)
				};

				vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
			}
		}
		//
		//
		//	Reset Command Pools
		//
		//
		void ResetCommandPools(std::vector<VkCommandBuffer>& CommandBuffers, std::vector<TriangleMesh*>& MeshCache)
		{
			for (size_t i = 0; i < CommandBuffers.size(); i++)
			{
				//
				//	Secondary CommandBuffer Inheritance Info
				VkCommandBufferInheritanceInfo inheritanceInfo = vks::initializers::commandBufferInheritanceInfo();
				inheritanceInfo.renderPass = WorldEngine::VulkanDriver::renderPass;
				inheritanceInfo.framebuffer = WorldEngine::VulkanDriver::frameBuffers_Main[i];
				inheritanceInfo.subpass = 1;
				//
				//	Secondary CommandBuffer Begin Info
				VkCommandBufferBeginInfo commandBufferBeginInfo = vks::initializers::commandBufferBeginInfo();
				commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
				commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				VK_CHECK_RESULT(vkBeginCommandBuffer(CommandBuffers[i], &commandBufferBeginInfo));

				//
				//	Begin recording commandbuffer				
				vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
				vkCmdSetViewport(CommandBuffers[i], 0, 1, &WorldEngine::VulkanDriver::viewport_Deferred);
				vkCmdSetScissor(CommandBuffers[i], 0, 1, &WorldEngine::VulkanDriver::scissor_Deferred);
				//
				//	Draw all SceneNodes
				for (auto& Mesh : MeshCache)
				{
					Mesh->draw(CommandBuffers[i], i);
				}
				//
				//	End scene node pass
				VK_CHECK_RESULT(vkEndCommandBuffer(CommandBuffers[i]));
			}
		}
	};
}