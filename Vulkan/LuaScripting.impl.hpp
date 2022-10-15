#include "Lua_GameMode.hpp"
#include "Lua_Ent.hpp"
#include "Lua_Item.hpp"

namespace WorldEngine
{
	namespace LUA
	{

		int print(lua_State* L)
		{
			switch (lua_type(L, -1)) {
			case LUA_TNUMBER:
				printf("%g\n", lua_tonumber(L, -1));
				break;
			case LUA_TSTRING:
				printf("%s\n", lua_tostring(L, -1));
				break;
			case LUA_TBOOLEAN:
				printf("%s\n", (lua_toboolean(L, -1) ? "true" : "false"));
				break;
			case LUA_TNIL:
				printf("%s\n", "nil");
				break;
			default:
				printf("%p\n", lua_topointer(L, -1));
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
			printf("TopLevel  Path: %s\n", TopLevel.generic_string().c_str());
			printf("MainLevel Path: %s\n", MainLevel.generic_string().c_str());
			printf("BaseLevel Path: %s\n", BaseLevel.generic_string().c_str());
			printf("SGmLevel  Path: %s\n", SGmLevel.generic_string().c_str());
			printf("SEntLevel Path: %s\n", SEntLevel.generic_string().c_str());
			printf("SItmLevel Path: %s\n", SItmLevel.generic_string().c_str());

			state = luaL_newstate();
			luaL_openlibs(state);

			lua_pushcfunction(state, print);
			lua_setglobal(state, "print");

			//
			//	Initialize bindings
			GM::Initialize();
			Ent::Initialize();
			Itm::Initialize();
			//
			//	Load core scripted objects
			GM::Load("test_gamemode");
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