#pragma once
#include <stdexcept>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

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



