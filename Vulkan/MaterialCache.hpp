#pragma once

#include "Pipe_Default.hpp"
#include "Pipe_GUI.hpp"

enum Pipelines {
	Default,
	GUI
};

namespace WorldEngine
{
	namespace MaterialCache
	{
		VkPipelineCache pipelineCache;
		std::vector<PipelineObject*> Pipes;
		//

		//
		//	Create Default Pipe
		void CreateDefault() {
			printf("Create Default Pipe\n");
			Pipes.emplace_back(new Pipeline::Default(pipelineCache));

		}

		//
		//	Create GUI Pipe
		void CreateGUI() {
			printf("Create GUI Pipe\n");
			Pipes.emplace_back(new Pipeline::GUI(pipelineCache));
		}

		//
		void Initialize() {
			CreateDefault();
			CreateGUI();
			VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
			pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			if (vkCreatePipelineCache(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, &pipelineCacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
			{
#ifdef _DEBUG
				throw std::runtime_error("vkCreatePipelineCache Failed!");
#endif
			}
		}

		void Deinitialize() {
			for (auto Pipe : Pipes) {
				delete Pipe;
			}
			vkDestroyPipelineCache(WorldEngine::VulkanDriver::_VulkanDevice->logicalDevice, pipelineCache, nullptr);
		}

		Pipeline::Default* GetPipe_Default() {
			return static_cast<Pipeline::Default*>(Pipes[Default]);
		}
		Pipeline::GUI* GetPipe_GUI() {
			return static_cast<Pipeline::GUI*>(Pipes[GUI]);
		}
	}
}