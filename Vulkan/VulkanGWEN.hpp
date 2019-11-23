#pragma once

#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H

#include <unordered_map>
#include <array>
#include <map>

typedef struct GWENFaceRec_ {
	const char*	file_path;
	int			face_index;
} GWENFaceRec, * GWENFace;

static FT_Error GWENFaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face* aface) {
	GWENFace face = (GWENFace)face_id;
	return FT_New_Face(library, face->file_path, face->face_index, aface);
}

namespace Gwen
{
	namespace Renderer
	{
		class Vulkan : public Gwen::Renderer::Base
		{
			VulkanDriver* _Driver;

			VkDescriptorPool FontTexture_DescriptorPool = VK_NULL_HANDLE;
			std::vector<VkDescriptorSet> FontTexture_DescriptorSets = {};

			size_t currentBuffer = 0;
			std::vector<VkCommandBuffer> commandBuffers = {};

			std::unordered_map<Vertex, uint32_t> writeUnique_Vertices = {};
			std::unordered_map<Vertex, uint32_t> drawUnique_Vertices = {};

			std::vector<Vertex> writeVertices = {};
			std::vector<Vertex> drawVertices = {};
			uint32_t vertexBufferSize = sizeof(Vertex) * 131072;

			std::vector<uint32_t> writeIndices = {};
			std::vector<uint32_t> drawIndices = {};
			uint32_t indexBufferSize = sizeof(uint32_t) * 131072;

			std::vector<VkDrawIndexedIndirectCommand> indirectCommands;
			VkBuffer indirectBuffer = VK_NULL_HANDLE;
			VmaAllocation indirectAllocation = VMA_NULL;
			uint32_t CurIndirectDraw = 0;
			uint32_t CurDrawIndex = 0;
			uint32_t TotalDrawIndex = 0;
			uint32_t TotalDrawVerts = 0;

			VkBuffer GUI_VertexBuffer = VK_NULL_HANDLE;
			VmaAllocation GUI_VertexAllocation = VMA_NULL;
			VkBuffer GUI_IndexBuffer = VK_NULL_HANDLE;
			VmaAllocation GUI_IndexAllocation = VMA_NULL;

			VkBuffer Font_VertexBuffer = VK_NULL_HANDLE;
			VmaAllocation Font_VertexAllocation = VMA_NULL;

			VmaAllocation Font_TextureAllocation = VMA_NULL;
			VkImage Font_TextureImage;
			VkImageView Font_TextureImageView;
			VkSampler Font_TextureSampler;

			FT_Library library;

			FTC_Manager manager;
			FTC_SBitCache sbitCache;
			FTC_CMapCache cmapCache;

			Gwen::Rect clipOld;
			Gwen::Rect clipNew;

