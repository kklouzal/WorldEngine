#pragma once
//
#include "PipelineObject.hpp"
//
#include "Import_GLTF.hpp"
#include "TriangleMesh.hpp"
//
#include "Pipe_Shadow.hpp"
#include "Pipe_Static.hpp"
#include "Pipe_Animated.hpp"
#include "Pipe_Composition.hpp"
#include "Pipe_GUI.hpp"
#include "Pipe_CEF.hpp"

enum Pipelines {
	Shadow,
	Static,
	Animated,
	Composition,
	GUI,
	CEF
};

namespace WorldEngine
{
	namespace MaterialCache
	{
		VkPipelineCache pipelineCache = VK_NULL_HANDLE;
		std::vector<PipelineObject*> Pipes = {};
		//
		ImportGLTF* _ImportGLTF = nullptr;
		//

		//
		//	Create Shadow Pipe
		//	Handles creating shadow maps for scene nodes offscreen
		void CreateShadow()
		{
			printf("Create Shadow Pipe\n");
			Pipes.emplace_back(new Pipeline::Shadow(pipelineCache));
		}

		//
		//	Create Static Pipe
		//	Handles rendering static scene nodes offscreen
		void CreateStatic() {
			printf("Create Static Pipe\n");
			Pipes.emplace_back(new Pipeline::Static(pipelineCache));
		}

		//
		//	Create Animated Pipe
		//	Handles rendering animated scene nodes offscreen
		void CreateAnimated() {
			printf("Create Animated Pipe\n");
			Pipes.emplace_back(new Pipeline::Animated(pipelineCache));
		}

		//
		//	Create Composition Pipe
		//	Handles rendering final deferred composition output to the screen
		void CreateComposition()
		{
			printf("Create Composition Pipe\n");
			Pipes.emplace_back(new Pipeline::Composition(pipelineCache));
		}

		//
		//	Create GUI Pipe
		//	Handles drawing imGUI on top of our composition output
		void CreateGUI() {
			printf("Create GUI Pipe\n");
			Pipes.emplace_back(new Pipeline::GUI(pipelineCache));
		}

		//
		//	Create CEF Pipe
		//	Handles drawing CEF on top of our composition output
		void CreateCEF() {
			printf("Create CEF Pipe\n");
			Pipes.emplace_back(new Pipeline::CEF(pipelineCache));
		}

		//
		//	Pipes *MUST* be initialized in the order they appear in the Pipelines enum.
		void Initialize() {
			//
			_ImportGLTF = new ImportGLTF;
			//
			CreateShadow();
			CreateStatic();
			CreateAnimated();
			CreateComposition();
			CreateGUI();
			CreateCEF();
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
			//
			delete _ImportGLTF;
		}

		Pipeline::Shadow* GetPipe_Shadow() {
			return static_cast<Pipeline::Shadow*>(Pipes[Pipelines::Shadow]);
		}
		Pipeline::Static* GetPipe_Static() {
			return static_cast<Pipeline::Static*>(Pipes[Pipelines::Static]);
		}
		Pipeline::Animated* GetPipe_Animated() {
			return static_cast<Pipeline::Animated*>(Pipes[Pipelines::Animated]);
		}
		Pipeline::Composition* GetPipe_Composition() {
			return static_cast<Pipeline::Composition*>(Pipes[Pipelines::Composition]);
		}
		Pipeline::GUI* GetPipe_GUI() {
			return static_cast<Pipeline::GUI*>(Pipes[Pipelines::GUI]);
		}
		Pipeline::CEF* GetPipe_CEF() {
			return static_cast<Pipeline::CEF*>(Pipes[Pipelines::CEF]);
		}
	}
}