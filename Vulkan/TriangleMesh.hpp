#pragma once
#include <vector>
#include <array>

#include <chrono>

class TriangleMesh {

public:

	VulkanDriver* _Driver = VK_NULL_HANDLE;
	Pipeline::Default* Pipe;

	GLTFInfo* _GLTF;
	size_t vertexBufferSize;
	size_t indexBufferSize;


	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VmaAllocation vertexAllocation = VMA_NULL;

	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VmaAllocation indexAllocation = VMA_NULL;

	std::vector<VkBuffer> uniformBuffers = {};
	std::vector<VmaAllocation> uniformAllocations = {};

	TextureObject* Texture_Albedo;
	TextureObject* Texture_Normal;
	DescriptorObject* Descriptor;

public:
	
	TriangleMesh(VulkanDriver* Driver, Pipeline::Default* Pipeline, GLTFInfo* GLTF, TextureObject* Albedo, TextureObject* Normal)
		: _Driver(Driver), Pipe(Pipeline), _GLTF(GLTF), vertexBufferSize(sizeof(Vertex)* GLTF->Vertices.size()), indexBufferSize(sizeof(uint32_t)* GLTF->Indices.size()) {
		createVertexBuffer();
		createUniformBuffers();
		Texture_Albedo = Albedo;
		Texture_Normal = Normal;
		Descriptor = Pipe->createDescriptor(Albedo, Normal, uniformBuffers);
	}

	~TriangleMesh() {
		printf("Destroy TriangleMesh\n");
		//	Destroy VMA Buffers
		vmaDestroyBuffer(_Driver->allocator, vertexBuffer, vertexAllocation);
		vmaDestroyBuffer(_Driver->allocator, indexBuffer, indexAllocation);
		//	Destroy VMA Buffers
		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			vmaDestroyBuffer(_Driver->allocator, uniformBuffers[i], uniformAllocations[i]);
		}
		delete Descriptor;
	}


	void createUniformBuffers()
	{
		uniformBuffers.resize(_Driver->swapChain.images.size());
		uniformAllocations.resize(_Driver->swapChain.images.size());

		for (size_t i = 0; i < _Driver->swapChain.images.size(); i++) {

			VkBufferCreateInfo uniformBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			uniformBufferInfo.size = sizeof(UniformBufferObject);
			uniformBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			uniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo uniformAllocInfo = {};
			uniformAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			uniformAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			vmaCreateBuffer(_Driver->allocator, &uniformBufferInfo, &uniformAllocInfo, &uniformBuffers[i], &uniformAllocations[i], nullptr);
		}
	}

	void createVertexBuffer() {
		//
		//	Vertex Buffer
		VkBufferCreateInfo vertexBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		vertexBufferInfo.size = vertexBufferSize;
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vertexAllocInfo = {};
		vertexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		vertexAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkBuffer stagingVertexBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingVertexBufferAlloc = VK_NULL_HANDLE;
		vmaCreateBuffer(_Driver->allocator, &vertexBufferInfo, &vertexAllocInfo, &stagingVertexBuffer, &stagingVertexBufferAlloc, nullptr);

		memcpy(stagingVertexBufferAlloc->GetMappedData(), _GLTF->Vertices.data(), vertexBufferSize);

		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vertexAllocInfo.flags = 0;
		vmaCreateBuffer(_Driver->allocator, &vertexBufferInfo, &vertexAllocInfo, &vertexBuffer, &vertexAllocation, nullptr);

		//
		//	Index Buffer
		VkBufferCreateInfo indexBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		indexBufferInfo.size = indexBufferSize;
		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo indexAllocInfo = {};
		indexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		indexAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkBuffer stagingIndexBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingIndexBufferAlloc = VK_NULL_HANDLE;
		vmaCreateBuffer(_Driver->allocator, &indexBufferInfo, &indexAllocInfo, &stagingIndexBuffer, &stagingIndexBufferAlloc, nullptr);

		memcpy(stagingIndexBufferAlloc->GetMappedData(), _GLTF->Indices.data(), indexBufferSize);

		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		indexAllocInfo.flags = 0;
		vmaCreateBuffer(_Driver->allocator, &indexBufferInfo, &indexAllocInfo, &indexBuffer, &indexAllocation, nullptr);

		//
		//	CPU->GPU Copy
		VkCommandBuffer commandBuffer = _Driver->beginSingleTimeCommands();

		VkBufferCopy vertexCopyRegion = {};
		vertexCopyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(commandBuffer, stagingVertexBuffer, vertexBuffer, 1, &vertexCopyRegion);

		VkBufferCopy indexCopyRegion = {};
		indexCopyRegion.size = indexBufferInfo.size;
		vkCmdCopyBuffer(commandBuffer, stagingIndexBuffer, indexBuffer, 1, &indexCopyRegion);

		_Driver->endSingleTimeCommands(commandBuffer);

		//
		//	Destroy Staging Buffers
		vmaDestroyBuffer(_Driver->allocator, stagingVertexBuffer, stagingVertexBufferAlloc);
		vmaDestroyBuffer(_Driver->allocator, stagingIndexBuffer, stagingIndexBufferAlloc);
	}

	void draw(const VkCommandBuffer& CmdBuffer, uint32_t CurFrame)
	{
		//	Bind Descriptor Sets
		vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipe->pipelineLayout, 0, 1, &Descriptor->DescriptorSets[CurFrame], 0, nullptr);
			
		//	Draw Vertex Buffer
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CmdBuffer, 0, 1, &vertexBuffer, offsets);
		vkCmdBindIndexBuffer(CmdBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(CmdBuffer, static_cast<uint32_t>(_GLTF->Indices.size()), 1, 0, 0, 0);
	}

	void updateUniformBuffer(const uint32_t &currentImage, UniformBufferObject &ubo)
	{
		memcpy(uniformAllocations[currentImage]->GetMappedData(), &ubo, sizeof(ubo));
	}
};