			const std::vector<Vertex> Font_TextureVertices = {
			{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
			{{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
			{{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
			{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
			};

			const unsigned int HEIGHT = 600;
			const unsigned int WIDTH = 800;
			std::vector<uint8_t> writeBuffer = {};
			std::vector<uint8_t> drawBuffer = {};
			const unsigned int dst_Size = (WIDTH * HEIGHT) * 4;
			const unsigned int dst_Pitch = WIDTH * 4;
			GWENFace Face1;
			FTC_Scaler Face1Scale;
			FTC_ImageType Face1Rec;

			FTC_SBit Bit;	//	Glyph returned from character lookup in cache

			void initFreeType() {
				Face1 = new GWENFaceRec;
				Face1->file_path = "./OpenSans.ttf";
				Face1->face_index = 0;

				Face1Rec = new FTC_ImageTypeRec;
				Face1Rec->face_id = Face1;
				Face1Rec->flags = FT_LOAD_RENDER;
				Face1Rec->height = 12;
				Face1Rec->width = 12;

				FT_Error error = FT_Init_FreeType(&library);
				if (error)
				{
					printf("FreeType: Initialization Error\n");
				}
				error = FTC_Manager_New(
					library,
					0,  /* use default */
					0,  /* use default */
					0,  /* use default */
					&GWENFaceRequester,	/* use our requester */
					NULL,				/* don't need this.  */
					&manager);
				error = FTC_SBitCache_New(manager, &sbitCache);
				error = FTC_CMapCache_New(manager, &cmapCache);
			}

			virtual Gwen::Point MeasureText(Gwen::Font* pFont, const Gwen::UnicodeString& text) {
				unsigned int dst_Cursor_X = 0;

				for (const wchar_t c : text) {
					const FT_UInt gindex = FTC_CMapCache_Lookup(cmapCache, Face1, -1, c);
					FTC_SBitCache_Lookup(sbitCache, Face1Rec, gindex, &Bit, NULL);

					dst_Cursor_X += Bit->xadvance;
				}
				return Gwen::Point(dst_Cursor_X, Face1Rec->height);
			}

			void DrawText(const Gwen::UnicodeString& text, const unsigned int PenX, const unsigned int PenY) {
				unsigned int dst_Cursor_X = 0;

				for (const wchar_t c : text) {
					const FT_UInt gindex = FTC_CMapCache_Lookup(cmapCache, Face1, -1, c);
					FTC_SBitCache_Lookup(sbitCache, Face1Rec, gindex, &Bit, NULL);

					uint8_t* src = Bit->buffer;
					uint8_t* startOfLine = src;
					
					for (unsigned int y = 0; y < Bit->height; ++y) {
						src = startOfLine;
						startOfLine += Bit->pitch;
						const unsigned int dstY = (y - Bit->top + (PenY + 12));
						//	Check if Y position is within image bounds and clip rect
						if (dstY < static_cast<uint32_t>(clipOld.y) || dstY > HEIGHT || dstY > static_cast<uint32_t>(clipOld.h)) { continue; }
						for (unsigned int x = 0; x < Bit->width; ++x) {
							const unsigned int dstX = x + PenX + dst_Cursor_X;
							const uint8_t value = *src++;
							//	Check if X position is within image bounds and clip rect
							if (dstX < static_cast<uint32_t>(clipOld.x) || dstX > WIDTH || dstX > static_cast<uint32_t>(clipOld.w)) { continue; }
							//	y * dst_Pitch	== Image Row Destination Bit
							//	x * 4			== Image Column Destination Bit
							const unsigned int dstBit = (dstY * dst_Pitch) + (dstX * 4);

							writeBuffer[dstBit]		= 0xff;		// +0 == B
							writeBuffer[dstBit +1]	= 0xff;		// +1 == G
							writeBuffer[dstBit +2]	= 0xff;		// +2 == R
							writeBuffer[dstBit +3]	= value;	// +3 == A
						}
					}
					dst_Cursor_X += Bit->xadvance;
				}
			}

			virtual void RenderText(Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text) {
				int x = pos.x;
				int y = pos.y;
				Translate(x, y);
				DrawText(text, x, y);
			}

			bool FirstCopy = true;

			//
			//	GUI Vertex/Index Buffers
			//
			void createGUIBuffers() {
				//	Vertex Buffer
				VkBufferCreateInfo vertexBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				vertexBufferInfo.size = vertexBufferSize;
				vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VmaAllocationCreateInfo vertexAllocInfo = {};
				vertexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				vertexAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

				vmaCreateBuffer(_Driver->allocator, &vertexBufferInfo, &vertexAllocInfo, &GUI_VertexBuffer, &GUI_VertexAllocation, nullptr);
				printf("Create GUI Vertex Buffer\n");
				//
				//	Index Buffer
				VkBufferCreateInfo indexBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				indexBufferInfo.size = indexBufferSize;
				indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VmaAllocationCreateInfo indexAllocInfo = {};
				indexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				indexAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

				vmaCreateBuffer(_Driver->allocator, &indexBufferInfo, &indexAllocInfo, &GUI_IndexBuffer, &GUI_IndexAllocation, nullptr);
				printf("Create GUI Index Buffer\n");
			}
			//
			//	Font Texture
			//
			void createFontVertexBuffer() {
				VkBufferCreateInfo vertexBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				vertexBufferInfo.size = sizeof(Vertex) * 6;
				vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VmaAllocationCreateInfo vertexAllocInfo = {};
				vertexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				vertexAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

				vmaCreateBuffer(_Driver->allocator, &vertexBufferInfo, &vertexAllocInfo, &Font_VertexBuffer, &Font_VertexAllocation, nullptr);

				writeBuffer.resize(dst_Size);

				printf("Create Font Vertex Buffer\n");
			}
			void createFontTextureBuffer() {
				VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
				imageInfo.imageType = VK_IMAGE_TYPE_2D;
				imageInfo.extent.width = static_cast<uint32_t>(WIDTH);
				imageInfo.extent.height = static_cast<uint32_t>(HEIGHT);
				imageInfo.extent.depth = 1;
				imageInfo.mipLevels = 1;
				imageInfo.arrayLayers = 1;
				imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
				imageInfo.tiling = VK_IMAGE_TILING_LINEAR;
				imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageInfo.usage =  VK_IMAGE_USAGE_SAMPLED_BIT;
				imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

				VmaAllocationCreateInfo textureAllocInfo = {};
				textureAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				textureAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

				VmaAllocationInfo imageBufferAllocInfo = {};
				vmaCreateImage(_Driver->allocator, &imageInfo, &textureAllocInfo, &Font_TextureImage, &Font_TextureAllocation, nullptr);

				VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
				textureImageViewInfo.image = Font_TextureImage;
				textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				textureImageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
				textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				textureImageViewInfo.subresourceRange.baseMipLevel = 0;
				textureImageViewInfo.subresourceRange.levelCount = 1;
				textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
				textureImageViewInfo.subresourceRange.layerCount = 1;
				vkCreateImageView(_Driver->device, &textureImageViewInfo, nullptr, &Font_TextureImageView);

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
				if (vkCreateSampler(_Driver->device, &samplerInfo, nullptr, &Font_TextureSampler) != VK_SUCCESS) {
#ifdef _DEBUG
					throw std::runtime_error("failed to create texture sampler!");
#endif
				}
				//	Descriptor Pool
				std::array<VkDescriptorPoolSize, 1> poolSizes = {};
				poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				poolSizes[0].descriptorCount = static_cast<uint32_t>(_Driver->swapChainImages.size());

				VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
				poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
				poolInfo.pPoolSizes = poolSizes.data();
				poolInfo.maxSets = static_cast<uint32_t>(_Driver->swapChainImages.size());

				if (vkCreateDescriptorPool(_Driver->device, &poolInfo, nullptr, &FontTexture_DescriptorPool) != VK_SUCCESS) {
#ifdef _DEBUG
					throw std::runtime_error("failed to create descriptor pool!");
#endif
				}
				//	Descriptor Sets
				auto Pipe = _Driver->_MaterialCache->GetPipe(Pipelines::GUI);
				std::vector<VkDescriptorSetLayout> layouts(_Driver->swapChainImages.size(), Pipe->descriptorSetLayout);
				VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
				allocInfo.descriptorPool = FontTexture_DescriptorPool;
				allocInfo.descriptorSetCount = static_cast<uint32_t>(_Driver->swapChainImages.size());
				allocInfo.pSetLayouts = layouts.data();

				FontTexture_DescriptorSets.resize(_Driver->swapChainImages.size());
				if (vkAllocateDescriptorSets(_Driver->device, &allocInfo, FontTexture_DescriptorSets.data()) != VK_SUCCESS) {
#ifdef _DEBUG
					throw std::runtime_error("failed to allocate descriptor sets!");
#endif
				}
				for (size_t i = 0; i < _Driver->swapChainImages.size(); i++) {

					VkDescriptorImageInfo imageInfo = {};
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = Font_TextureImageView;
					imageInfo.sampler = Font_TextureSampler;

					std::array<VkWriteDescriptorSet, 1> descriptorWrites = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

					descriptorWrites[0].dstSet = FontTexture_DescriptorSets[i];
					descriptorWrites[0].dstBinding = 0;
					descriptorWrites[0].dstArrayElement = 0;
					descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					descriptorWrites[0].descriptorCount = 1;
					descriptorWrites[0].pImageInfo = &imageInfo;

					vkUpdateDescriptorSets(_Driver->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
				}
			}
			//
			//	Indirect Drawing
			//
			void createIndirectBuffer() {
				VkBufferCreateInfo indirectBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				indirectBufferInfo.size = sizeof(VkDrawIndirectCommand) * 1024;
				indirectBufferInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
				indirectBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VmaAllocationCreateInfo indirectAllocInfo = {};
				indirectAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				indirectAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

				vmaCreateBuffer(_Driver->allocator, &indirectBufferInfo, &indirectAllocInfo, &indirectBuffer, &indirectAllocation, nullptr);

				printf("Create Indirect Buffer\n");
			}
		public:

			void SetBuffer(size_t BufferNumber) {
				currentBuffer = BufferNumber;
			}

			VkCommandBuffer GetBuffer(size_t BufferNumber) {
				return commandBuffers[BufferNumber];
			}

			Vulkan(VulkanDriver* Driver) : _Driver(Driver) {}
			~Vulkan() {
				printf("Delete GWEN\n");
				vmaDestroyBuffer(_Driver->allocator, indirectBuffer, indirectAllocation);
				vmaDestroyBuffer(_Driver->allocator, GUI_VertexBuffer, GUI_VertexAllocation);
				vmaDestroyBuffer(_Driver->allocator, GUI_IndexBuffer, GUI_IndexAllocation);
				vmaDestroyBuffer(_Driver->allocator, Font_VertexBuffer, Font_VertexAllocation);
				vkDestroySampler(_Driver->device, Font_TextureSampler, nullptr);
				vkDestroyImageView(_Driver->device, Font_TextureImageView, nullptr);
				vmaDestroyImage(_Driver->allocator, Font_TextureImage, Font_TextureAllocation);
				vkDestroyDescriptorPool(_Driver->device, FontTexture_DescriptorPool, nullptr);
			}

			virtual void Init() {
				printf("GWEN Vulkan Renderer Initialize\n");
				initFreeType();
				for (size_t i = 0; i < _Driver->swapChainImages.size(); i++) {
					auto NewBuffer = _Driver->_SceneGraph->newCommandBuffer();
					commandBuffers.insert(commandBuffers.end(), NewBuffer.begin(), NewBuffer.end());
				}

				createIndirectBuffer();
				createGUIBuffers();
				createFontVertexBuffer();
				createFontTextureBuffer();
			}

			virtual void Begin()
			{
				vkResetCommandBuffer(commandBuffers[currentBuffer], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
				VkCommandBufferInheritanceInfo inheritanceInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
				inheritanceInfo.renderPass = _Driver->renderPass;

				VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
				beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
				beginInfo.pInheritanceInfo = &inheritanceInfo;

				vkBeginCommandBuffer(commandBuffers[currentBuffer], &beginInfo);
				auto Pipe = _Driver->_MaterialCache->GetPipe(Pipelines::GUI);

				VkRect2D scissor = {};
				scissor.offset = { 0, 0 };
				scissor.extent = _Driver->swapChainExtent;
				vkCmdSetScissor(commandBuffers[currentBuffer], 0, 1, &scissor);

				vkCmdBindDescriptorSets(commandBuffers[currentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, Pipe->pipelineLayout, 0, 1, &FontTexture_DescriptorSets[currentBuffer], 0, nullptr);

				if (FirstCopy) {
					auto CB = _Driver->_SceneGraph->beginSingleTimeCommands();
					VkImageMemoryBarrier imgMemBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
					imgMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					imgMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
					imgMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					imgMemBarrier.subresourceRange.baseMipLevel = 0;
					imgMemBarrier.subresourceRange.levelCount = 1;
					imgMemBarrier.subresourceRange.baseArrayLayer = 0;
					imgMemBarrier.subresourceRange.layerCount = 1;
					imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
					imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imgMemBarrier.image = Font_TextureImage;
					imgMemBarrier.srcAccessMask = 0;
					imgMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

					vkCmdPipelineBarrier(
						CB,
						VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
						VK_PIPELINE_STAGE_TRANSFER_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &imgMemBarrier);
					FirstCopy = false;
					_Driver->_SceneGraph->endSingleTimeCommands(CB);
				}
				vkCmdBindPipeline(commandBuffers[currentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, Pipe->graphicsPipeline);

				VkBuffer vertexBuffers[1] = { GUI_VertexBuffer };
				VkDeviceSize offsets[1] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[currentBuffer], 0, 1, vertexBuffers, offsets);
				vkCmdBindIndexBuffer(commandBuffers[currentBuffer], GUI_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
			}
			virtual void End()
			{
				//	Vertex Buffer CPU->GPU Copy
				drawVertices.swap(writeVertices);
				memcpy(GUI_VertexAllocation->GetMappedData(), drawVertices.data(), sizeof(Vertex) * drawVertices.size());
				writeVertices.clear();

				//	Index Buffer CPU->GPU Copy
				drawIndices.swap(writeIndices);
				memcpy(GUI_IndexAllocation->GetMappedData(), drawIndices.data(), sizeof(uint32_t) * drawIndices.size());
				writeIndices.clear();

				//	Indirect Buffer CPU->GPU Copy
				memcpy(indirectAllocation->GetMappedData(), indirectCommands.data(), sizeof(VkDrawIndirectCommand) * indirectCommands.size());
				CurIndirectDraw = 0;
				TotalDrawIndex = 0;
				indirectCommands.clear();
				TotalDrawVerts = 0;

				drawUnique_Vertices.swap(writeUnique_Vertices);
				writeUnique_Vertices.clear();

				auto Pipe = _Driver->_MaterialCache->GetPipe(Pipelines::GUI);
				vkCmdBindDescriptorSets(commandBuffers[currentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, Pipe->pipelineLayout, 0, 1, &FontTexture_DescriptorSets[currentBuffer], 0, nullptr);

				auto CB = _Driver->_SceneGraph->beginSingleTimeCommands();
				VkImageMemoryBarrier imgMemBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
				imgMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imgMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				imgMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imgMemBarrier.subresourceRange.baseMipLevel = 0;
				imgMemBarrier.subresourceRange.levelCount = 1;
				imgMemBarrier.subresourceRange.baseArrayLayer = 0;
				imgMemBarrier.subresourceRange.layerCount = 1;
				imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
				imgMemBarrier.image = Font_TextureImage;
				imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(
					CB,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &imgMemBarrier);

				drawBuffer.swap(writeBuffer);
				memcpy(Font_TextureAllocation->GetMappedData(), drawBuffer.data(), drawBuffer.size());
				writeBuffer.clear();
				writeBuffer.resize(dst_Size);

				imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
				imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imgMemBarrier.image = Font_TextureImage;
				imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(
					CB,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					0,
					0, nullptr,
					0, nullptr,
					1, &imgMemBarrier);
				_Driver->_SceneGraph->endSingleTimeCommands(CB);

				memcpy(Font_VertexAllocation->GetMappedData(), Font_TextureVertices.data(), sizeof(Vertex) * Font_TextureVertices.size());
				VkBuffer vertexBuffers2[] = { Font_VertexBuffer };
				VkDeviceSize offsets2[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffers[currentBuffer], 0, 1, vertexBuffers2, offsets2);
				vkCmdDraw(commandBuffers[currentBuffer], static_cast<uint32_t>(Font_TextureVertices.size()), 1, 0, 0);
				vkEndCommandBuffer(commandBuffers[currentBuffer]);
			}
			virtual void DrawFilledRect(Gwen::Rect rect)
			{
				Translate(rect);
				AddVert(rect.x, rect.y);
				AddVert(rect.x + rect.w, rect.y);
				AddVert(rect.x, rect.y + rect.h);
				AddVert(rect.x + rect.w, rect.y);
				AddVert(rect.x + rect.w, rect.y + rect.h);
				AddVert(rect.x, rect.y + rect.h);
			}

			void StartClip()
			{
				clipOld = clipNew;
				clipOld.w += clipOld.x;
				clipOld.h += clipOld.y;

				//if (drawIndices.size() > 0) {
				if (CurDrawIndex > 0) {
					VkDrawIndexedIndirectCommand iCommand = {};
					iCommand.firstIndex = TotalDrawIndex - CurDrawIndex;
					iCommand.indexCount = CurDrawIndex;
					iCommand.firstInstance = 0;
					iCommand.instanceCount = 1;
					indirectCommands.push_back(iCommand);

					vkCmdDrawIndexedIndirect(commandBuffers[currentBuffer], indirectBuffer, CurIndirectDraw * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
					CurIndirectDraw++;
					CurDrawIndex = 0;
				}
				clipNew = ClipRegion();
				clipNew.x *= Scale();
				clipNew.y *= Scale();
				clipNew.w *= Scale();
				clipNew.h *= Scale();
				VkRect2D scissor = {};
				scissor.offset = { static_cast<int32_t>(clipNew.x), static_cast<int32_t>(clipNew.y) };
				scissor.extent = { static_cast<uint32_t>(clipNew.w), static_cast<uint32_t>(clipNew.h) };
				vkCmdSetScissor(commandBuffers[currentBuffer], 0, 1, &scissor);
			};

			void DrawTexturedRect(Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1 = 0.0f, float v1 = 0.0f, float u2 = 1.0f, float v2 = 1.0f)
			{
				auto Pipe = _Driver->_MaterialCache->GetPipe(Pipelines::GUI);
				vkCmdBindDescriptorSets(commandBuffers[currentBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS, Pipe->pipelineLayout, 0, 1, &((TextureObject*)pTexture->data)->DescriptorSets[currentBuffer], 0, nullptr);
				Translate(pTargetRect);

				AddVert(pTargetRect.x, pTargetRect.y, u1, v1);
				AddVert(pTargetRect.x + pTargetRect.w, pTargetRect.y, u2, v1);
				AddVert(pTargetRect.x, pTargetRect.y + pTargetRect.h, u1, v2);
				AddVert(pTargetRect.x + pTargetRect.w, pTargetRect.y, u2, v1);
				AddVert(pTargetRect.x + pTargetRect.w, pTargetRect.y + pTargetRect.h, u2, v2);
				AddVert(pTargetRect.x, pTargetRect.y + pTargetRect.h, u1, v2);
			}
			void LoadTexture(Gwen::Texture* pTexture)
			{
				const char* FileName = pTexture->name.c_str();
				TextureObject* Tex = _Driver->_MaterialCache->createTextureImage(FileName);
				if (Tex == nullptr) {
					pTexture->failed = true;
					return;
				}
				pTexture->data = Tex;
				pTexture->width = Tex->Width;
				pTexture->height = Tex->Height;
			}
			void FreeTexture(Gwen::Texture* pTexture)
			{
				TextureObject* tex = (TextureObject* ) pTexture->data;

				if ( !tex ) { return; }

				delete tex;
				pTexture->data = NULL;
			}
			Gwen::Color PixelColour(Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default)
			{
				TextureObject* tex = (TextureObject* ) pTexture->data;

				if ( !tex ) { return col_default; }

				const unsigned int iOffset = ( (y * pTexture->width) + x ) * 4;
				//	iOffset = r; iOffset+1 = g; iOffset+2 = b; iOffset+3 = a;
				return Gwen::Color(tex->Pixels[iOffset], tex->Pixels[iOffset+1], tex->Pixels[iOffset+2], tex->Pixels[iOffset+3]);
			}

			virtual void SetDrawColor(Gwen::Color color)
			{
				m_Color = color;
			}

		protected:

			Gwen::Color	m_Color;

			void AddVert(int x, int y, float u = 0.0f, float v = 0.0f)
			{
				Vertex NewVertex;

				NewVertex.pos.x = ((float)x / 400) - 1.f;
				NewVertex.pos.y = ((float)y / 300) - 1.f;
				//	Our image is 512x512 px
				NewVertex.texCoord.x = u;
				NewVertex.texCoord.y = v;
				//
				NewVertex.color.r = m_Color.r / 128;
				NewVertex.color.g = m_Color.g / 128;
				NewVertex.color.b = m_Color.b / 128;
				NewVertex.color.a = m_Color.a / 128;

				//if (writeUnique_Vertices.count(NewVertex) == 0) {
					writeUnique_Vertices[NewVertex] = TotalDrawVerts;
					writeVertices.push_back(NewVertex);
				//}
				//writeIndices.push_back(writeUnique_Vertices[NewVertex]);
					writeIndices.push_back(TotalDrawVerts);
					TotalDrawVerts++;

				CurDrawIndex++;
				TotalDrawIndex++;
			}
		};
	}
}