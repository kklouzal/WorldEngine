#pragma once
#include <stdexcept>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <filesystem>

/**
 * C++ exception class wrapper for Lua error.
 * This can be used to convert the result of a lua_pcall or

 * similar protected Lua C function into a C++ exception.
 * These Lua C functions place the error on the Lua stack.
 * The LuaError class maintains the error on the Lua stack until
 * all copies of the exception are destroyed (after the exception is
 * caught), at which time the Lua error object is popped from the
 * Lua stack.
 * We assume the Lua stack is identical at destruction as
 * it was at construction.
 */

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


class ScriptedEntityFactory {
	//LuaTableObject* LTable;

public:
	ScriptedEntityFactory(std::string ClassName)
	{
		//	Create new table entry in lua global entities table
		//	This table will be used as a metamethod attached to objects created of this type
	}

	void Create()
	{
		//	Called from within lua?
		//	Returns a new scripted entity object of this type
		//	With the appropriate metamethod table attached
	}
};

namespace ScriptedEntityManager {
	namespace {
		std::unordered_map<std::string, ScriptedEntityFactory*> Entities;
	}

	void ScanForEntities(std::string Path)
	{
		//	Loop through all subfolders inside of Path
		//	If a subfolder contains a init.lua file
		//	Create a new ScriptedEntityFactory
		//	Use functions defined inside this init.lua file as the basis for a metamethod table
	}

	void Register(std::string ClassName)
	{
		Entities[ClassName] = new ScriptedEntityFactory(ClassName);
	}

	//
	//	Called from Lua, [ local NewEnt = entities.create("SomeClassName") ]
	void CreateEntity(std::string ClassName)
	{
		Entities[ClassName]->Create();
	}
}

namespace WorldEngine
{
	namespace LUA
	{
		namespace
		{
			lua_State* state;
			std::filesystem::path TopLevel = std::filesystem::current_path() /= "Lua";
			std::filesystem::path MainLevel = TopLevel / "main";
			std::filesystem::path SEntLevel = TopLevel / "s_ent";
			std::filesystem::path SWepLevel = TopLevel / "s_wep";
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
				printf("Lua Error: %s\n", e.what());
			}
		}
	}
}