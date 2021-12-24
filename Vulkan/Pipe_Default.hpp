#pragma once

namespace Pipeline {
	struct Default : public PipelineObject
	{
		VkPipeline graphicsPipeline_Composition = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> DescriptorSets_Composition = {};
		VkDescriptorSetLayout descriptorSetLayout_Composition = VK_NULL_HANDLE;
		VkDescriptorPool DescriptorPool_Composition = VK_NULL_HANDLE;

		~Default()
		{
			vkDestroyPipeline(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, graphicsPipeline_Composition, nullptr);
			vkDestroyDescriptorPool(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, DescriptorPool_Composition, nullptr);
		}

		Default(VkPipelineCache PipelineCache)
			: PipelineObject()
		{

			//
			//	DescriptorSetLayout
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				//	Binding 0 : Vertex UBO
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0),
				//	Binding 1 : Position/Color texture target
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
				//	Binding 2 : Normal texture target
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2),
				//	Binding 3 : Albedo texture target
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3),
				//	Binding 4 : Fragment UBO
				vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 4)
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
			pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
			pipelineCI.pStages = shaderStages.data();
			//
			//
			// 
			// 
			//	Final fullscreen composition pass pipeline
			rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
			//	Load shader files
			VkPipelineShaderStageCreateInfo vertShaderStageInfo1 = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertShaderStageInfo1.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo1.module = createShaderModule(readFile("shaders/deferred.vert.spv"));
			vertShaderStageInfo1.pName = "main";
			shaderStages[0] = vertShaderStageInfo1;
			VkPipelineShaderStageCreateInfo fragShaderStageInfo1 = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragShaderStageInfo1.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo1.module = createShaderModule(readFile("shaders/deferred.frag.spv"));
			fragShaderStageInfo1.pName = "main";
			shaderStages[1] = fragShaderStageInfo1;
			//	Empty vertex input state, vertices are generated by the vertex shader
			VkPipelineVertexInputStateCreateInfo emptyInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
			pipelineCI.pVertexInputState = &emptyInputState;
			//	Create composite graphics pipeline
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, PipelineCache, 1, &pipelineCI, nullptr, &graphicsPipeline_Composition));
			//	Cleanup Shader Modules
			vkDestroyShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, vertShaderStageInfo1.module, nullptr);
			vkDestroyShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, fragShaderStageInfo1.module, nullptr);
			//
			//
			// 
			// 
			//	Offscreen model render pass pipeline
			rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
			//	Load shader files
			VkPipelineShaderStageCreateInfo vertShaderStageInfo2 = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertShaderStageInfo2.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo2.module = createShaderModule(readFile("shaders/Vertex_Default.vert.spv"));
			vertShaderStageInfo2.pName = "main";
			shaderStages[0] = vertShaderStageInfo2;
			VkPipelineShaderStageCreateInfo fragShaderStageInfo2 = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragShaderStageInfo2.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo2.module = createShaderModule(readFile("shaders/Fragment_Default.frag.spv"));
			fragShaderStageInfo2.pName = "main";
			shaderStages[1] = fragShaderStageInfo2;
			//	Bind vertex input
			auto binding = Vertex::getBindingDescription();
			auto description = Vertex::getAttributeDescriptions();
			VkPipelineVertexInputStateCreateInfo vertexInputInfo = vks::initializers::pipelineVertexInputStateCreateInfo(binding, description);
			pipelineCI.pVertexInputState = &vertexInputInfo;
			//	Separate render pass
			pipelineCI.renderPass = WorldEngine::VulkanDriver::frameBuffers.deferred->renderPass;
			//	Blend attachment states required for all color attachments
			std::array<VkPipelineColorBlendAttachmentState, 3> blendAttachmentStates = {
				vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
				vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE),
				vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE)
			};
			colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
			colorBlendState.pAttachments = blendAttachmentStates.data();
			//	Create offscreen graphics pipeline
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, PipelineCache, 1, &pipelineCI, nullptr, &graphicsPipeline));
			//	Cleanup Shader Modules
			vkDestroyShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, vertShaderStageInfo2.module, nullptr);
			vkDestroyShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, fragShaderStageInfo2.module, nullptr);



			//
			//
			//	Deferred Descriptor
			//
			//	Create Descriptor Pool
			std::vector<VkDescriptorPoolSize> poolSizes = {
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 3),
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 3)
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

				std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
					// Binding 1 : Position texture target
					vks::initializers::writeDescriptorSet(DescriptorSets_Composition[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &texDescriptorPosition),
					// Binding 2 : Normals texture target
					vks::initializers::writeDescriptorSet(DescriptorSets_Composition[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &texDescriptorNormal),
					// Binding 3 : Albedo texture target
					vks::initializers::writeDescriptorSet(DescriptorSets_Composition[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, &texDescriptorAlbedo),
					// Binding 4 : Fragment shader uniform buffer
					vks::initializers::writeDescriptorSet(DescriptorSets_Composition[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &bufferInfo_composition)
				};
				vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
			}
		}
		//
		//
		//	Create Descriptor
		//
		//
		DescriptorObject* createDescriptor(const TextureObject* TextureColor, const TextureObject* TextureNormal, const std::vector<VkBuffer>& UniformBuffers) {
			DescriptorObject* NewDescriptor = new DescriptorObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice);
			//
			//	Create Descriptor Pool
			std::vector<VkDescriptorPoolSize> poolSizes = {
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 3),
				vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (uint32_t)WorldEngine::VulkanDriver::swapChain.images.size() * 3)
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
				bufferInfo.buffer = UniformBuffers[i];
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformBufferObject);

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
					// Binding 0 : vertex data
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferInfo),
					// Binding 1 : color
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &textureImageColor),
					// Binding 2 : normal
					vks::initializers::writeDescriptorSet(NewDescriptor->DescriptorSets[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &textureImageNormal)
				};

				vkUpdateDescriptorSets(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
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
				auto Tex = _Textures.emplace(File, new TextureObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, WorldEngine::VulkanDriver::allocator)).first->second;
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
				Tex->Pixels.clear();
				Tex->Pixels.shrink_to_fit();

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
				textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				textureImageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
				textureImageViewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
				textureImageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
				textureImageViewInfo.image = Tex->Image;
				vkCreateImageView(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &textureImageViewInfo, nullptr, &Tex->ImageView);

				return Tex;
			}
		}

		TextureObject* createTextureImage2(tinygltf::Image& ImgData) {
			//printf("[Pipe][Default]: CreateTextureImage (%s)\n", File.c_str());

			auto Tex = new TextureObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, WorldEngine::VulkanDriver::allocator);
			Tex->Width = ImgData.width;
			Tex->Height = ImgData.height;
			Tex->Pixels = ImgData.image;

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
			textureImageViewInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
			textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			textureImageViewInfo.subresourceRange.baseMipLevel = 0;
			textureImageViewInfo.subresourceRange.levelCount = 1;
			textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
			textureImageViewInfo.subresourceRange.layerCount = 1;
			vkCreateImageView(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &textureImageViewInfo, nullptr, &Tex->ImageView);

			_Textures2.push_back(Tex);
			return Tex;
		}
	};
}