#pragma once

#include "Gwen/Gwen.h"
#include "Gwen/BaseRender.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H

#include <unordered_map>
#include <array>
#include <map>

struct GWENTex {
	DescriptorObject* Descriptor;
	TextureObject* Texture;
	~GWENTex() {
		delete Descriptor;
	}
};

typedef struct GWENFaceRec_ {
	const char*	file_path;
	int			face_index;
} GWENFaceRec, * GWENFace;

static FT_Error GWENFaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face* aface)
{
	GWENFace face = (GWENFace)face_id;
	return FT_New_Face(library, face->file_path, face->face_index, aface);
}

namespace Gwen
{
	namespace Renderer
	{
		class Vulkan : public Gwen::Renderer::Base
		{

			Pipeline::GUI* Pipe = nullptr;

			size_t currentBuffer = 0;
			VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

			std::unordered_map<Vertex, const uint32_t> GUI_UniqueVertices = {};

			std::vector<Vertex> GUI_Vertices = {};
			const uint32_t vertexBufferSize = sizeof(Vertex) * 64000;
			std::vector<uint32_t> GUI_Indices = {};
			const uint32_t indexBufferSize = sizeof(uint32_t) * 192000;

			uint32_t CurIndirectDraw = 0;
			uint32_t CurIndexCount = 0;
			uint32_t TotalIndexCount = 0;
			uint32_t TotalVertexCount = 0;

			std::vector<VkDrawIndexedIndirectCommand> indirectCommands;
			VkBuffer indirectBuffer = VK_NULL_HANDLE;
			VmaAllocation indirectAllocation = VMA_NULL;

			VkBuffer GUI_VertexBuffer = VK_NULL_HANDLE;
			VmaAllocation GUI_VertexAllocation = VMA_NULL;

			VkBuffer GUI_IndexBuffer = VK_NULL_HANDLE;
			VmaAllocation GUI_IndexAllocation = VMA_NULL;

			VkBuffer Font_VertexBuffer = VK_NULL_HANDLE;
			VmaAllocation Font_VertexAllocation = VMA_NULL;

			VmaAllocation Font_TextureAllocation = VMA_NULL;
			VkImage Font_TextureImage = VK_NULL_HANDLE;
			VkImageView Font_TextureImageView = VK_NULL_HANDLE;

			DescriptorObject* Font_Descriptor = nullptr;

			FT_Library library = nullptr;

			FTC_Manager manager = nullptr;
			FTC_SBitCache sbitCache = nullptr;
			FTC_CMapCache cmapCache = nullptr;

			std::vector<VkRect2D> ClipScissors = {};
			Gwen::Rect clipOld = {};
			Gwen::Rect clipNew = {};

			Gwen::Color	m_Color = {};

			std::deque<unsigned char*> LastWrites = {};

			const uint32_t dst_Size;
			const uint32_t dst_Pitch;
			GWENFace Face1 = nullptr;
			FTC_Scaler Face1Scale = nullptr;
			FTC_ImageType Face1Rec = nullptr;

			FTC_SBit Bit = nullptr;	//	Glyph returned from character lookup in cache

