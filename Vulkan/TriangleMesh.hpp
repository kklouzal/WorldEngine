#pragma once

class TriangleMesh {

public:
	bool bCastsShadows;
	bool bAnimated;
	bool bFirstInstance = true;
	std::vector<InstanceData> instanceData{};						//TODO: Make these pointers and give one to each owning scene node
	//std::vector<glm::mat4*> instanceData_Shadow{};
	std::vector<InstanceData_Animation> instanceData_Animation{};	//TODO: Make these pointers and give one to each owning scene node
	PipelineObject* Pipe;

	std::string Name{};

	const char* FileName;
	GLTFInfo* _GLTF;
	size_t vertexBufferSize;
	size_t indexBufferSize;

	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VmaAllocation vertexAllocation = VMA_NULL;

	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VmaAllocation indexAllocation = VMA_NULL;

	//	TODO: Use single VkBuffer. Apply offsets in descriptor create/update.
	std::vector <VkBuffer> instanceStorageSpaceBuffers = {};
	std::vector <VmaAllocation> instanceStorageSpaceAllocations = {};
	//
	//std::vector <VkBuffer> instanceStorageSpaceBuffers = {};
	//std::vector <VmaAllocation> instanceStorageSpaceAllocations = {};
	//
	//	Inverse Bind Poses (global to entire triangle mesh)
	VkBuffer IBP_Buffer = VK_NULL_HANDLE;
	VmaAllocation IBP_Alloc = VMA_NULL;

	TextureObject* Texture_Albedo;
	TextureObject* Texture_Normal;
	DescriptorObject* Descriptor;

public:
	
	TriangleMesh(PipelineObject* Pipeline, const char* FileName, GLTFInfo* GLTF, TextureObject* Albedo, TextureObject* Normal, bool bCastsShadows, bool bAnimated)
		: bCastsShadows(bCastsShadows), bAnimated(bAnimated),
		Pipe(Pipeline), FileName(FileName), _GLTF(GLTF),
		vertexBufferSize(sizeof(Vertex)* GLTF->Vertices.size()),
		indexBufferSize(sizeof(uint32_t)* GLTF->Indices.size())
	{
		createVertexBuffer();
		//
		//	Start with zero instances, grow as needed
		//	TODO: Make this actually start at 0
		instanceData.resize(1);
		if (bCastsShadows) {
			//instanceData_Shadow.resize(1);
		}
		if (bAnimated) {
			//
			//	Staged Inverse Bind Poses
			createBuffer_IBP();
			instanceData_Animation.resize(1);
		}
		size_t SSBOSize1 = sizeof(InstanceData) * instanceData.size();
		size_t SSBOSize2 = sizeof(InstanceData_Animation) * instanceData_Animation.size();
		createStorageBuffer(SSBOSize1, SSBOSize2);
		Texture_Albedo = Albedo;
		Texture_Normal = Normal;
		Descriptor = Pipe->createDescriptor(this);
	}

