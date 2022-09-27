#pragma once

class TriangleMesh {

public:
	std::vector<InstanceData> instanceData;
	PipelineObject* Pipe;

	GLTFInfo* _GLTF;
	size_t vertexBufferSize;
	size_t indexBufferSize;


	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VmaAllocation vertexAllocation = VMA_NULL;

	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VmaAllocation indexAllocation = VMA_NULL;

	std::vector <VkBuffer> instanceStorageSpaceBuffers = {};
	std::vector <VmaAllocation> instanceStorageSpaceAllocations = {};

	TextureObject* Texture_Albedo;
	TextureObject* Texture_Normal;
	DescriptorObject* Descriptor;

public:
	
	TriangleMesh(PipelineObject* Pipeline, GLTFInfo* GLTF, TextureObject* Albedo, TextureObject* Normal)
		: Pipe(Pipeline), _GLTF(GLTF), vertexBufferSize(sizeof(Vertex)* GLTF->Vertices.size()), indexBufferSize(sizeof(uint32_t)* GLTF->Indices.size()) {
		createVertexBuffer();
		//
		//	Start with a single instance, grow as needed.
		instanceData.resize(1);
		size_t SSBOSize = sizeof(InstanceData) * instanceData.size();
		createStorageBuffer(SSBOSize);
		Texture_Albedo = Albedo;
		Texture_Normal = Normal;
		Descriptor = Pipe->createDescriptor(Albedo, Normal, instanceStorageSpaceBuffers);
	}

	~TriangleMesh() {
		printf("Destroy TriangleMesh\n");
		//	Destroy VMA Buffers
		vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, vertexBuffer, vertexAllocation);
		vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, indexBuffer, indexAllocation);
		//	Destroy VMA Buffers
		for (size_t i = 0; i < instanceStorageSpaceBuffers.size(); i++) {
			vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, instanceStorageSpaceBuffers[i], instanceStorageSpaceAllocations[i]);
		}
		delete Descriptor;
	}

	void createStorageBuffer(size_t SSBO_Size)
	{
		instanceStorageSpaceBuffers.resize(WorldEngine::VulkanDriver::swapChain.images.size());
		instanceStorageSpaceAllocations.resize(WorldEngine::VulkanDriver::swapChain.images.size());

		for (size_t i = 0; i < WorldEngine::VulkanDriver::swapChain.images.size(); i++) {
			VkBufferCreateInfo ssboBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			ssboBufferInfo.size = SSBO_Size;
			ssboBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			ssboBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo ssboAllocInfo = {};
			ssboAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			ssboAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &ssboBufferInfo, &ssboAllocInfo, &instanceStorageSpaceBuffers[i], &instanceStorageSpaceAllocations[i], nullptr);
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
		vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &vertexBufferInfo, &vertexAllocInfo, &stagingVertexBuffer, &stagingVertexBufferAlloc, nullptr);

		memcpy(stagingVertexBufferAlloc->GetMappedData(), _GLTF->Vertices.data(), vertexBufferSize);

		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vertexAllocInfo.flags = 0;
		vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &vertexBufferInfo, &vertexAllocInfo, &vertexBuffer, &vertexAllocation, nullptr);

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
		vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &indexBufferInfo, &indexAllocInfo, &stagingIndexBuffer, &stagingIndexBufferAlloc, nullptr);

		memcpy(stagingIndexBufferAlloc->GetMappedData(), _GLTF->Indices.data(), indexBufferSize);

		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		indexAllocInfo.flags = 0;
		vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &indexBufferInfo, &indexAllocInfo, &indexBuffer, &indexAllocation, nullptr);

		//
		//	CPU->GPU Copy
		VkCommandBuffer commandBuffer = WorldEngine::VulkanDriver::beginSingleTimeCommands();

		VkBufferCopy vertexCopyRegion = {};
		vertexCopyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(commandBuffer, stagingVertexBuffer, vertexBuffer, 1, &vertexCopyRegion);

		VkBufferCopy indexCopyRegion = {};
		indexCopyRegion.size = indexBufferInfo.size;
		vkCmdCopyBuffer(commandBuffer, stagingIndexBuffer, indexBuffer, 1, &indexCopyRegion);

		WorldEngine::VulkanDriver::endSingleTimeCommands(commandBuffer);

		//
		//	Destroy Staging Buffers
		vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, stagingVertexBuffer, stagingVertexBufferAlloc);
		vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, stagingIndexBuffer, stagingIndexBufferAlloc);
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

	//
	//	TODO: Do away with this? memcpy only changed regions of the buffer when editing instanceData vector.
	void updateSSBuffer(const uint32_t& currentImage)
	{
		memcpy(instanceStorageSpaceAllocations[currentImage]->GetMappedData(), instanceData.data(), sizeof(InstanceData) * instanceData.size());
	}
};