			void initFreeType()
			{
				Face1 = new GWENFaceRec;
				Face1->file_path = "./media/OpenSans.ttf";
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

			Gwen::Point MeasureText(Gwen::Font* pFont, const Gwen::UnicodeString& text)
			{
				unsigned int dst_Cursor_X = 0;

				for (const wchar_t c : text) {
					const FT_UInt gindex = FTC_CMapCache_Lookup(cmapCache, Face1, -1, c);
					FTC_SBitCache_Lookup(sbitCache, Face1Rec, gindex, &Bit, NULL);

					dst_Cursor_X += Bit->xadvance;
				}
				return Gwen::Point(dst_Cursor_X, Face1Rec->height);
			}

			void DrawText(const Gwen::UnicodeString& text, const unsigned int PenX, const unsigned int PenY)
			{
				unsigned int dst_Cursor_X = 0;

				for (const wchar_t c : text) {
					const FT_UInt gindex = FTC_CMapCache_Lookup(cmapCache, Face1, -1, c);
					FTC_SBitCache_Lookup(sbitCache, Face1Rec, gindex, &Bit, NULL);

					uint8_t* src = Bit->buffer;
					uint8_t* startOfLine = src;
					for (unsigned int y = 0; y < Bit->height; ++y) {
						src = startOfLine;
						startOfLine += Bit->pitch;
						const unsigned int dstY = (y - Bit->top + (PenY + Face1Rec->height));
						//	Check if Y position is within image bounds and clip rect
						if (dstY <= static_cast<uint32_t>(clipOld.y) || dstY >= WorldEngine::VulkanDriver::HEIGHT || dstY >= static_cast<uint32_t>(clipOld.h)) { continue; }
						for (unsigned int x = 0; x < Bit->width; ++x) {
							const unsigned int dstX = x + PenX + dst_Cursor_X;
							const uint8_t value = *src++;
							//
							//	Skip writing 0 alpha pixels
							if (value == 0) { continue; }
							//
							//	Check if X position is within image bounds and clip rect
							if (dstX <= static_cast<uint32_t>(clipOld.x) || dstX >= WorldEngine::VulkanDriver::WIDTH || dstX >= static_cast<uint32_t>(clipOld.w)) { continue; }
							//	y * dst_Pitch	== Image Row Destination Bit
							//	x * 4			== Image Column Destination Bit
							const unsigned int dstBit = (dstY * dst_Pitch) + (dstX * 4);


							//writeBuffer[dstBit]		= m_Color.r;// +0 == R
							//writeBuffer[dstBit +1]	= m_Color.g;// +1 == G
							//writeBuffer[dstBit +2]	= m_Color.b;// +2 == B
							//writeBuffer[dstBit +3]	= value;	// +3 == A
							unsigned char* TextureData = reinterpret_cast<unsigned char*>(Font_TextureAllocation->GetMappedData());
							//
							//	Write changed pixels
							TextureData[dstBit] = m_Color.r;
							TextureData[dstBit+1] = m_Color.g;
							TextureData[dstBit+2] = m_Color.b;
							//
							//	Store the alpha position into our 'LastWrites' buffer before setting the value
							LastWrites.push_back(&TextureData[dstBit + 3]);
							*LastWrites.back() = value;
						}
					}
					dst_Cursor_X += Bit->xadvance;
				}
			}
			
			void RenderText(Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text)
			{
				auto Clp = clipNew;
				int ClpX = Clp.x;
				int ClpY = Clp.y;
				int ClpW = ClpX + Clp.w;
				int ClpH = ClpY + Clp.h;
				int x = pos.x;
				int y = pos.y;
				Translate(x, y);
				//printf("%i (%i) %i | %i (%i) %i\n", ClpX, x, ClpW, ClpY, y, ClpH);
				//printf("%i %i\n", x, y);
				if (x >= ClpX && y >= ClpY && x <= ClpW && y <= ClpH) {
					DrawText(text, x, y);
				}
			}

			//
			//	GUI Vertex/Index Buffers
			//
			void createGUIBuffers()
			{
				//	Vertex Buffer
				VkBufferCreateInfo vertexBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				vertexBufferInfo.size = vertexBufferSize;
				vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
				vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VmaAllocationCreateInfo vertexAllocInfo = {};
				vertexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				vertexAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

				vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &vertexBufferInfo, &vertexAllocInfo, &GUI_VertexBuffer, &GUI_VertexAllocation, nullptr);
				//	Vertex Buffer
				VkBufferCreateInfo indexBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				indexBufferInfo.size = vertexBufferSize;
				indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
				indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VmaAllocationCreateInfo indexAllocInfo = {};
				indexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				indexAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

				vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &indexBufferInfo, &indexAllocInfo, &GUI_IndexBuffer, &GUI_IndexAllocation, nullptr);
			}
			//
			//	Font Texture
			//
			void createFontVertexBuffer()
			{
				const std::vector<Vertex> Font_TextureVertices = {
				{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
				{{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
				{{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
				{{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
				{{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
				{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}
				};

				VkBuffer Font_VertexBuffer_Stage = VK_NULL_HANDLE;
				VmaAllocation Font_VertexAllocation_Stage = VMA_NULL;

				VkBufferCreateInfo vertexBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				vertexBufferInfo.size = sizeof(Vertex) * Font_TextureVertices.size();
				vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VmaAllocationCreateInfo vertexAllocInfo = {};
				vertexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				vertexAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

				vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &vertexBufferInfo, &vertexAllocInfo, &Font_VertexBuffer_Stage, &Font_VertexAllocation_Stage, nullptr);

				vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

				vertexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
				vertexAllocInfo.flags = 0;
				
				vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &vertexBufferInfo, &vertexAllocInfo, &Font_VertexBuffer, &Font_VertexAllocation, nullptr);

				VkBufferCopy copyRegion = {};
				copyRegion.size = vertexBufferInfo.size;

				memcpy(Font_VertexAllocation_Stage->GetMappedData(), Font_TextureVertices.data(), vertexBufferInfo.size);

				auto CB = WorldEngine::VulkanDriver::beginSingleTimeCommands();
				vkCmdCopyBuffer(CB, Font_VertexBuffer_Stage, Font_VertexBuffer, 1, &copyRegion);
				WorldEngine::VulkanDriver::endSingleTimeCommands(CB);

				vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, Font_VertexBuffer_Stage, Font_VertexAllocation_Stage);
			}
			void createFontTextureBuffer()
			{
				VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
				imageInfo.imageType = VK_IMAGE_TYPE_2D;
				imageInfo.extent.width = static_cast<uint32_t>(WorldEngine::VulkanDriver::WIDTH);
				imageInfo.extent.height = static_cast<uint32_t>(WorldEngine::VulkanDriver::HEIGHT);
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
				textureAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

				vmaCreateImage(WorldEngine::VulkanDriver::allocator, &imageInfo, &textureAllocInfo, &Font_TextureImage, &Font_TextureAllocation, nullptr);

				VkImageViewCreateInfo textureImageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
				textureImageViewInfo.image = Font_TextureImage;
				textureImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				textureImageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
				textureImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				textureImageViewInfo.subresourceRange.baseMipLevel = 0;
				textureImageViewInfo.subresourceRange.levelCount = 1;
				textureImageViewInfo.subresourceRange.baseArrayLayer = 0;
				textureImageViewInfo.subresourceRange.layerCount = 1;
				vkCreateImageView(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &textureImageViewInfo, nullptr, &Font_TextureImageView);

				//
				//	Transition image from initial state to workable state
				auto CB = WorldEngine::VulkanDriver::beginSingleTimeCommands();
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

				WorldEngine::VulkanDriver::endSingleTimeCommands(CB);

				Font_Descriptor = Pipe->createRawDescriptor(Font_TextureImageView, Pipe->Sampler);
			}
			//
			//	Indirect Drawing
			//
			void createIndirectBuffer()
			{
				VkBufferCreateInfo indirectBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				indirectBufferInfo.size = sizeof(VkDrawIndexedIndirectCommand) * 1024;
				indirectBufferInfo.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
				indirectBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				VmaAllocationCreateInfo indirectAllocInfo = {};
				indirectAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
				indirectAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

				vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &indirectBufferInfo, &indirectAllocInfo, &indirectBuffer, &indirectAllocation, nullptr);
			}

			void AddVert(const int x, const int y, const float u = 0.0f, const float v = 0.0f)
			{
				Vertex NewVertex{};

				NewVertex.pos.x = ((float)x / (WorldEngine::VulkanDriver::WIDTH/2)) - 1.f;
				NewVertex.pos.y = ((float)y / (WorldEngine::VulkanDriver::HEIGHT/2)) - 1.f;
				NewVertex.texCoord.x = u;
				NewVertex.texCoord.y = v;
				NewVertex.color.r = m_Color.r / 128;
				NewVertex.color.g = m_Color.g / 128;
				NewVertex.color.b = m_Color.b / 128;
				NewVertex.color.a = m_Color.a / 128;

				if (GUI_UniqueVertices.count(NewVertex) == 0) {
					GUI_UniqueVertices.emplace(NewVertex, TotalVertexCount++);
					GUI_Vertices.emplace_back(NewVertex);
				}
				GUI_Indices.push_back(GUI_UniqueVertices[NewVertex]);

				CurIndexCount++;
				TotalIndexCount++;
			}
		public:

			void SetBuffer(const VkCommandBuffer& Buff) {
				commandBuffer = Buff;
			}

			Vulkan()
				: dst_Size((WorldEngine::VulkanDriver::WIDTH* WorldEngine::VulkanDriver::HEIGHT) * 4), dst_Pitch(WorldEngine::VulkanDriver::WIDTH * 4)
			{
				Pipe = WorldEngine::MaterialCache::GetPipe_GUI();
			}

			~Vulkan()
			{
				printf("Destroy GWEN\n");
				vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, indirectBuffer, indirectAllocation);
				vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, GUI_VertexBuffer, GUI_VertexAllocation);
				vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, GUI_IndexBuffer, GUI_IndexAllocation);
				vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, Font_VertexBuffer, Font_VertexAllocation);
				vkDestroyImageView(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, Font_TextureImageView, nullptr);
				vmaDestroyImage(WorldEngine::VulkanDriver::allocator, Font_TextureImage, Font_TextureAllocation);
				delete Font_Descriptor;
			}

			void Init()
			{
				printf("GWEN Vulkan Renderer Initialize\n");
				initFreeType();
				const size_t SwapChainCount = WorldEngine::VulkanDriver::swapChain.images.size();
				ClipScissors.resize(SwapChainCount);

				createIndirectBuffer();
				createGUIBuffers();
				createFontVertexBuffer();
				createFontTextureBuffer();
			}

			//
			//	Begin Render
			void Begin()
			{
				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipe->graphicsPipeline);

				VkDeviceSize offsets[1] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &GUI_VertexBuffer, offsets);
				vkCmdBindIndexBuffer(commandBuffer, GUI_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
				//
				//	Clear our previous font texture by setting alphas to 0
				while (!LastWrites.empty())
				{
					*LastWrites.front() = 0;
					LastWrites.pop_front();
				}

				//WorldEngine::VulkanDriver::pool.parallelize_loop(0, LastWrites.size(),
				//	[this](const uint32_t& a, const uint32_t& b)
				//	{
				//		for (uint32_t i = a; i < b; i++)
				//		{
				//			*LastWrites[i] = 0;
				//		}
				//	});
				//WorldEngine::VulkanDriver::pool.wait_for_tasks();
				LastWrites.clear();
			}
			void DrawFilledRect(Gwen::Rect rect)
			{
				Translate(rect);
				Gwen::Rect CLR = clipNew;
				for (unsigned int X = rect.x; X <= rect.x+rect.w; X++)
				{
					// X = Column Position
					if (X <= static_cast<uint32_t>(CLR.x) || X >= WorldEngine::VulkanDriver::WIDTH || X >= static_cast<uint32_t>(CLR.x+CLR.w)) { continue; }
					for (unsigned int Y = rect.y; Y <= rect.y + rect.h; Y++)
					{
						// Y = Row Position
						if (Y <= static_cast<uint32_t>(CLR.y) || Y >= WorldEngine::VulkanDriver::HEIGHT || Y >= static_cast<uint32_t>(CLR.y+CLR.h)) { continue; }
						//
						//	Calculate Pixel Bit Position
						unsigned int PixelPos = (X * 4) + (Y * dst_Pitch);
						//
						//	Draw Pixel
						unsigned char* TextureData = reinterpret_cast<unsigned char*>(Font_TextureAllocation->GetMappedData());
						TextureData[PixelPos] = m_Color.r;
						TextureData[PixelPos + 1] = m_Color.g;
						TextureData[PixelPos + 2] = m_Color.b;
						//TextureData[PixelPos + 3] = m_Color.a;
						//
						//	Store the alpha position into our 'LastWrites' buffer before setting the value
						LastWrites.push_back(&TextureData[PixelPos + 3]);
						*LastWrites.back() = m_Color.a;
						//printf("draw filled");
					}
				}
			}
			void DrawLinedRect(Gwen::Rect rect)
			{
				printf("draw lined");
			}
			void DrawPixel(int x, int y)
			{
				Translate(x, y);

				const unsigned int dstY = y;
				//	Check if Y position is within image bounds and clip rects
				if (dstY <= static_cast<uint32_t>(clipOld.y) || dstY >= WorldEngine::VulkanDriver::HEIGHT || dstY >= static_cast<uint32_t>(clipOld.h)) { return; }
				const unsigned int dstX = x;
				//const uint8_t value = *src++;
				//	Check if X position is within image bounds and clip rect
				if (dstX <= static_cast<uint32_t>(clipOld.x) || dstX >= WorldEngine::VulkanDriver::WIDTH || dstX >= static_cast<uint32_t>(clipOld.w)) { return; }
				//	y * dst_Pitch	== Image Row Destination Bit
				//	x * 4			== Image Column Destination Bit
				const unsigned int dstBit = (dstY * dst_Pitch) + (dstX * 4);

				//writeBuffer[dstBit]		= m_Color.r;// +0 == R
				//writeBuffer[dstBit +1]	= m_Color.g;// +1 == G
				//writeBuffer[dstBit +2]	= m_Color.b;// +2 == B
				//writeBuffer[dstBit +3]	= value;	// +3 == A
				unsigned char* TextureData = reinterpret_cast<unsigned char*>(Font_TextureAllocation->GetMappedData());
				TextureData[dstBit] = m_Color.r;
				TextureData[dstBit + 1] = m_Color.g;
				TextureData[dstBit + 2] = m_Color.b;
				//
				//	Store the alpha position into our 'LastWrites' buffer before setting the value
				LastWrites.push_back(&TextureData[dstBit + 3]);
				*LastWrites.back() = m_Color.a;
				printf("draw pixel");
			}
			//
			//	Start Clip
			void StartClip()
			{
				clipOld = clipNew;
				clipOld.w += clipOld.x;
				clipOld.h += clipOld.y;
				clipNew = ClipRegion();
				clipNew.x *= Scale();
				clipNew.y *= Scale();
				clipNew.w *= Scale();
				clipNew.h *= Scale();

				if (CurIndexCount > 0) {
					VkDrawIndexedIndirectCommand iCommand = {};
					iCommand.firstIndex = TotalIndexCount - CurIndexCount;
					iCommand.indexCount = CurIndexCount;
					iCommand.firstInstance = 0;
					iCommand.instanceCount = 1;
					indirectCommands.emplace_back(iCommand);
					//	Clear FontTexture Inside ClipRect
					/*for (unsigned int dstY = clipOld.y; dstY < clipOld.h; ++dstY) {
						for (unsigned int dstX = clipOld.x; dstX < clipOld.w; ++dstX) {
							const unsigned int dstBit = (dstY * dst_Pitch) + (dstX * 4);
							writeBuffer[dstBit] = 0;// +0 == R
							writeBuffer[dstBit + 1] = 0;// +1 == G
							writeBuffer[dstBit + 2] = 0;// +2 == B
							writeBuffer[dstBit + 3] = 0;	// +3 == A
						}
					}*/

					vkCmdDrawIndexedIndirect(commandBuffer, indirectBuffer, CurIndirectDraw * sizeof(VkDrawIndexedIndirectCommand), 1, sizeof(VkDrawIndexedIndirectCommand));
					CurIndirectDraw++;
					CurIndexCount = 0;
				}
				ClipScissors[currentBuffer].offset = { static_cast<int32_t>(clipNew.x), static_cast<int32_t>(clipNew.y) };
				ClipScissors[currentBuffer].extent = { static_cast<uint32_t>(clipNew.w), static_cast<uint32_t>(clipNew.h) };
				vkCmdSetScissor(commandBuffer, 0, 1, &ClipScissors[currentBuffer]);
			};
			//
			//	End Render
			void End()
			{
				//	Vertex Buffer CPU->GPU Copy
				memcpy(GUI_VertexAllocation->GetMappedData(), GUI_Vertices.data(), sizeof(Vertex) * GUI_Vertices.size());

				//	Index Buffer CPU->GPU Copy
				memcpy(GUI_IndexAllocation->GetMappedData(), GUI_Indices.data(), sizeof(uint32_t) * GUI_Indices.size());

				//	Indirect Buffer CPU->GPU Copy
				memcpy(indirectAllocation->GetMappedData(), indirectCommands.data(), sizeof(VkDrawIndexedIndirectCommand) * indirectCommands.size());

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipe->pipelineLayout, 0, 1, &Font_Descriptor->DescriptorSets[currentBuffer], 0, nullptr);

