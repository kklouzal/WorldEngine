#pragma once

namespace WorldEngine
{
	namespace LUA
	{
		namespace GM
		{
			namespace
			{

			}

			//
			//	Registers Base Class metatable into global object table
			//	Sets the base metatable __index to itself
			//	And adds all cFunctions to the metatable from c++
			int RegisterBase(lua_State* L)
			{
				printf("[Lua:Register Base] GM\n");
				if (lua_istable(L, -1))
				{
					//	-1 (1)	| in_table
					//
					lua_pushstring(L, "__index");
					//	-2 (1)	| in_table
					//	-1 (2)	| '__index'
					//
					lua_pushvalue(L, -2);
					//	-3 (1)	| in_table
					//	-2 (2)	| '__index'
					//	-1 (3)	| in_table copy
					//
					lua_settable(L, -3);
					//	-1 (1)	| in_table
					//
					lua_getglobal(state, "GM");
					//	-2 (1)	| in_table
					//	-1 (2)	| Main table
					//
					lua_pushstring(state, "BaseMetatable");
					//	-3 (1)	| in_table
					//	-2 (2)	| Main table
					//	-1 (3)	| 'BaseMetatable'
					//
					lua_insert(state, 1);
					//	-3 (1)	| 'BaseMetatable'
					//	-2 (2)	| table_in
					//	-1 (3)	| Main table
					//
					lua_insert(state, 1);
					//	-3 (1)	| Main table
					//	-2 (2)	| 'BaseMetatable'
					//	-1 (3)	| table_in
					//
					lua_settable(state, -3);
					//	-1 (1)	| Main table
					//
					lua_remove(state, -1);
					//	Stack Empty
				}
				return 0;
			}

			//
			//	Registers derived metatable and applies base class metatable
			int Register(lua_State* L)
			{
				printf("[Lua:Register] GM\n");
				if (lua_istable(L, -1))
				{
					//	-1 (1)	| in_table
					//
					lua_getglobal(L, "GM");
					//	-2 (1)	| in_table
					//	-1 (2)	| GM table
					//
					lua_pushstring(L, "BaseMetatable");
					//	-3 (1)	| in_table
					//	-2 (2)	| GM table
					//	-1 (3)	| 'BaseMetatable'
					//
					lua_gettable(L, -2);
					//	-3 (1)	| in_table
					//	-2 (2)	| GM table
					//	-1 (3)	| BaseMetatable table
					//
					lua_setmetatable(L, -3);
					//	-2 (1)	| in_table
					//	-1 (2)	| GM table
					//
					lua_pushstring(L, "DerivedMetatable");
					//	-3 (1)	| in_table
					//	-2 (2)	| GM table
					//	-1 (3)	| 'DerivedMetatable'
					//
					lua_insert(L, 1);
					//	-3 (1)	| 'DerivedMetatable'
					//	-2 (2)	| in_table
					//	-1 (3)	| GM table
					//
					lua_insert(L, 1);
					//	-3 (1)	| GM table
					//	-2 (2)	| 'DerivedMetatable'
					//	-1 (3)	| in_table
					//
					lua_settable(L, -3);
					//	-1 (1)	| GM table
					//
					lua_settop(L, 0);
					//	Stack Empty
				}
				return 0;
			}

			//
			//	Creates object into world
			int Create(lua_State* L)
			{
				printf("[Lua:Create] GM\n");
				return 1;
			}

			void Load(const char* Gamemode)
			{
				printf("[Lua:Load] GM Base\n");
				std::filesystem::path BasePath = BaseLevel / "base_gm";
				if (std::filesystem::exists(BasePath) && std::filesystem::is_directory(BasePath))
				{
					std::filesystem::path InitLua = BasePath / "init.lua";
					if (std::filesystem::exists(InitLua))
					{
						printf("\tinit.lua\n");
						LoadFile(InitLua.generic_string().c_str());
					}
				}
				printf("[Lua:Load] GM: %s\n", Gamemode);
				std::filesystem::path DerivedPath = SGmLevel / Gamemode;
				if (std::filesystem::exists(DerivedPath) && std::filesystem::is_directory(DerivedPath))
				{
					std::filesystem::path InitLua = DerivedPath / "init.lua";
					if (std::filesystem::exists(InitLua))
					{
						printf("\tinit.lua\n");
						LoadFile(InitLua.generic_string().c_str());
					}
				}
			}

