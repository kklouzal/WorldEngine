#pragma once
#include "VulkanGWEN.hpp"

namespace WorldEngine
{
	namespace GUI
	{
		Gwen::Renderer::Vulkan* pRenderer;
		Gwen::Skin::TexturedBase* pSkin;
		Gwen::Controls::Canvas* pCanvas;

		void Initialize()
		{
			pRenderer = new Gwen::Renderer::Vulkan();

			pRenderer->Init();
			pSkin = new Gwen::Skin::TexturedBase(pRenderer);
			pSkin->Init("media/DefaultSkin.png");

			pCanvas = new Gwen::Controls::Canvas(pSkin);
			pCanvas->SetSize(WorldEngine::VulkanDriver::WIDTH, WorldEngine::VulkanDriver::HEIGHT);
			pCanvas->SetDrawBackground(false);
			pCanvas->SetBackgroundColor(Gwen::Color(150, 170, 170, 255));
			pCanvas->SetKeyboardInputEnabled(false);
		}

		void Deinitialize()
		{
			delete pCanvas;
			delete pSkin;
			delete pRenderer;
		}

		void Draw(const VkCommandBuffer& Buff)
		{
			pRenderer->SetBuffer(Buff);
			pCanvas->RenderCanvas();
		}
	}
}