				//const VkCommandBuffer &CB = WorldEngine::VulkanDriver::beginSingleTimeCommands();
				//VkImageMemoryBarrier imgMemBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
				//imgMemBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				//imgMemBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				//imgMemBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				//imgMemBarrier.subresourceRange.baseMipLevel = 0;
				//imgMemBarrier.subresourceRange.levelCount = 1;
				//imgMemBarrier.subresourceRange.baseArrayLayer = 0;
				//imgMemBarrier.subresourceRange.layerCount = 1;
				//imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				//imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
				//imgMemBarrier.image = Font_TextureImage;
				//imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				//imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				//
				//vkCmdPipelineBarrier(
				//	CB,
				//	VK_PIPELINE_STAGE_TRANSFER_BIT,
				//	VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				//	0,
				//	0, nullptr,
				//	0, nullptr,
				//	1, &imgMemBarrier);

				//imgMemBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
				//imgMemBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				//imgMemBarrier.image = Font_TextureImage;
				//imgMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				//imgMemBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				//vkCmdPipelineBarrier(
				//	CB,
				//	VK_PIPELINE_STAGE_TRANSFER_BIT,
				//	VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				//	0,
				//	0, nullptr,
				//	0, nullptr,
				//	1, &imgMemBarrier);
				//WorldEngine::VulkanDriver::endSingleTimeCommands(CB);

