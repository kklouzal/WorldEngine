#pragma once

#include "lodepng.h"

struct PipelineObject
{
	VkSampler Sampler = VK_NULL_HANDLE;
	VkSampler DeferredSampler = VK_NULL_HANDLE;
	VkSampler ShadowSampler = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline = VK_NULL_HANDLE;
	
	virtual ~PipelineObject() {
		printf("\tDestroy Base PipelineObject\n");
		for (auto Tex : _Textures) {
			delete Tex.second;
		}
		for (auto Tex : _Textures2) {
			delete Tex;
		}
		vkDestroyPipeline(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, descriptorSetLayout, nullptr);
		vkDestroySampler(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, Sampler, nullptr);
		vkDestroySampler(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, DeferredSampler, nullptr);
		vkDestroySampler(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, ShadowSampler, nullptr);
	}

	void EmptyCache() {
		for (auto Tex : _Textures) {
			delete Tex.second;
		}
		_Textures.clear();
		for (auto Tex : _Textures2) {
			delete Tex;
		}
		_Textures2.clear();
	}

protected:

	std::unordered_map<std::string, TextureObject*> _Textures;
	std::deque<TextureObject*> _Textures2;

	PipelineObject()
	{
		//
		//	Image Sampler For Entire Pipeline
		VkSamplerCreateInfo samplerInfo1 = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
		samplerInfo1.magFilter = VK_FILTER_LINEAR;
		samplerInfo1.minFilter = VK_FILTER_LINEAR;
		samplerInfo1.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo1.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo1.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo1.anisotropyEnable = VK_TRUE;
		samplerInfo1.maxAnisotropy = 16;
		samplerInfo1.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo1.unnormalizedCoordinates = VK_FALSE;
		samplerInfo1.compareEnable = VK_FALSE;
		samplerInfo1.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo1.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo1.mipLodBias = 0.0f;
		samplerInfo1.minLod = 0.0f;
		samplerInfo1.maxLod = 0.0f;
		if (vkCreateSampler(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &samplerInfo1, nullptr, &Sampler) != VK_SUCCESS) {
			#ifdef _DEBUG
			throw std::runtime_error("failed to create texture sampler!");
			#endif
		}

		VkSamplerCreateInfo samplerInfo2 = vks::initializers::samplerCreateInfo();
		samplerInfo2.magFilter = VK_FILTER_NEAREST;
		samplerInfo2.minFilter = VK_FILTER_NEAREST;
		samplerInfo2.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo2.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo2.addressModeV = samplerInfo2.addressModeU;
		samplerInfo2.addressModeW = samplerInfo2.addressModeU;
		samplerInfo2.mipLodBias = 0.0f;
		samplerInfo2.maxAnisotropy = 1.0f;
		samplerInfo2.minLod = 0.0f;
		samplerInfo2.maxLod = 1.0f;
		samplerInfo2.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		if (vkCreateSampler(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &samplerInfo2, nullptr, &DeferredSampler) != VK_SUCCESS) {
			#ifdef _DEBUG
			throw std::runtime_error("failed to create texture sampler!");
			#endif
		}

		VkSamplerCreateInfo samplerInfo3 = vks::initializers::samplerCreateInfo();
		samplerInfo3.magFilter = VK_FILTER_LINEAR;
		samplerInfo3.minFilter = VK_FILTER_LINEAR;
		samplerInfo3.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo3.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo3.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo3.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo3.mipLodBias = 0.0f;
		samplerInfo3.maxAnisotropy = 1.0f;
		samplerInfo3.minLod = 0.0f;
		samplerInfo3.maxLod = 1.0f;
		samplerInfo3.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		if (vkCreateSampler(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &samplerInfo2, nullptr, &ShadowSampler) != VK_SUCCESS) {
#ifdef _DEBUG
			throw std::runtime_error("failed to create texture sampler!");
#endif
		}
	}

	//
	//	Helper Functions
	std::vector<char> readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			#ifdef _DEBUG
			throw std::runtime_error("failed to open file!");
			#endif
		}

		const size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}

	VkShaderModule createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			#ifdef _DEBUG
			throw std::runtime_error("failed to create shader module!");
			#endif
		}

		return shaderModule;
	}

public:

	virtual DescriptorObject*const createDescriptor(const TriangleMesh*const Mesh)
	{
		return nullptr;
	}

	inline virtual void updateDescriptor(DescriptorObject*const Descriptor, const TriangleMesh*const Mesh)
	{
	}

	//
	//
	//	Create Texture
	//
	//
	TextureObject* createTextureImage(const std::string & File) {
		printf("[Pipe]: CreateTextureImage (%s)\n", File.c_str());
		if (_Textures.count(File) == 1) {
			printf("\tReusing Existing Texture\n");
			return _Textures[File];
		}
		else {
			auto Tex = _Textures.emplace(File, new TextureObject(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, WorldEngine::VulkanDriver::allocator)).first->second;
			printf("\tLoad New Texture\n");
			const unsigned int error = lodepng::decode(Tex->Pixels, Tex->Width, Tex->Height, File);

			if (error) {
				printf("\t\tPNG Decode Error: (%i) %s\n\t\tUsing Default (missingimage.png)\n", error, lodepng_error_text(error));
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

	TextureObject* createTextureImage2(tinygltf::Image & ImgData) {
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