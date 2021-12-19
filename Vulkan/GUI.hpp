#pragma once
#include "VulkanGWEN.hpp"

namespace WorldEngine
{
	namespace GUI
	{
		Gwen::Renderer::Vulkan* pRenderer;
		Gwen::Skin::TexturedBase* pSkin;
		Gwen::Controls::Canvas* pCanvas;

        unsigned int SelectedItem = 0;
        std::vector<Gwen::Controls::Label*> HotbarItems_LBL;
        std::vector<Gwen::Controls::ImagePanel*> HotbarItems_ICO;

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

            //
            // TODO: This is hacky, need to not depend on this
            //	Reserve 10 item slots (hotbar slots currently)
            for (int i = 0; i < 10; i++)
            {
                HotbarItems_LBL.push_back(nullptr);
                HotbarItems_ICO.push_back(nullptr);
            }
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

        //
        //  TODO: Move this into it's own menu class
        void CreateBottomBar()
        {
            Gwen::Controls::Base* BottomBar = new Gwen::Controls::Base(pCanvas);
            BottomBar->SetSize(550, 70);
            BottomBar->SetPos(VulkanDriver::WIDTH / 2 - 272, VulkanDriver::HEIGHT - 70);

            HotbarItems_ICO[0] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[0]->SetSize(50, 50);
            HotbarItems_ICO[0]->SetPos(0, 0);
            HotbarItems_ICO[0]->SetImage("media/empty.png");
            HotbarItems_LBL[0] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[0]->SetSize(40, 20);
            HotbarItems_LBL[0]->SetPos(5, 50);
            HotbarItems_LBL[0]->SetText("Selected");
            HotbarItems_LBL[0]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[1] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[1]->SetSize(50, 50);
            HotbarItems_ICO[1]->SetPos(55, 0);
            HotbarItems_ICO[1]->SetImage("media/empty.png");
            HotbarItems_LBL[1] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[1]->SetSize(40, 20);
            HotbarItems_LBL[1]->SetPos(60, 50);
            HotbarItems_LBL[1]->SetText("Slot 2");
            HotbarItems_LBL[1]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[2] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[2]->SetSize(50, 50);
            HotbarItems_ICO[2]->SetPos(110, 0);
            HotbarItems_ICO[2]->SetImage("media/empty.png");
            HotbarItems_LBL[2] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[2]->SetSize(40, 20);
            HotbarItems_LBL[2]->SetPos(115, 50);
            HotbarItems_LBL[2]->SetText("Slot 3");
            HotbarItems_LBL[2]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[3] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[3]->SetSize(50, 50);
            HotbarItems_ICO[3]->SetPos(165, 0);
            HotbarItems_ICO[3]->SetImage("media/empty.png");
            HotbarItems_LBL[3] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[3]->SetSize(40, 20);
            HotbarItems_LBL[3]->SetPos(170, 50);
            HotbarItems_LBL[3]->SetText("Slot 4");
            HotbarItems_LBL[3]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[4] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[4]->SetSize(50, 50);
            HotbarItems_ICO[4]->SetPos(220, 0);
            HotbarItems_ICO[4]->SetImage("media/empty.png");
            HotbarItems_LBL[4] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[4]->SetSize(40, 20);
            HotbarItems_LBL[4]->SetPos(225, 50);
            HotbarItems_LBL[4]->SetText("Slot 5");
            HotbarItems_LBL[4]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[5] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[5]->SetSize(50, 50);
            HotbarItems_ICO[5]->SetPos(275, 0);
            HotbarItems_ICO[5]->SetImage("media/empty.png");
            HotbarItems_LBL[5] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[5]->SetSize(40, 20);
            HotbarItems_LBL[5]->SetPos(280, 50);
            HotbarItems_LBL[5]->SetText("Slot 6");
            HotbarItems_LBL[5]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[6] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[6]->SetSize(50, 50);
            HotbarItems_ICO[6]->SetPos(330, 0);
            HotbarItems_ICO[6]->SetImage("media/empty.png");
            HotbarItems_LBL[6] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[6]->SetSize(40, 20);
            HotbarItems_LBL[6]->SetPos(335, 50);
            HotbarItems_LBL[6]->SetText("Slot 7");
            HotbarItems_LBL[6]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[7] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[7]->SetSize(50, 50);
            HotbarItems_ICO[7]->SetPos(385, 0);
            HotbarItems_ICO[7]->SetImage("media/empty.png");
            HotbarItems_LBL[7] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[7]->SetSize(40, 20);
            HotbarItems_LBL[7]->SetPos(390, 50);
            HotbarItems_LBL[7]->SetText("Slot 8");
            HotbarItems_LBL[7]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[8] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[8]->SetSize(50, 50);
            HotbarItems_ICO[8]->SetPos(440, 0);
            HotbarItems_ICO[8]->SetImage("media/empty.png");
            HotbarItems_LBL[8] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[8]->SetSize(40, 20);
            HotbarItems_LBL[8]->SetPos(445, 50);
            HotbarItems_LBL[8]->SetText("Slot 9");
            HotbarItems_LBL[8]->SetTextColor(Gwen::Color(200, 200, 55, 255));

            HotbarItems_ICO[9] = new Gwen::Controls::ImagePanel(BottomBar);
            HotbarItems_ICO[9]->SetSize(50, 50);
            HotbarItems_ICO[9]->SetPos(495, 0);
            HotbarItems_ICO[9]->SetImage("media/empty.png");
            HotbarItems_LBL[9] = new Gwen::Controls::Label(BottomBar);
            HotbarItems_LBL[9]->SetSize(40, 20);
            HotbarItems_LBL[9]->SetPos(500, 50);
            HotbarItems_LBL[9]->SetText("Slot 10");
            HotbarItems_LBL[9]->SetTextColor(Gwen::Color(200, 200, 55, 255));
        }

        void ChangeItemIcon(unsigned int CurItem, const char* ImageFile)
        {
            if (HotbarItems_ICO[CurItem] != nullptr) {
                HotbarItems_ICO[CurItem]->SetImage(ImageFile);
            }
        }

        void ChangeItemSelection(unsigned int CurItem, const char* ImageFile)
        {
            if (HotbarItems_LBL[SelectedItem] != nullptr) {
                if (HotbarItems_LBL[CurItem] != nullptr) {
                    std::string Txt("Slot ");
                    Txt += std::to_string(SelectedItem + 1);
                    HotbarItems_LBL[SelectedItem]->SetText(Txt);
                    HotbarItems_LBL[CurItem]->SetText("Active");
                    ChangeItemIcon(CurItem, ImageFile);
                    SelectedItem = CurItem;
                }
            }
        }
	}
}