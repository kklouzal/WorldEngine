#include "Lua_GameMode.hpp"
#include "Lua_Player.hpp"
#include "Lua_Ent.hpp"
#include <string>

namespace WorldEngine
{
	namespace LUA
	{
		//int print(lua_State* L)
		//{
		//	const char* Str = lua_tostring(L, -1);
		//	if (Str == NULL) {
		//		Str = "nil";
		//	}
		//	wxLogMessage(Str);
		//	//lua_settop(L, 0);
		//	return 0;
		//}

		int print(lua_State* L)
		{
			switch (lua_type(L, -1)) {
			case LUA_TNUMBER:
				wxLogMessage("%g", lua_tonumber(L, -1));
				break;
			case LUA_TSTRING:
				wxLogMessage("%s", lua_tostring(L, -1));
				break;
			case LUA_TBOOLEAN:
				wxLogMessage("%s", (lua_toboolean(L, -1) ? "true" : "false"));
				break;
			case LUA_TNIL:
				wxLogMessage("%s", "nil");
				break;
			default:
				wxLogMessage("%p", lua_topointer(L, -1));
				break;
			}
			//	Empty Stack
			lua_remove(L, lua_gettop(L));
			return 0;
		}

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

			lua_pushcfunction(state, print);
			lua_setglobal(state, "print");


			//
			//	Initialize bindings
			GM::Initialize();
			Ply::Initialize();
			Ent::Initialize();
			//
			//	Load core scripted objects
			GM::Load(WorldEngine::CurrentGamemode.c_str());
			Ply::Load();
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