#pragma once
#include <vector>
#include <array>

#include <chrono>

class TriangleMesh {

public:

	VulkanDriver* _Driver = VK_NULL_HANDLE;
	PipelineObject* Pipe;

	const std::vector<Vertex> vertices;
	const std::vector<uint32_t> indices;
	size_t vertexBufferSize;
	size_t indexBufferSize;


	VkBuffer vertexBuffer = VK_NULL_HANDLE;
	VmaAllocation vertexAllocation = VMA_NULL;

	VkBuffer indexBuffer = VK_NULL_HANDLE;
	VmaAllocation indexAllocation = VMA_NULL;

	std::vector<VkCommandBuffer> commandBuffers = {};

	std::vector<VkBuffer> uniformBuffers = {};
	std::vector<VmaAllocation> uniformAllocations = {};

	TextureObject* Texture;
	DescriptorObject* Descriptor;

public:
	
	TriangleMesh(VulkanDriver* Driver, PipelineObject* Pipeline, const std::vector<Vertex> &Vertices, const std::vector<uint32_t> &Indices, TextureObject* Diffuse)
		: _Driver(Driver), Pipe(Pipeline), vertices(Vertices), indices(Indices),
		vertexBufferSize(sizeof(vertices[0])* vertices.size()), indexBufferSize(sizeof(uint32_t)* indices.size()) {
		createVertexBuffer();
		createUniformBuffers();
		Texture = Diffuse;
		Descriptor = Pipe->createDescriptor(Texture, uniformBuffers);

		draw();
	}

	~TriangleMesh() {
		//	Destroy VMA Buffers
		vmaDestroyBuffer(_Driver->allocator, vertexBuffer, vertexAllocation);
		vmaDestroyBuffer(_Driver->allocator, indexBuffer, indexAllocation);
		//	Destroy VMA Buffers
		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			vmaDestroyBuffer(_Driver->allocator, uniformBuffers[i], uniformAllocations[i]);
		}
		delete Descriptor;
	}


	void createUniformBuffers() {
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		uniformBuffers.resize(_Driver->swapChainImages.size());
		uniformAllocations.resize(_Driver->swapChainImages.size());

		for (size_t i = 0; i < _Driver->swapChainImages.size(); i++) {

			VkBufferCreateInfo uniformBufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
			uniformBufferInfo.size = bufferSize;
			uniformBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			uniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo uniformAllocInfo = {};
			uniformAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
			uniformAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

			VmaAllocationInfo uniformBufferAllocInfo = {};

			vmaCreateBuffer(_Driver->allocator, &uniformBufferInfo, &uniformAllocInfo, &uniformBuffers[i], &uniformAllocations[i], &uniformBufferAllocInfo);
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

		memcpy(stagingVertexBufferAlloc->GetMappedData(), vertices.data(), vertexBufferSize);

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

		memcpy(stagingIndexBufferAlloc->GetMappedData(), indices.data(), indexBufferSize);

		indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		indexAllocInfo.flags = 0;
		vmaCreateBuffer(_Driver->allocator, &indexBufferInfo, &indexAllocInfo, &indexBuffer, &indexAllocation, nullptr);

		//
		//	CPU->GPU Copy
		VkCommandBuffer commandBuffer = _Driver->_SceneGraph->beginSingleTimeCommands();

		VkBufferCopy vertexCopyRegion = {};
		vertexCopyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(commandBuffer, stagingVertexBuffer, vertexBuffer, 1, &vertexCopyRegion);

		VkBufferCopy indexCopyRegion = {};
		indexCopyRegion.size = indexBufferInfo.size;
		vkCmdCopyBuffer(commandBuffer, stagingIndexBuffer, indexBuffer, 1, &indexCopyRegion);

		_Driver->_SceneGraph->endSingleTimeCommands(commandBuffer);

		//
		//	Destroy Staging Buffers
		vmaDestroyBuffer(_Driver->allocator, stagingVertexBuffer, stagingVertexBufferAlloc);
		vmaDestroyBuffer(_Driver->allocator, stagingIndexBuffer, stagingIndexBufferAlloc);
	}

	void draw() {
#ifdef _DEBUG
		std::cout << "TriangleMesh Draw" << std::endl;
#endif
		//	Command buffers are returned in the recording state
		commandBuffers = _Driver->_SceneGraph->newCommandBuffer();
		for (size_t i = 0; i < commandBuffers.size(); i++) {

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, Pipe->graphicsPipeline);
			//	Bind Descriptor Sets
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, Pipe->pipelineLayout, 0, 1, &Descriptor->DescriptorSets[i], 0, nullptr);
			
			//	Draw Vertex Buffer
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &vertexBuffer, offsets);
			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
			//
			//	End recording
			vkEndCommandBuffer(commandBuffers[i]);
		}
	}

	void updateUniformBuffer(const uint32_t &currentImage, UniformBufferObject &ubo) {
		Camera Cam = _Driver->_SceneGraph->GetCamera();
		//Cam.SetPosition(glm::vec3(-48.0f, 0.0f, 48.0f));
		Cam.SetPosition(glm::vec3(-10.0f, -3.0f, 5.0f));
		Cam.SetAngle(glm::vec3(1.0f, 0.0f, -0.5f));
			//ubo.view = glm::lookAt(glm::vec3(512.0f, 512.0f, 128.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			ubo.view = glm::lookAt(Cam.Pos, Cam.Center, glm::vec3(0.0f, 1.0f, 0.0f));
			ubo.proj = glm::perspective(glm::radians(45.0f), _Driver->swapChainExtent.width / (float)_Driver->swapChainExtent.height, 0.1f, 1024.0f);
			ubo.proj[1][1] *= -1;

		memcpy(uniformAllocations[currentImage]->GetMappedData(), &ubo, sizeof(ubo));
	}

	void drawFrame(const VkCommandBuffer &primaryCommandBuffer) {
		//std::cout << "TriangleMesh DrawFrame" << std::endl;
		vkCmdExecuteCommands(primaryCommandBuffer, commandBuffers.size(), commandBuffers.data());
	}
};