	~TriangleMesh() {
		printf("Destroy TriangleMesh %s\n\tSSBO Count %zu\n", Name.c_str(), instanceStorageSpaceBuffers.size());
		//	Destroy VMA Buffers
		vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, vertexBuffer, vertexAllocation);
		vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, indexBuffer, indexAllocation);
		//	Destroy VMA Buffers
		for (size_t i = 0; i < instanceStorageSpaceBuffers.size(); i++) {
			vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, instanceStorageSpaceBuffers[i], instanceStorageSpaceAllocations[i]);
		}
		if (bAnimated)
		{
			vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, IBP_Buffer, IBP_Alloc);
		}
		delete Descriptor;
	}

	//	TODO: This needs fixed to allow allocations to truly start at 0 instead of 1 and faking the 0 index.
	inline size_t RegisterInstanceIndex()
	{
		if (instanceData.size() == 1 && bFirstInstance)
		{
			bFirstInstance = false;
			//
			//	Invalidate our command buffers
			WorldEngine::MaterialCache::bRecordBuffers = true;
			return 0;
		}
		instanceData.push_back(InstanceData());
		if (bAnimated) {
			instanceData_Animation.push_back(InstanceData_Animation());
		}
		ResizeInstanceBuffer();
		if (bCastsShadows)
		{
			//instanceData_Shadow.resize(instanceData.size());
		}
		//
		//	Invalidate our command buffers
		WorldEngine::MaterialCache::bRecordBuffers = true;
		//
		return instanceData.size() - 1;
	}

	//
	//	TODO: This is bad.. Need to ensure any of the buffers aren't currently in use.
	inline void ResizeInstanceBuffer()
	{
		//
		//	Delete
		for (size_t i = 0; i < instanceStorageSpaceBuffers.size(); i++) {
			vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, instanceStorageSpaceBuffers[i], instanceStorageSpaceAllocations[i]);
		}
		//
		//	Recreate
		size_t SSBOSize1 = sizeof(InstanceData) * instanceData.size();
		size_t SSBOSize2 = sizeof(InstanceData_Animation) * instanceData_Animation.size();
		createStorageBuffer(SSBOSize1, SSBOSize2);
		Pipe->updateDescriptor(Descriptor, this);

		size_t SwapChainSize = WorldEngine::VulkanDriver::swapChain.images.size();
		for (size_t i = 0; i < SwapChainSize; i++) {
			memcpy(instanceStorageSpaceAllocations[i]->GetMappedData(), instanceData.data(), SSBOSize1);
			if (bAnimated) {
				memcpy(instanceStorageSpaceAllocations[i+SwapChainSize]->GetMappedData(), instanceData_Animation.data(), SSBOSize2);
			}
		}
	}

	inline void createStorageBuffer(const size_t& SSBO_Size1, const size_t& SSBO_Size2)
	{
		size_t modifier = 1;
		if (bAnimated) { modifier = 2; }

		size_t SwapChainSize = WorldEngine::VulkanDriver::swapChain.images.size();
		instanceStorageSpaceBuffers.resize(SwapChainSize * modifier);
		instanceStorageSpaceAllocations.resize(SwapChainSize * modifier);

		printf("SSBO Count %zu\n", SwapChainSize * modifier);

		for (size_t i = 0; i < SwapChainSize; i++) {
			VkBufferCreateInfo ssboBufferInfo1 = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			ssboBufferInfo1.size = SSBO_Size1;
			ssboBufferInfo1.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			ssboBufferInfo1.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo ssboAllocInfo = {};
			ssboAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			ssboAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &ssboBufferInfo1, &ssboAllocInfo, &instanceStorageSpaceBuffers[i], &instanceStorageSpaceAllocations[i], nullptr);

			if (bAnimated)
			{
				printf("SSBO 2 size %zu\n", SSBO_Size2);
				VkBufferCreateInfo ssboBufferInfo2 = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
				ssboBufferInfo2.size = SSBO_Size2;
				ssboBufferInfo2.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
				ssboBufferInfo2.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

				vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &ssboBufferInfo2, &ssboAllocInfo, &instanceStorageSpaceBuffers[i+SwapChainSize], &instanceStorageSpaceAllocations[i+SwapChainSize], nullptr);
			}
		}
	}
	
	void createBuffer_IBP() {
		//
		//	Vertex Buffer
		VkBufferCreateInfo BufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		BufferInfo.size = _GLTF->InverseBindMatrices.size() * sizeof(glm::mat4);
		BufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo AllocInfo = {};
		AllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		AllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VmaAllocation stagingBufferAlloc = VK_NULL_HANDLE;
		vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &BufferInfo, &AllocInfo, &stagingBuffer, &stagingBufferAlloc, nullptr);

		memcpy(stagingBufferAlloc->GetMappedData(), _GLTF->InverseBindMatrices.data(), _GLTF->InverseBindMatrices.size() * sizeof(glm::mat4));

		BufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		AllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		AllocInfo.flags = 0;
		vmaCreateBuffer(WorldEngine::VulkanDriver::allocator, &BufferInfo, &AllocInfo, &IBP_Buffer, &IBP_Alloc, nullptr);

		//
		//	CPU->GPU Copy
		VkCommandBuffer commandBuffer = WorldEngine::VulkanDriver::beginSingleTimeCommands();

		VkBufferCopy vertexCopyRegion = {};
		vertexCopyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(commandBuffer, stagingBuffer, IBP_Buffer, 1, &vertexCopyRegion);

		WorldEngine::VulkanDriver::endSingleTimeCommands(commandBuffer);

		//
		//	Destroy Staging Buffer
		vmaDestroyBuffer(WorldEngine::VulkanDriver::allocator, stagingBuffer, stagingBufferAlloc);
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

	inline void draw(const VkCommandBuffer& CmdBuffer, const uint32_t& CurFrame)
	{
		//	Bind Descriptor Sets
		vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Pipe->pipelineLayout, 0, 1, &Descriptor->DescriptorSets[CurFrame], 0, nullptr);
		//	Draw Vertex Buffer
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(CmdBuffer, 0, 1, &vertexBuffer, offsets);	//	Model
		vkCmdBindVertexBuffers(CmdBuffer, 1, 1, &instanceStorageSpaceBuffers[CurFrame], offsets);	//	Instance
		vkCmdBindIndexBuffer(CmdBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(CmdBuffer, static_cast<uint32_t>(_GLTF->Indices.size()), static_cast<uint32_t>(instanceData.size()), 0, 0, 0);
	}

	//
	//	TODO: Do away with this. Give each scene node their pointer to the data. Update directly.
	inline void updateSSBuffer(const uint32_t& currentImage)
	{
		memcpy(instanceStorageSpaceAllocations[currentImage]->GetMappedData(), instanceData.data(), sizeof(InstanceData) * instanceData.size());
		if (bAnimated) {
			const size_t SwapChainSize = WorldEngine::VulkanDriver::swapChain.images.size();
			memcpy(instanceStorageSpaceAllocations[currentImage+SwapChainSize]->GetMappedData(), instanceData_Animation.data(), sizeof(InstanceData_Animation) * instanceData_Animation.size());
		}
	}
};