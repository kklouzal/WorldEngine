#pragma once

namespace Pipeline {
	struct Animated : public PipelineObject
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
			TriangleMesh* Mesh = new TriangleMesh(this, FileName, GLTFInfo_, GLTFInfo_->DiffuseTex, GLTFInfo_->NormalTex, bCastsShadows, true);
			Mesh->Name = "Animated";
			MeshCache.push_back(Mesh);
			return Mesh;
		}

		~Animated()
		{
			for (auto& Mesh : MeshCache) {
				delete Mesh;
			}
		}

		Animated(VkPipelineCache PipelineCache)
			: PipelineObject()
		{
			//
			//	DescriptorSetLayout
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				//	Binding 0 : Camera UBO
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
				//	Binding 1 : Joint Matrices
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 1),
				//	Binding 2 : Joint Matrices
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 2),
				//	Binding 3 : Color texture target
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
				//	Binding 4 : Normal texture target
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4)
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
			vertShaderStageInfo.module = createShaderModule(readFile("shaders/Vertex_Animated.vert.spv"));
			vertShaderStageInfo.pName = "main";
			shaderStages[0] = vertShaderStageInfo;
			VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = createShaderModule(readFile("shaders/Fragment_Animated.frag.spv"));
			fragShaderStageInfo.pName = "main";
			shaderStages[1] = fragShaderStageInfo;
			//	Bind vertex input
			auto binding = Vertex::getBindingDescription();
			auto description = Vertex::getAttributeDescriptions_Animated();
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
		//	TODO: Pass in StorageBuffers payload instead of vector so we can send in multiple buffer 'pools'.
		DescriptorObject*const createDescriptor(const TriangleMesh*const Mesh) {
			DescriptorObject* NewDescriptor = new DescriptorObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice);
			//
			//	Create Descriptor Pool
			std::vector<VkDescriptorPoolSize> poolSizes = {
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 16),
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 16),
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 16)
			};
			VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size());
			VK_CHECK_RESULT(vkCreateDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &descriptorPoolInfo, nullptr, &NewDescriptor->DescriptorPool));
			//
			//	Create and Update individual Descriptor sets
			size_t SwapChainSize = WorldEngine::VulkanDriver::swapChain.images.size();
			NewDescriptor->DescriptorSets.resize(SwapChainSize);
			for (size_t i = 0; i < SwapChainSize; i++) {
				VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(NewDescriptor->DescriptorPool, &descriptorSetLayout, 1);
				VK_CHECK_RESULT(vkAllocateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &allocInfo, &NewDescriptor->DescriptorSets[i]));

				//
				VkDescriptorBufferInfo bufferInfo_camera = {};
				bufferInfo_camera.buffer = WorldEngine::SceneGraph::GetCamera()->uboCamBuff[i];
				bufferInfo_camera.offset = 0;
				bufferInfo_camera.range = sizeof(CameraUniformBuffer);
				//
				VkDescriptorBufferInfo bufferInfo_InstanceData_Animation = {};
				bufferInfo_InstanceData_Animation.buffer = Mesh->instanceStorageSpaceBuffers[i+SwapChainSize];
				bufferInfo_InstanceData_Animation.offset = 0;
				bufferInfo_InstanceData_Animation.range = VK_WHOLE_SIZE;
				//
				VkDescriptorBufferInfo bufferInfo_IBP = {};
				bufferInfo_IBP.buffer = Mesh->IBP_Buffer;
				bufferInfo_IBP.offset = 0;
				bufferInfo_IBP.range = VK_WHOLE_SIZE;

				VkDescriptorImageInfo textureImageColor =
					vks::initializers::descriptorImageInfo(
						Sampler,
						Mesh->Texture_Albedo->ImageView,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				VkDescriptorImageInfo textureImageNormal =
					vks::initializers::descriptorImageInfo(
						Sampler,
						Mesh->Texture_Normal->ImageView,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
					// Binding 0 : Camera UBO
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferInfo_camera),
					// Binding 1 : Joint Matrices Storage Buffer
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, &bufferInfo_InstanceData_Animation),
					// Binding 2 : Joint Inverse Bind Poses Buffer
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, &bufferInfo_IBP),
					// Binding 3 : color
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &textureImageColor),
					// Binding 4 : normal
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &textureImageNormal)
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
		inline void updateDescriptor(DescriptorObject*const Descriptor, const TriangleMesh*const Mesh) final
		{
			size_t SwapChainSize = WorldEngine::VulkanDriver::swapChain.images.size();
			for (size_t i = 0; i < SwapChainSize; i++) {
				VkDescriptorBufferInfo bufferInfo_InstanceData = {};
				bufferInfo_InstanceData.buffer = Mesh->instanceStorageSpaceBuffers[i];
				bufferInfo_InstanceData.offset = 0;
				bufferInfo_InstanceData.range = VK_WHOLE_SIZE;
				//
				VkDescriptorBufferInfo bufferInfo_camera = {};
				bufferInfo_camera.buffer = WorldEngine::SceneGraph::GetCamera()->uboCamBuff[i];
				bufferInfo_camera.offset = 0;
				bufferInfo_camera.range = sizeof(CameraUniformBuffer);
				//
				VkDescriptorBufferInfo bufferInfo_InstanceData_Animation = {};
				bufferInfo_InstanceData_Animation.buffer = Mesh->instanceStorageSpaceBuffers[i+SwapChainSize];
				bufferInfo_InstanceData_Animation.offset = 0;
				bufferInfo_InstanceData_Animation.range = VK_WHOLE_SIZE;
				//
				VkDescriptorBufferInfo bufferInfo_IBP = {};
				bufferInfo_IBP.buffer = Mesh->IBP_Buffer;
				bufferInfo_IBP.offset = 0;
				bufferInfo_IBP.range = VK_WHOLE_SIZE;

				VkDescriptorImageInfo textureImageColor =
					vks::initializers::descriptorImageInfo(
						Sampler,
						Mesh->Texture_Albedo->ImageView,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				VkDescriptorImageInfo textureImageNormal =
					vks::initializers::descriptorImageInfo(
						Sampler,
						Mesh->Texture_Normal->ImageView,
						VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
					// Binding 0 : Instancing SSBO
					vks::initializers::writeDescriptorSet(Descriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0, &bufferInfo_InstanceData),
					// Binding 1 : Camera UBO
					vks::initializers::writeDescriptorSet(Descriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &bufferInfo_camera),
					// Binding 2 : Joint Matrices Storage Buffer
					vks::initializers::writeDescriptorSet(Descriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2, &bufferInfo_InstanceData_Animation),
					// Binding 3 : Joint Inverse Bind Poses Buffer
					vks::initializers::writeDescriptorSet(Descriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, &bufferInfo_IBP),
					// Binding 4 : color
					vks::initializers::writeDescriptorSet(Descriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &textureImageColor),
					// Binding 5 : normal
					vks::initializers::writeDescriptorSet(Descriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, &textureImageNormal)
				};

				vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
			}
		}
		//
		//
		//	Reset Command Pools
		//
		//
		inline void ResetCommandPools(std::vector<VkCommandBuffer>& CommandBuffers, std::vector<TriangleMesh*>& MeshCache)
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
					Mesh->draw(CommandBuffers[i], static_cast<uint32_t>(i));
				}
				//
				//	End scene node pass
				VK_CHECK_RESULT(vkEndCommandBuffer(CommandBuffers[i]));
			}
		}
	};
}