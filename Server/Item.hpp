#pragma once

namespace WorldEngine
{
	class Item : public SceneNode
	{
		const char*const Classname;
	public:

		Item(const char*const Classname)
			: SceneNode(), Classname(Classname)
		{
			WorldEngine::SceneGraph::AddSceneNode(this);
			wxLogMessage("[Item] Created!");
		}

		~Item()
		{
			wxLogMessage("[Item] Deleted!");
		}

		const char* const GetClassname() const
		{
			return Classname;
		}
	};
}