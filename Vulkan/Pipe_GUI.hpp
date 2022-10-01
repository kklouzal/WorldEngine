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
			pipelineCI.subpass = 2;	//	Subpass
			//
			rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;

			auto binding = Vertex::getBindingDescription();
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
		//
		//
		//	Create Texture
		//
		//
		TextureObject* createTextureImage(const std::string& File) {

			if (_Textures.count(File) == 1) {
				return _Textures[File];
			}
			else {
				auto Tex = _Textures.emplace(File, new TextureObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, WorldEngine::VulkanDriver::allocator)).first->second;

				const unsigned int error = lodepng::decode(Tex->Pixels, Tex->Width, Tex->Height, File);

				if (error) {
					printf("MISSING TEXTURE\n");
					const unsigned int error2 = lodepng::decode(Tex->Pixels, Tex->Width, Tex->Height, "media/missingimage.png");
					if (error2) {
						printf("\tFAILED!!\n");
						_Textures.erase(File);
						delete Tex;
						return nullptr;
					}
				}

				const VkDeviceSize imageSize = Tex->Width * Tex->Height * 4;

				//
				//	Image Staging Buffer
				VkBufferCreateInfo stagingBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				stagingBufferInfo.size = imageSize;
				stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VmaAllocationCreateInfo allocInfo = {};
				allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
				allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

				VkBuffer stagingImageBuffer = VK_NULL_HANDLE;
				VmaAllocation stagingImageBufferAlloc = VK_NULL_HANDLE;
				vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &stagingBufferInfo, &allocInfo, &stagingImageBuffer, &stagingImageBufferAlloc, nullptr);

				memcpy(stagingImageBufferAlloc->GetMappedData(), Tex->Pixels.data(), static_cast<size_t>(imageSize));

				VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
				imageInfo.imageType = VK_IMAGE_TYPE_2D;
				imageInfo.extent.width = static_cast<uint32_t>(Tex->Width);
				imageInfo.extent.height = static_cast<uint32_t>(Tex->Height);
				imageInfo.extent.depth = 1;
				imageInfo.mipLevels = 1;
				imageInfo.arrayLayers = 1;
				imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
				imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

				allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

				vmaCreateImage(WorldEngine::VulkanDriver::allocator, &imageInfo, &allocInfo, &Tex->Image, &Tex->Allocation, nullptr);
				//
				//	CPU->GPU Copy
				VkCommandBuffer commandBuffer = WorldEngine::VulkanDriver::beginSingleTimeCommands();
				VkImageMemoryBarrier imgMemBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
				imgMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imgMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imgMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imgMemBarrier.subresourceRange.baseMipLevel = 0;
				imgMemBarrier.subresourceRange.levelCount = 1;
				imgMemBarrier.subresourceRange.baseArrayLayer = 0;
				imgMemBarrier.subresourceRange.layerCount = 1;
				imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imgMemBarrier.image = Tex->Image;
				imgMemBarrier.srcAccessMask = 0;
				imgMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

				vkCmdPipelineBarrier(
					commandBuffer,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &imgMemBarrier);

				VkBufferImageCopy region = {};
				region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				region.imageSubresource.layerCount = 1;
				region.imageExtent.width = static_cast<uint32_t>(Tex->Width);
				region.imageExtent.height = static_cast<uint32_t>(Tex->Height);
				region.imageExtent.depth = 1;

				vkCmdCopyBufferToImage(commandBuffer, stagingImageBuffer, Tex->Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

				imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imgMemBarrier.image = Tex->Image;
				imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(
					commandBuffer,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &imgMemBarrier);

				WorldEngine::VulkanDriver::endSingleTimeCommands(commandBuffer);

				vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, stagingImageBuffer, stagingImageBufferAlloc);

				VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
				textureImageViewInfo.image = Tex->Image;
				textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				textureImageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
				textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				textureImageViewInfo.subresourceRange.baseMipLevel = 0;
				textureImageViewInfo.subresourceRange.levelCount = 1;
				textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
				textureImageViewInfo.subresourceRange.layerCount = 1;
				vkCreateImageView(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &textureImageViewInfo, nullptr, &Tex->ImageView);

				return Tex;
			}
		}
	};
}