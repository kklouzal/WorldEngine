#pragma once

class Tool_NPC : public Tool
{
public:
	Tool_NPC()
		: Tool()
	{
		Name = "NPC";
	}

	~Tool_NPC()
	{}

	void PrimaryAction(btCollisionWorld::ClosestRayResultCallback Ray)
	{
		/*if (Ray.hasHit()) {
			printf(" - Hit\n");

			_NavMesh->SetMoveTarget(Ray.m_hitPointWorld);

		}
		else {
			printf(" - Miss\n");
		}*/
	}

	void SecondaryAction(btCollisionWorld::ClosestRayResultCallback Ray)
	{
		/*if (Ray.hasHit()) {
			printf(" - Hit\n");

			_NavMesh->AddAgent(Ray.m_hitPointWorld);

		}
		else {
			printf(" - Miss\n");
		}*/
	}

	void DrawGUI()
	{
		ImGui::SetNextWindowSize(ImVec2(200, 70));
		ImGui::SetNextWindowPos(ImVec2(WorldEngine::VulkanDriver::WIDTH / 2 - 100, WorldEngine::VulkanDriver::HEIGHT - 150));
		ImGui::Begin("NPC Settings", 0, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::TextDisabled("NPC Tool");
		// TODO: MORE STUFF HERE...
		ImGui::End();
	}
};