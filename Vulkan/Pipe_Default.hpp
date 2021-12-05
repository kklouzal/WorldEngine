#pragma once

namespace Pipeline {
	struct Default : public PipelineObject {

		VulkanDriver* _Driver;

		Default(VulkanDriver* Driver, VkPipelineCache PipelineCache)
			: PipelineObject(Driver), _Driver(Driver)
		{
			//
			//	DescriptorSetLayout
			//
			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

			VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
			samplerLayoutBinding.binding = 1;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			VkDescriptorSetLayoutBinding uboLayoutBinding_Lighting = {};
			uboLayoutBinding_Lighting.binding = 2;
			uboLayoutBinding_Lighting.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding_Lighting.descriptorCount = 1;
			uboLayoutBinding_Lighting.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBinding, samplerLayoutBinding, uboLayoutBinding_Lighting };
			VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();

			createDescriptorSetLayout(layoutInfo);

			//
			//	Graphics Pipeline
			//
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

			VkPipelineShaderStageCreateInfo vertShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = createShaderModule(readFile("Vertex_Default.vert.spv"));
			vertShaderStageInfo.pName = "main";
			shaderStages[0] = vertShaderStageInfo;
			VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = createShaderModule(readFile("Fragment_Default.frag.spv"));
			fragShaderStageInfo.pName = "main";
			shaderStages[1] = fragShaderStageInfo;

			VkGraphicsPipelineCreateInfo pipelineCI = vks::initializers::pipelineCreateInfo(pipelineLayout, _Driver->renderPass);
			pipelineCI.pInputAssemblyState = &inputAssemblyState;
			pipelineCI.pRasterizationState = &rasterizationState;
			pipelineCI.pColorBlendState = &colorBlendState;
			pipelineCI.pMultisampleState = &multisampleState;
			pipelineCI.pViewportState = &viewportState;
			pipelineCI.pDepthStencilState = &depthStencilState;
			pipelineCI.pDynamicState = &dynamicState;
			pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
			pipelineCI.pStages = shaderStages.data();
			

			auto bindingDescription = Vertex::getBindingDescription();
			auto attributeDescriptions = Vertex::getAttributeDescriptions();
			VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
			pipelineCI.pVertexInputState = &vertexInputInfo;
			rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;