			void Initialize()
			{
				//
				//	GM Table
				lua_newtable(state);						//	-3 | Table
				//
				//	GM.BaseMetatable table
				//	Base gamemode metatable
				lua_pushstring(state, "BaseMetatable");		//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	GM.DerivedMetatable table
				//	Derived gamemode metatable
				lua_pushstring(state, "DerivedMetatable");	//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	GM.Objects table
				//	All created objects
				lua_pushstring(state, "Objects");			//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	GM.RegisterBase function
				//	Register/overwrite base metatable
				lua_pushstring(state, "RegisterBase");		//	-2 | key
				lua_pushcfunction(state, RegisterBase);		//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	GM.Register function
				//	Register/overwrite derived metatable
				lua_pushstring(state, "Register");			//	-2 | key
				lua_pushcfunction(state, Register);			//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	GM.Create function
				//	Create gamemode object
				lua_pushstring(state, "Create");			//	-2 | key
				lua_pushcfunction(state, Create);			//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)

				//
				//	Finish table creation
				lua_setglobal(state, "GM");					//	Stack Empty
				//
				//	[Result]
				//	GM.BaseMetatable = {}					|	Single Table
				//	GM.DerivedMetatable = {}				|	Single tables
				//	GM.Object = {}							|	LightUserdata
				//	GM.RegisterBase(table)					|	CFunction - table metatable
				//	GM.Register(table)						|	CFunction - table metatable
				//	local NewGM = GM.Create()				|	CFunction - returns new LightUserdata object
			}

			//	Push the current gamemode object onto the stack
			void PushCurrentGamemode(lua_State* L)
			{
				lua_getglobal(L, "GM");
				//	-1 | GM table
				//
				lua_pushstring(L, "DerivedMetatable");
				//	-2 | GM table
				//	-1 | 'DerivedMetatable'
				//
				lua_gettable(L, -2);
				//	-2 | GM table
				//	-1 | DerivedMetatable table
				//
				lua_remove(L, lua_gettop(L) - 1);
				//	-1 | DerivedMetatable table
				//
			}

			//	Calls OnPlayerSpawn gamemode function
			//	Clears the stack.
			void Call_OnPlayerSpawn(uintmax_t ObjectID)
			{
				PushCurrentGamemode(state);
				//	-1 (1)	|	Gamemode Table
				//
				lua_getfield(state, -1, "OnPlayerSpawn");
				//	-2 (1)	|	Gamemode Table
				//	-1 (2)	|	OnPlayerSpawn function (or nil)
				//
				lua_remove(state, lua_gettop(state) - 1);
				//	-1 (1)	|	OnPlayerSpawn function (or nil)
				//
				if (lua_isfunction(state, -1))
				{
					lua_pushinteger(state, ObjectID);
					//	-2 (1)	|	OnPlayerSpawn function
					//	-1 (2)	|	ObjectID integer
					//
					Ply::PushPlayer(ObjectID);
					//	-3 (1)	|	OnPlayerSpawn function
					//	-2 (2)	|	ObjectID integer
					//	-1 (3)	|	Player table (or nil)
					//
					if (lua_istable(state, -1))
					{
						try {
							int res = lua_pcall(state, 2, 1, 0);
							if (res != LUA_OK) throw LuaError(state);
						}
						catch (std::exception& e) {
							wxLogMessage("Lua Error: %s\n", e.what());
						}
						//	Stack Empty
						//
					}
					else {
						wxLogMessage("[LUA] GM:OnPlayerSpawn() failed; Player does not exist.");
						lua_settop(state, 0);
						//	Stack Empty
					}
				}
				else {
					wxLogMessage("[LUA] GM:OnPlayerSpawn() failed; Function does not exist.");
					lua_remove(state, lua_gettop(state));
					//	Stack Empty
				}
			}

			void Deinitialize()
			{

			}
		}
	}
}