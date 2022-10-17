#pragma once

#include "Lua_GameMode.hpp"
#include "Lua_Player.hpp"
#include "Lua_Ent.hpp"
#include "Lua_Item.hpp"
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
			wxLogMessage("TopLevel  Path: %s", TopLevel.generic_string().c_str());
			wxLogMessage("MainLevel Path: %s", MainLevel.generic_string().c_str());
			wxLogMessage("BaseLevel Path: %s", BaseLevel.generic_string().c_str());
			wxLogMessage("SGmLevel  Path: %s", SGmLevel.generic_string().c_str());
			wxLogMessage("SPlyLevel Path: %s", SPlyLevel.generic_string().c_str());
			wxLogMessage("SEntLevel Path: %s", SEntLevel.generic_string().c_str());
			wxLogMessage("SItmLevel Path: %s", SItmLevel.generic_string().c_str());

			state = luaL_newstate();
			luaL_openlibs(state);

			lua_pushcfunction(state, print);
			lua_setglobal(state, "print");


			//
			//	Initialize bindings
			GM::Initialize();
			Ply::Initialize();
			Ent::Initialize();
			Itm::Initialize();
			//
			//	Load core scripted objects
			GM::Load(WorldEngine::CurrentGamemode.c_str());
			Ply::Load();
			Ent::Load();
			Itm::Load();

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