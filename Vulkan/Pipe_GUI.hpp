#pragma once

namespace Pipeline {
	struct GUI : public PipelineObject {

		VulkanDriver* _Driver;

		GUI(VulkanDriver* Driver) : PipelineObject(Driver), _Driver(Driver) {
			//
			//	DescriptorSetLayout
			//
			VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
			samplerLayoutBinding.binding = 0;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

			std::array<VkDescriptorSetLayoutBinding, 1> bindings = { samplerLayoutBinding };
			VkDescriptorSetLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
			layoutInfo.pBindings = bindings.data();

			createDescriptorSetLayout(layoutInfo);

			//
			//	Graphics Pipeline
			//
			auto vertShaderCode = readFile("Vertex_GUI.vert.spv");
			auto fragShaderCode = readFile("Fragment_GUI.frag.spv");
			VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
			VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

			VkPipelineShaderStageCreateInfo vertShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageInfo.module = vertShaderModule;
			vertShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
			fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageInfo.module = fragShaderModule;
			fragShaderStageInfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

			auto bindingDescription = Vertex::getBindingDescription();
			auto attributeDescriptions = Vertex::getAttributeDescriptions();
			VkPipelineVertexInputStateCreateInfo vertexInputInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
			vertexInputInfo.vertexBindingDescriptionCount = 1;
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

			VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			inputAssembly.primitiveRestartEnable = VK_FALSE;

			VkViewport viewport = {};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = (float)_Driver->swapChainExtent.width;
			viewport.height = (float)_Driver->swapChainExtent.height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
			viewportState.viewportCount = 1;
			viewportState.pViewports = &viewport;
			viewportState.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
			rasterizer.depthClampEnable = VK_FALSE;
			rasterizer.rasterizerDiscardEnable = VK_FALSE;
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizer.lineWidth = 1.0f;
			rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizer.depthBiasEnable = VK_FALSE;

			VkPipelineMultisampleStateCreateInfo multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
			multisampling.sampleShadingEnable = VK_FALSE;
			multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampling.minSampleShading = 1.0f; // Optional
			multisampling.pSampleMask = nullptr; // Optional
			multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
			multisampling.alphaToOneEnable = VK_FALSE; // Optional

			VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
			colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			colorBlendAttachment.blendEnable = VK_TRUE;
			colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
			colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

			VkPipelineColorBlendStateCreateInfo colorBlending = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &colorBlendAttachment;

			VkPipelineDepthStencilStateCreateInfo depthStencil = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
			depthStencil.depthTestEnable = VK_FALSE;
			depthStencil.depthWriteEnable = VK_FALSE;
			depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencil.depthBoundsTestEnable = VK_FALSE;
			depthStencil.minDepthBounds = 0.0f; // Optional
			depthStencil.maxDepthBounds = 1.0f; // Optional
			depthStencil.stencilTestEnable = VK_FALSE;
			depthStencil.front = {}; // Optional
			depthStencil.back = {}; // Optional

			std::vector<VkDynamicState> dynamicStateEnables = {
				//VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};
			VkPipelineDynamicStateCreateInfo dynamicState = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
			dynamicState.pDynamicStates = dynamicStateEnables.data();
			dynamicState.dynamicStateCount = dynamicStateEnables.size();
			dynamicState.flags = 0;

			createPipeline(shaderStages, vertexInputInfo, inputAssembly, viewportState, rasterizer, multisampling, depthStencil, colorBlending, dynamicState);

			vkDestroyShaderModule(_Driver->_VulkanDevice->logicalDevice, fragShaderModule, nullptr);
			vkDestroyShaderModule(_Driver->_VulkanDevice->logicalDevice, vertShaderModule, nullptr);

			//
			//	Image Sampler For Entire Pipeline
			VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = 16;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 0.0f;
			if (vkCreateSampler(_Driver->_VulkanDevice->logicalDevice, &samplerInfo, nullptr, &Sampler) != VK_SUCCESS) {
#ifdef _DEBUG
				throw std::runtime_error("failed to create texture sampler!");
#endif
			}
		}
		//
		//
		//	Create Descriptor
		//
		//
		DescriptorObject* createDescriptor(const TextureObject* Texture, const std::vector<VkBuffer>& UniformBuffers = std::vector<VkBuffer>()) {
			DescriptorObject* NewDescriptor = new DescriptorObject(_Driver);

			std::array<VkDescriptorPoolSize, 1> poolSizes = {};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[0].descriptorCount = static_cast<uint32_t>(_Driver->swapChain.images.size());

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
			VkDescriptorSetAllocateInfo allocInfoDS = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			allocInfoDS.descriptorPool = NewDescriptor->DescriptorPool;
			allocInfoDS.descriptorSetCount = static_cast<uint32_t>(_Driver->swapChain.images.size());
			allocInfoDS.pSetLayouts = layouts.data();

			NewDescriptor->DescriptorSets.resize(_Driver->swapChain.images.size());
			if (vkAllocateDescriptorSets(_Driver->_VulkanDevice->logicalDevice, &allocInfoDS, NewDescriptor->DescriptorSets.data()) != VK_SUCCESS) {
#ifdef _DEBUG
				throw std::runtime_error("failed to allocate descriptor sets!");
#endif
			}

			for (size_t i = 0; i < _Driver->swapChain.images.size(); i++) {

				VkDescriptorImageInfo imageInfo = {};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = Texture->ImageView;
				imageInfo.sampler = Sampler;

				std::array<VkWriteDescriptorSet, 1> descriptorWrites = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

				descriptorWrites[0].dstSet = NewDescriptor->DescriptorSets[i];
				descriptorWrites[0].dstBinding = 0;
				descriptorWrites[0].dstArrayElement = 0;
				descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrites[0].descriptorCount = 1;
				descriptorWrites[0].pImageInfo = &imageInfo;

				vkUpdateDescriptorSets(_Driver->_VulkanDevice->logicalDevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
			}

			return NewDescriptor;
		}

		DescriptorObject* createRawDescriptor(const VkImageView Image, const VkSampler Sampler) {
			DescriptorObject* NewDescriptor = new DescriptorObject(_Driver);

			std::array<VkDescriptorPoolSize, 1> poolSizes = {};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[0].descriptorCount = static_cast<uint32_t>(_Driver->swapChain.images.size());

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
			VkDescriptorSetAllocateInfo allocInfoDS = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
			allocInfoDS.descriptorPool = NewDescriptor->DescriptorPool;
			allocInfoDS.descriptorSetCount = static_cast<uint32_t>(_Driver->swapChain.images.size());
			allocInfoDS.pSetLayouts = layouts.data();

			NewDescriptor->DescriptorSets.resize(_Driver->swapChain.images.size());
			if (vkAllocateDescriptorSets(_Driver->_VulkanDevice->logicalDevice, &allocInfoDS, NewDescriptor->DescriptorSets.data()) != VK_SUCCESS) {
#ifdef _DEBUG
				throw std::runtime_error("failed to allocate descriptor sets!");
#endif
			}

			for (size_t i = 0; i < _Driver->swapChain.images.size(); i++) {

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

			if (_Textures.count(File) == 1) {
				return _Textures[File];
			}
			else {
				auto Tex = _Textures.emplace(File, new TextureObject(_Driver)).first->second;

				const unsigned int error = lodepng::decode(Tex->Pixels, Tex->Width, Tex->Height, File);

				if (error) {
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

				vmaCreateImage(_Driver->allocator, &imageInfo, &allocInfo, &Tex->Image, &Tex->Allocation, nullptr);
				//
				//	CPU->GPU Copy
				VkCommandBuffer commandBuffer = _Driver->_SceneGraph->beginSingleTimeCommands();
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

				_Driver->_SceneGraph->endSingleTimeCommands(commandBuffer);

				vmaDestroyBuffer(_Driver->allocator, stagingImageBuffer, stagingImageBufferAlloc);

				VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
				textureImageViewInfo.image = Tex->Image;
				textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				textureImageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
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