				VkDeviceSize offsets2[] = { 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, 1, &Font_VertexBuffer, offsets2);
				vkCmdDraw(commandBuffer, Font_VertexAllocation->GetSize(), 1, 0, 0);

				CurIndirectDraw = 0;
				TotalVertexCount = 0;
				TotalIndexCount = 0;
				indirectCommands.clear();
				GUI_Vertices.clear();
				GUI_Indices.clear();
				GUI_UniqueVertices.clear();
			}
			void DrawTexturedRect(Gwen::Texture* pTexture, Gwen::Rect pTargetRect, float u1 = 0.0f, float v1 = 0.0f, float u2 = 1.0f, float v2 = 1.0f)
			{
				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipe->pipelineLayout, 0, 1, &(static_cast<GWENTex*>(pTexture->data)->Descriptor)->DescriptorSets[currentBuffer], 0, nullptr);
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
				TextureObject* Tex = Pipe->createTextureImage(std::string(pTexture->name.c_str()));
				DescriptorObject* Descriptor = Pipe->createDescriptor(Tex);
				GWENTex* TexObj = new GWENTex;
				TexObj->Descriptor = Descriptor;
				TexObj->Texture = Tex;
				if (Tex == nullptr) {
					printf("GWEN: Load Texture Failed\n");
					pTexture->failed = true;
					return;
				}
				pTexture->data = TexObj;
				pTexture->width = Tex->Width;
				pTexture->height = Tex->Height;
			}
			void FreeTexture(Gwen::Texture* pTexture)
			{
				GWENTex* TexObj = static_cast<GWENTex*>(pTexture->data);

				if ( !TexObj) { return; }

				delete TexObj;
				pTexture->data = NULL;
			}
			Gwen::Color PixelColour(Gwen::Texture* pTexture, unsigned int x, unsigned int y, const Gwen::Color& col_default)
			{
				GWENTex* TexObj = static_cast<GWENTex*>(pTexture->data);

				if ( !TexObj) { return col_default; }

				const unsigned int iOffset = ( (y * pTexture->width) + x ) * 4;
				//	iOffset = r; iOffset+1 = g; iOffset+2 = b; iOffset+3 = a;
				return Gwen::Color(TexObj->Texture->Pixels[iOffset], TexObj->Texture->Pixels[iOffset+1], TexObj->Texture->Pixels[iOffset+2], TexObj->Texture->Pixels[iOffset+3]);
			}
			void SetDrawColor(Gwen::Color color)
			{
				m_Color = color;
			}
		};
	}
}