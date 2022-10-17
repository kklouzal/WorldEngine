﻿#pragma once

#include <stdexcept>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <filesystem>

static void dumpstack(lua_State* L) {
	int top = lua_gettop(L);
	for (int i = 1; i <= top; i++) {
		switch (lua_type(L, i)) {
		case LUA_TNUMBER:
			wxLogMessage("[%d] %s\t%g", i, luaL_typename(L, i), lua_tonumber(L, i));
			break;
		case LUA_TSTRING:
			wxLogMessage("[%d] %s\t%s", i, luaL_typename(L, i), lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
			wxLogMessage("[%d] %s\t%s", i, luaL_typename(L, i), (lua_toboolean(L, i) ? "true" : "false"));
			break;
		case LUA_TNIL:
			wxLogMessage("[%d] %s\t%s", i, luaL_typename(L, i), "nil");
			break;
		default:
			wxLogMessage("[%d] %s\t%p", i, luaL_typename(L, i), lua_topointer(L, i));
			break;
		}
	}
}

static void LuaError_lua_resource_delete(lua_State* L) {
	lua_pop(L, 1);
}

class LuaError : public std::exception {
private:
	lua_State* m_L;
	// resource for error object on Lua stack (is to be popped
	// when no longer used)
	std::shared_ptr<lua_State> m_lua_resource;
	LuaError& operator=(const LuaError& other) {}; // prevent
public:
	// Construct using top-most element on Lua stack as error.
	LuaError(lua_State* L) : m_L(L), m_lua_resource(L, LuaError_lua_resource_delete) {}

	LuaError(const LuaError& other) : m_L(other.m_L), m_lua_resource(other.m_lua_resource) {}

	~LuaError() {}

	const char* what() const throw() {
		const char* s = lua_tostring(m_L, -1);
		if (s == NULL) s = "unrecognized Lua error";
		return s;
	}
};

namespace WorldEngine
{
	class Item;

	namespace LUA
	{
		namespace
		{
			lua_State* state;
			std::filesystem::path TopLevel  = std::filesystem::current_path() /= "Lua";
			std::filesystem::path MainLevel = TopLevel / "main";
			std::filesystem::path BaseLevel = TopLevel / "base";
			std::filesystem::path SGmLevel  = TopLevel / "s_gm";
			std::filesystem::path SPlyLevel = TopLevel / "s_ply";
			std::filesystem::path SEntLevel = TopLevel / "s_ent";
			std::filesystem::path SItmLevel = TopLevel / "s_itm";
		}

		void LoadFile(const char* File)
		{
			try {
				int res = luaL_loadfile(state, File);
				if (res != LUA_OK) throw LuaError(state);

				res = lua_pcall(state, 0, LUA_MULTRET, 0);
				if (res != LUA_OK) throw LuaError(state);
			}
			catch (std::exception& e) {
				wxLogMessage("Lua Error(%s): %s\n", File, e.what());
			}
		}

		//
		//	Forward Declarations
		namespace GM
		{
			void Call_OnPlayerSpawn(uintmax_t ObjectID);
		}
		namespace Ply
		{
			void Create(Player* NewPlayer, const char* Classname);
			void PushPlayer(uintmax_t ObjectID);
		}
		namespace Itm
		{
			//
			//	Keep track of all our Operation IDs
			//	This list should match the client side exactly.
			enum class OPID : uint8_t {
				Give
			};

			WorldEngine::Item* Create(const char* Classname);
		}
	}
}