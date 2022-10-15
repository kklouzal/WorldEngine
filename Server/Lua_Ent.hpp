#pragma once

namespace WorldEngine
{
	namespace LUA
	{
		namespace Ent
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
				printf("[Lua:Register Base] Ents\n");
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
					lua_getglobal(state, "Ents");
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
			//	Registers scripted object and applies base class metatable
			int Register(lua_State* L)
			{
				printf("[Lua:Register] Entity\n");
				if (lua_isstring(L, -2))
				{
					if (lua_istable(L, -1))
					{
						//	-2 (1)	| in_string
						//	-1 (2)	| in_table
						//
						lua_pushstring(L, "__index");
						//	-3 (1)	| in_string
						//	-2 (2)	| in_table
						//	-1 (3)	| '__index'
						//
						lua_pushvalue(L, -2);
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| '__index'
						//	-1 (4)	| in_table copy
						//
						lua_settable(L, -3);
						//	-2 (1)	| in_string
						//	-1 (2)	| in_table
						//
						lua_getglobal(L, "Ents");
						//	-3 (1)	| in_string
						//	-2 (2)	| in_table
						//	-1 (3)	| Ents table
						//
						lua_pushstring(L, "BaseMetatable");
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ents table
						//	-1 (4)	| 'BaseMetatable'
						//
						lua_gettable(L, -2);
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ents table
						//	-1 (4)	| BaseMetatable table
						//
						lua_setmetatable(L, -3);
						//	-3 (1)	| in_string
						//	-2 (2)	| in_table
						//	-1 (3)	| Ents table
						//
						lua_pushstring(L, "ObjectMetatables");
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ents table
						//	-1 (4)	| 'ObjectMetatables'
						//
						lua_gettable(L, -2);
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ents table
						//	-1 (4)	| ObjectMetatables table
						//
						lua_insert(L, 1);
						//	-4 (1)	| ObjectMetatables table
						//	-3 (2)	| in_string
						//	-2 (3)	| in_table
						//	-1 (4)	| Ents table
						//
						lua_insert(L, 1);
						//	-4 (1)	| Ents table
						//	-3 (2)	| ObjectMetatables table
						//	-2 (3)	| in_string
						//	-1 (4)	| in_table
						//
						lua_settable(L, -3);
						//	-2 (1)	| Ents table
						//	-1 (2)	| ObjectMetatables table
						//
						lua_settop(L, 0);
						//	Stack Empty
					}
				}
				return 0;
			}

			//
			//	Creates object into world
			int Create(lua_State* L)
			{
				printf("[Lua:Create] Entity\n");
				return 1;
			}

			void Load()
			{
				printf("[Lua:Load] Entity Base\n");
				std::filesystem::path BasePath = BaseLevel / "base_ent";
				if (std::filesystem::exists(BasePath) && std::filesystem::is_directory(BasePath))
				{
					std::filesystem::path InitLua = BasePath / "init.lua";
					if (std::filesystem::exists(InitLua))
					{
						printf("\tinit.lua\n");
						LoadFile(InitLua.generic_string().c_str());
					}

				}
				printf("[Lua:Load] Entities\n");
				for (auto const& sub_dir : std::filesystem::directory_iterator{ SEntLevel })
				{
					if (sub_dir.is_directory())
					{
						std::string Path = sub_dir.path().generic_string();
						size_t LastSlash = Path.rfind('/') + 1;	//	TODO: Check '/' not portable?
						std::string Folder = Path.substr(LastSlash, Path.size() - LastSlash);
						//
						std::filesystem::path InitLua = sub_dir.path() / "init.lua";
						if (std::filesystem::exists(InitLua))
						{
							printf("\t%s -> init.lua\n", Folder.c_str());
							LoadFile(InitLua.generic_string().c_str());
						}
					}
				}
			}

			void Initialize()
			{
				//
				//	Ents Table
				lua_newtable(state);						//	-3 | Table
				//
				//	Ents.BaseMetatable table
				//	Base metatable for all object metatables
				lua_pushstring(state, "BaseMetatable");		//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Ents.ObjectMetatables table
				//	Unique object type metatables
				lua_pushstring(state, "ObjectMetatables");	//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Ents.Objects table
				//	All created objects
				lua_pushstring(state, "Objects");			//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Ents.RegisterBase function
				//	Register/overwrite base metatable
				lua_pushstring(state, "RegisterBase");		//	-2 | key
				lua_pushcfunction(state, RegisterBase);		//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Ents.Register function
				//	Register/overwrite unique object metatable
				lua_pushstring(state, "Register");			//	-2 | key
				lua_pushcfunction(state, Register);			//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Ents.Create function
				//	Create an object of unique type
				lua_pushstring(state, "Create");			//	-2 | key
				lua_pushcfunction(state, Create);			//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				
				//
				//	Finish table creation
				lua_setglobal(state, "Ents");				//	Stack Empty
				//
				//	[Result]
				//	Ents.BaseMetatable = {}					|	Single Table
				//	Ents.ObjectMetatables = {}				|	Table of tables
				//	Ents.Objects = {}						|	LightUserdata objects
				//	Ents.RegisterBase(table)				|	CFunction - table metatable
				//	Ents.Register(string, table)			|	CFunction - string classname, table metatable
				//	local NewEnt = Ents.Create(string)		|	CFunction - returns new LightUserdata object
			}

			void Deinitialize()
			{

			}
		}
	}
}