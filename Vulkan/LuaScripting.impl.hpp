#include "Lua_Ent.hpp"

namespace WorldEngine
{
	namespace LUA
	{
		void Initialize()
		{
			//
			//	Paths

			printf("TopLevel Path: %s\n", TopLevel.generic_string().c_str());
			printf("MainLevel Path: %s\n", MainLevel.generic_string().c_str());
			printf("SEntLevel Path: %s\n", SEntLevel.generic_string().c_str());
			printf("SWepLevel Path: %s\n", SWepLevel.generic_string().c_str());

			state = luaL_newstate();
			luaL_openlibs(state);

			//
			//	Initialize bindings
			Ent::Initialize();
			//
			//	Load core scripted objects
			Ent::Load();

			//
			//	Load main lua file
			LoadFile((MainLevel / "init.lua").generic_string().c_str());

		}

		void Deinitialize()
		{
			lua_close(state);
		}
	}
}