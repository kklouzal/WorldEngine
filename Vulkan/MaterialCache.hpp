#pragma once

#include "Pipe_Default.hpp"
#include "Pipe_GUI.hpp"
#include "Pipe_Skinned.hpp"

enum Pipelines {
	Default,
	GUI,
	Skinned
};

class MaterialCache {

public:
	VulkanDriver* _Driver = VK_NULL_HANDLE;
	VkPipelineCache pipelineCache;
	std::vector<PipelineObject*> Pipes;
	//

	MaterialCache(VulkanDriver* Driver) : _Driver(Driver) {
		CreateDefault();
		CreateGUI();
		CreateSkinned();
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		if (vkCreatePipelineCache(Driver->_VulkanDevice->logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
		{
			#ifdef _DEBUG
			throw std::runtime_error("vkCreatePipelineCache Failed!");
			#endif
		}
	}

	~MaterialCache() {
		for (auto Pipe : Pipes) {
			delete Pipe;
		}
		vkDestroyPipelineCache(_Driver->_VulkanDevice->logicalDevice, pipelineCache, nullptr);
	}

	Pipeline::Default* GetPipe_Default() {
		return static_cast<Pipeline::Default*>(Pipes[Default]);
	}
	Pipeline::GUI* GetPipe_GUI() {
		return static_cast<Pipeline::GUI*>(Pipes[GUI]);
	}
	Pipeline::Skinned* GetPipe_Skinned() {
		return static_cast<Pipeline::Skinned*>(Pipes[Skinned]);
	}

	//
	//	Create Default Pipe
	void CreateDefault() {
		printf("Create Default Pipe\n");
		Pipes.emplace_back(new Pipeline::Default(_Driver));

	}

	//
	//	Create GUI Pipe
	void CreateGUI() {
		printf("Create GUI Pipe\n");
		Pipes.emplace_back(new Pipeline::GUI(_Driver));
	}

	//
	//	Create Skinned Pipe
	void CreateSkinned() {
		printf("Create Skinned Pipe\n");
		Pipes.emplace_back(new Pipeline::Skinned(_Driver));
	}
};