			vkCreateGraphicsPipelines(_Driver->_VulkanDevice->logicalDevice, PipelineCache, 1, &pipelineCI, nullptr, &graphicsPipeline);
			//
			//	Cleanup Shader Modules
			vkDestroyShaderModule(_Driver->_VulkanDevice->logicalDevice, vertShaderStageInfo.module, nullptr);
			vkDestroyShaderModule(_Driver->_VulkanDevice->logicalDevice, fragShaderStageInfo.module, nullptr);
		}
		//
		//
		//	Create Descriptor
		//
		//
		DescriptorObject* createDescriptor(const TextureObject* Texture, const std::vector<VkBuffer>& UniformBuffers) {
			DescriptorObject* NewDescriptor = new DescriptorObject(_Driver);

			std::array<VkDescriptorPoolSize, 3> poolSizes = {};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount = static_cast<uint32_t>(_Driver->swapChain.images.size());
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = static_cast<uint32_t>(_Driver->swapChain.images.size());
			poolSizes[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[2].descriptorCount = static_cast<uint32_t>(_Driver->swapChain.images.size());

			VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
			poolInfo.pPoolSizes = poolSizes.data();
			poolInfo.maxSets = static_cast<uint32_t>(_Driver->swapChain.images.size());

			if (vkCreateDescriptorPool(_Driver->_VulkanDevice->logicalDevice, &poolInfo, nullptr, &NewDescriptor->DescriptorPool) != VK_SUCCESS) {
#ifdef _DEBUG
				throw std::runtime_error("failed to create descriptor pool!");
#endif
			}

			std::vector<VkDescriptorSetLayout> layouts(_Driver->swapChain.images.size(), descriptorSetLayout);
			VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			allocInfo.descriptorPool = NewDescriptor->DescriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(_Driver->swapChain.images.size());
			allocInfo.pSetLayouts = layouts.data();

			NewDescriptor->DescriptorSets.resize(_Driver->swapChain.images.size());
			if (vkAllocateDescriptorSets(_Driver->_VulkanDevice->logicalDevice, &allocInfo, NewDescriptor->DescriptorSets.data()) != VK_SUCCESS) {
#ifdef _DEBUG
				throw std::runtime_error("failed to allocate descriptor sets!");
#endif
			}

			for (size_t i = 0; i < _Driver->swapChain.images.size(); i++) {
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = UniformBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformBufferObject);

				VkDescriptorImageInfo imageInfo = {};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = Texture->ImageView;
				imageInfo.sampler = Sampler;

				VkDescriptorBufferInfo bufferInfo_Lighting = {};
				bufferInfo_Lighting.buffer = _Driver->_SceneGraph->UniformBuffers_Lighting[i];
				bufferInfo_Lighting.offset = 0;
				bufferInfo_Lighting.range = sizeof(UniformBufferObject_PointLights);

				std::array<VkWriteDescriptorSet, 3> descriptorWrites = {};

				descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[0].dstSet = NewDescriptor->DescriptorSets[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pBufferInfo = &bufferInfo;

				descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[1].dstSet = NewDescriptor->DescriptorSets[i];
				descriptorWrites[1].dstBinding = 1;
				descriptorWrites[1].dstArrayElement = 0;
				descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[1].descriptorCount = 1;
				descriptorWrites[1].pImageInfo = &imageInfo;

				descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrites[2].dstSet = NewDescriptor->DescriptorSets[i];
				descriptorWrites[2].dstBinding = 2;
				descriptorWrites[2].dstArrayElement = 0;
				descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrites[2].descriptorCount = 1;
				descriptorWrites[2].pBufferInfo = &bufferInfo_Lighting;

				vkUpdateDescriptorSets(_Driver->_VulkanDevice->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}

			return NewDescriptor;
		}
		//
		//
		//	Create Texture
		//
		//
		TextureObject* createTextureImage(const std::string& File) {
			//printf("[Pipe][Default]: CreateTextureImage (%s)\n", File.c_str());
			if (_Textures.count(File) == 1) {
				//printf("\tReusing Existing Texture\n");
				return _Textures[File];
			}
			else {
				auto Tex = _Textures.emplace(File, new TextureObject(_Driver)).first->second;
				//printf("\tLoad New Texture: %s\n", File.c_str());
				const unsigned int error = lodepng::decode(Tex->Pixels, Tex->Width, Tex->Height, File);

				if (error) {
					//printf("\t\tPNG Decode Error: (%i) %s\n\t\tUsing Default (missingimage.png)\n", error, lodepng_error_text(error));
					const unsigned int error2 = lodepng::decode(Tex->Pixels, Tex->Width, Tex->Height, "media/missingimage.png");
					if (error2) {
						_Textures.erase(File);
						delete Tex;
						return nullptr;
					}
				}

				Tex->Empty = false;

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
				vmaCreateBuffer(_Driver->allocator, &stagingBufferInfo, &allocInfo, &stagingImageBuffer, &stagingImageBufferAlloc, nullptr);

				memcpy(stagingImageBufferAlloc->GetMappedData(), Tex->Pixels.data(), static_cast<size_t>(imageSize));
				Tex->Pixels.clear();
				Tex->Pixels.shrink_to_fit();

				VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
				imageInfo.imageType = VK_IMAGE_TYPE_2D;
				imageInfo.extent.width = static_cast<uint32_t>(Tex->Width);
				imageInfo.extent.height = static_cast<uint32_t>(Tex->Height);
				imageInfo.extent.depth = 1;
				imageInfo.mipLevels = 1;
				imageInfo.arrayLayers = 1;
				imageInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
				imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
				imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

				allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

				vmaCreateImage(_Driver->allocator, &imageInfo, &allocInfo, &Tex->Image, &Tex->Allocation, nullptr);
				//
				//	CPU->GPU Copy
				VkCommandBuffer commandBuffer = _Driver->beginSingleTimeCommands();
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

				_Driver->endSingleTimeCommands(commandBuffer);

				vmaDestroyBuffer(_Driver->allocator, stagingImageBuffer, stagingImageBufferAlloc);

				VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
				textureImageViewInfo.image = Tex->Image;
				textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				textureImageViewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
				textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				textureImageViewInfo.subresourceRange.baseMipLevel = 0;
				textureImageViewInfo.subresourceRange.levelCount = 1;
				textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
				textureImageViewInfo.subresourceRange.layerCount = 1;
				vkCreateImageView(_Driver->_VulkanDevice->logicalDevice, &textureImageViewInfo, nullptr, &Tex->ImageView);

				return Tex;
			}
		}
	};
}