#pragma once

namespace WorldEngine
{
	namespace LUA
	{
		namespace Itm
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
				printf("[Lua:Register Base] Item\n");
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
					lua_getglobal(state, "Item");
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
				printf("[Lua:Register] Item\n");
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
						lua_getglobal(L, "Item");
						//	-3 (1)	| in_string
						//	-2 (2)	| in_table
						//	-1 (3)	| Main table
						//
						lua_pushstring(L, "BaseMetatable");
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Main table
						//	-1 (4)	| 'BaseMetatable'
						//
						lua_gettable(L, -2);
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Main table
						//	-1 (4)	| BaseMetatable table
						//
						lua_setmetatable(L, -3);
						//	-3 (1)	| in_string
						//	-2 (2)	| in_table
						//	-1 (3)	| Main table
						//
						lua_pushstring(L, "ObjectMetatables");
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Main table
						//	-1 (4)	| 'ObjectMetatables'
						//
						lua_gettable(L, -2);
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Main table
						//	-1 (4)	| ObjectMetatables table
						//
						lua_insert(L, 1);
						//	-4 (1)	| ObjectMetatables table
						//	-3 (2)	| in_string
						//	-2 (3)	| in_table
						//	-1 (4)	| Main table
						//
						lua_insert(L, 1);
						//	-4 (1)	| Main table
						//	-3 (2)	| ObjectMetatables table
						//	-2 (3)	| in_string
						//	-1 (4)	| in_table
						//
						lua_settable(L, -3);
						//	-2 (1)	| Main table
						//	-1 (2)	| ObjectMetatables table
						//
						lua_settop(L, 0);
						//	Stack Empty
					}
				}
				return 0;
			}


			void PushItem(lua_State* L, const uintmax_t NodeID)
			{
				lua_getglobal(state, "Item");
				//	-1 | Main table
				//
				lua_pushstring(state, "Objects");
				//	-2 | Main table
				//	-1 | 'Objects'
				//
				lua_gettable(state, -2);
				//	-2 | Main table
				//	-1 | Objects table
				//
				lua_remove(state, lua_gettop(state) - 1);
				//	-1 | Objects table
				//
				lua_geti(state, -1, NodeID);
				//	-2 | Objects table
				//	-1 | Object table (or nil)
				//
				if (!lua_istable(state, -1))
				{
					printf("[Lua] PushItem tried to retrieve a nonexistent item id (%Iu)\n", NodeID);
				}
				lua_remove(state, lua_gettop(state) - 1);
				//	-1 | player table (or nil)
				//
			}

			Item* PushNewItem(lua_State* L, uintmax_t NodeID, const char*const Classname)
			{
				Item* NewItem_ = new Item(NodeID, Classname);

				lua_newtable(L);
				//	-1	| Object table
				//
				lua_pushstring(L, "__pointer");
				//	-2	| Object table
				//	-1	| '__pointer'
				//
				lua_pushlightuserdata(L, NewItem_);
				//	-3	| Object table
				//	-2	| '__pointer'
				//	-1	| NewPlayer lightuserdata
				//
				lua_settable(L, -3);
				//	-1	| Object table
				//
				lua_pushstring(L, "__name");
				//	-2	| Object table
				//	-1	| '__name'
				//
				lua_pushstring(L, "Item");
				//	-3	| Object table
				//	-2	| '__name'
				//	-1	| 'Item'
				//
				lua_settable(L, -3);
				//	-1	| Object table
				//
				return NewItem_;
			}

			Item* Create(uintmax_t NodeID, const char*const Classname)
			{
				Item* NewItem_ = PushNewItem(state, NodeID, Classname);
				//	-1	(1)	| Object table
				//
				lua_getglobal(state, "Item");
				//	-2 (1)	| Object table
				//	-1 (2)	| Main table
				//
				lua_pushstring(state, "ObjectMetatables");
				//	-3 (1)	| Object table
				//	-2 (2)	| Main table
				//	-1 (3)	| 'ObjectMetatables'
				//
				lua_gettable(state, -2);
				//	-3 (1)	| Object table
				//	-2 (2)	| Main table
				//	-1 (3)	| ObjectMetatables table
				//
				lua_pushstring(state, Classname);
				//	-4 (1)	| Object table
				//	-3 (2)	| Main table
				//	-2 (3)	| ObjectMetatables table
				//	-1 (4)	| in_string (classname)
				//
				lua_gettable(state, -2);
				//	-4 (1)	| Object table
				//	-3 (2)	| Main table
				//	-2 (3)	| ObjectMetatables table
				//	-1 (4)	| (classname) table (or nil)
				//
				if (!lua_istable(state, -1))
				{
					printf("[LUA] Attempted to create item object with nonexistent classname (%s), falling back to (itm_default).\n", Classname);
					lua_settop(state, -2);
					//	-3 (1)	| Object table
					//	-2 (2)	| Main table
					//	-1 (3)	| ObjectMetatables table
					//
					lua_pushstring(state, "itm_default");
					//	-4 (1)	| Object table
					//	-3 (2)	| Main table
					//	-2 (3)	| ObjectMetatables table
					//	-1 (4)	| 'itm_default'
					//
					lua_gettable(state, -2);
					//	-4 (1)	| Object table
					//	-3 (2)	| Main table
					//	-2 (3)	| ObjectMetatables table
					//	-1 (4)	| itm_default table
					//
				}
				lua_setmetatable(state, -4);
				//	-3 (1)	| Object table
				//	-2 (2)	| Main table
				//	-1 (3)	| ObjectMetatables table
				//
				lua_settop(state, -2);
				//	-2 (1)	| Object table
				//	-1 (2)	| Main table
				//
				lua_pushstring(state, "Objects");
				//	-3 (1)	| Object table
				//	-2 (2)	| Main table
				//	-1 (3)	| 'Objects'
				//
				lua_gettable(state, -2);
				//	-3 (1)	| Object table
				//	-2 (2)	| Main table
				//	-1 (3)	| Objects table
				//
				lua_pushinteger(state, NewItem_->GetNodeID());
				//	-4 (1)	| Object table
				//	-3 (2)	| Main table
				//	-2 (3)	| Objects table
				//	-1 (4)	| NodeID integer
				//
				lua_insert(state, 1);
				//	-4 (1)	| NodeID integer
				//	-3 (2)	| Object table
				//	-2 (3)	| Main table
				//	-1 (4)	| Objects table
				//
				lua_insert(state, 1);
				//	-4 (1)	| Objects table
				//	-3 (2)	| NodeID integer
				//	-2 (3)	| Object table
				//	-1 (4)	| Main table
				//
				lua_insert(state, 1);
				//	-4 (1)	| Main table
				//	-3 (2)	| Objects table
				//	-2 (3)	| NodeID integer
				//	-1 (4)	| Object table
				//
				lua_settable(state, -3);
				//	-2 (1)	| Main table
				//	-1 (2)	| Objects table
				lua_settop(state, 0);
				//	Stack Empty
				return NewItem_;
			}

			void Load()
			{
				printf("[Lua:Load] Item Base\n");
				std::filesystem::path BasePath = BaseLevel / "base_item";
				if (std::filesystem::exists(BasePath) && std::filesystem::is_directory(BasePath))
				{
					std::filesystem::path InitLua = BasePath / "init.lua";
					if (std::filesystem::exists(InitLua))
					{
						printf("\tinit.lua\n");
						LoadFile(InitLua.generic_string().c_str());
					}

				}
				printf("[Lua:Load] Item Derived\n");
				for (auto const& sub_dir : std::filesystem::directory_iterator{ SItmLevel })
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
				//	Component Table
				lua_newtable(state);						//	-3 | Table
				//
				//	Component.BaseMetatable table
				//	Base metatable for all object metatables
				lua_pushstring(state, "BaseMetatable");		//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Component.ObjectMetatables table
				//	Unique object type metatables
				lua_pushstring(state, "ObjectMetatables");	//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Component.Objects table
				//	All created objects
				lua_pushstring(state, "Objects");			//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Component.RegisterBase function
				//	Register/overwrite base metatable

				lua_pushstring(state, "RegisterBase");		//	-2 | key
				lua_pushcfunction(state, RegisterBase);		//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Component.Register function
				//	Register/overwrite unique object metatable
				lua_pushstring(state, "Register");			//	-2 | key
				lua_pushcfunction(state, Register);			//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)

				//
				//	Finish table creation
				lua_setglobal(state, "Item");				//	Stack Empty
				//
				//	[Result]
				//	Ents.BaseMetatable = {}					|	Single Table
				//	Ents.ObjectMetatables = {}				|	Table of tables
				//	Ents.Objects = {}						|	LightUserdata objects
				//	Ents.RegisterBase(table)				|	CFunction - table metatable
				//	Ents.Register(string, table)			|	CFunction - string classname, table metatable
			}

			void Deinitialize()
			{

			}

			//
			//	CallFunc
			//	NodeID = item node id to call function on
			//	FunctionName = lua function name to call
			//	*note* only works for functions with 0 arguments
			void CallFunc(uintmax_t NodeID, const char* const FunctionName)
			{
				PushItem(state, NodeID);
				//	-1	|	Object table (or nil)
				if (lua_istable(state, -1))
				{
					if (lua_getfield(state, -1, "__name") == LUA_TSTRING)
					{
						//	-2	|	Object table
						//	-1	|	'__name' value
						if (strcmp(lua_tostring(state, -1), "Item") == 0)
						{
							lua_remove(state, -1);
							//	-1	|	Object table
							//
							int type = lua_getfield(state, -1, FunctionName);
							if (type == LUA_TFUNCTION)
							{
								//	-2	|	Object table
								//	-1	|	FunctionName function
								//
								lua_call(state, 0, 0);
								//	-1	|	Object table
								//
								lua_remove(state, -1);
								//	Stack Empty
								return;
							}
							else {
								printf("[Lua][CallFunc] Error: Function not a function (%s)(%i)\n", FunctionName, type);
							}
							//	-2	|	Object table
							//	-1	|	something not a function
							lua_remove(state, -1);
							//	-1	|	Object table
							// 
							lua_remove(state, -1);
							//	Stack Empty
							return;
						}
						else {
							printf("[Lua][CallFunc] Error: non-item table (3)\n");
						}
					}
					else {
						printf("[Lua][CallFunc] Error: non-item table (2)\n");
					}
					lua_remove(state, -1);
					//	-1	|	Object table
					//
					lua_remove(state, -1);
					//	Stack Empty
					return;
				}
				else {
					printf("[Lua][CallFunc] Error: non-item table (1)\n");
				}
				lua_remove(state, -1);
				//	Stack Empty
			}
		}
	}
}