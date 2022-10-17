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
			//	Returns a SceneNode* from an object table index on the stack
			//	returns nullptr if the function fails in any way
			SceneNode* GetEntity(lua_State* L, const int index)
			{
				//	?		|	?
				//	index	|	object table
				//	?		|	?
				if (lua_istable(L, index)) {
					if (lua_getfield(L, index, "__name") == LUA_TSTRING) {
						//	?		|	?
						//	index	|	object table
						//	?		|	?
						//	-1		|	__name string
						//
						if (strcmp(lua_tostring(L, -1), "Entity") == 0) {
							lua_remove(L, -1);
							//	?		|	?
							//	index	|	object table
							//	?		|	?
							//
							if (lua_getfield(L, index, "__pointer") == LUA_TLIGHTUSERDATA) {
								//	?		|	?
								//	index	|	object table
								//	?		|	?
								//	-1		|	__pointer lightuserdata
								//
								SceneNode* Object_ = reinterpret_cast<SceneNode*>(lua_touserdata(L, -1));
								lua_remove(L, lua_gettop(L));
								//	?		|	?
								//	index	|	player object table
								//	?		|	?
								//
								return Object_;
							}
							else {
								printf("[LUA:Cerr] GetEntity (code 4)\n");
							}
						}
						else {
							printf("[LUA:Cerr] GetEntity (code 3)\n");
						}
					}
					else {
						printf("[LUA:Cerr] GetEntity (code 2)\n");
					}
				}
				else {
					printf("[LUA:Cerr] GetEntity (code 1)\n");
				}
				return nullptr;
			}

			//	TODO: NO SAFETY CHECKS HERE YET...
			int SetPos(lua_State* L)
			{
				//	-2	|	Object table
				//	-1	|	Vector3 userdata
				if (lua_gettop(L) == 2)
				{
					if (lua_isuserdata(L, -1))
					{
						SceneNode* Object = GetEntity(L, -2);
						if (Object)
						{
							glm::vec3* Vec = reinterpret_cast<glm::vec3*>(lua_touserdata(L, -1));
							Object->SetPosition(*Vec);
						}
					}
				}
				else {
					printf("[Lua][Entity] SetPos: Invalid arguments\n");
				}
				return 0;
			}

			//	TODO: NO SAFETY CHECKS HERE YET...
			int GetPos(lua_State* L)
			{
				//	-1	(1)	|	Item table
				//
				lua_getfield(L, -1, "__pointer");
				//	-2	(1)	|	Item table
				//	-1	(2)	|	Item pointer
				//
				SceneNode* Object = reinterpret_cast<SceneNode*>(lua_touserdata(L, -1));
				size_t DataSize = sizeof(glm::vec3);
				void* Data = lua_newuserdata(L, DataSize);
				memcpy(Data, &Object->GetPosition(), DataSize);
				//	-1	(1)	|	vec3 userdata
				//
				lua_getglobal(L, "WorldEngine_Vector3Metatable");
				//	-2	(1)	|	vec3 userdata
				//	-1	(2)	|	metatable
				//
				lua_setmetatable(L, -2);
				//	-1	(1)	|	vec3 userdata
				//
				return 1;
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
						lua_getglobal(state, "Ents");
						//	-3 (1)	| in_string
						//	-2 (2)	| in_table
						//	-1 (3)	| Ents table
						//
						lua_pushstring(state, "BaseMetatable");
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ents table
						//	-1 (4)	| 'BaseMetatable'
						//
						lua_gettable(state, -2);
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ents table
						//	-1 (4)	| BaseMetatable table
						//
						lua_setmetatable(state, -3);
						//	-3 (1)	| in_string
						//	-2 (2)	| in_table
						//	-1 (3)	| Ents table
						//
						lua_pushstring(state, "ObjectMetatables");
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ents table
						//	-1 (4)	| 'ObjectMetatables'
						//
						lua_gettable(state, -2);
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ents table
						//	-1 (4)	| ObjectMetatables table
						//
						lua_insert(state, 1);
						//	-4 (1)	| ObjectMetatables table
						//	-3 (2)	| in_string
						//	-2 (3)	| in_table
						//	-1 (4)	| Ents table
						//
						lua_insert(state, 1);
						//	-4 (1)	| Ents table
						//	-3 (2)	| ObjectMetatables table
						//	-2 (3)	| in_string
						//	-1 (4)	| in_table
						//
						lua_settable(state, -3);
						//	-2 (1)	| Ents table
						//	-1 (2)	| ObjectMetatables table
						//
						lua_settop(state, 0);
						//	Stack Empty
					}
				}
				return 0;
			}

			void PushNewEntity(lua_State* L, SceneNode* NewObject)
			{
				lua_newtable(L);
				//	-1	| object table
				//
				lua_pushstring(L, "__pointer");
				//	-2	| object table
				//	-1	| '__object'
				//
				lua_pushlightuserdata(L, NewObject);
				//	-3	| object table
				//	-2	| '__object'
				//	-1	| NewObject lightuserdata
				//
				lua_settable(L, -3);
				//	-1	| object table
				//
				lua_pushstring(L, "__name");
				//	-2	| object table
				//	-1	| '__name'
				//
				lua_pushstring(L, "Entity");
				//	-3	| object table
				//	-2	| '__name'
				//	-1	| 'Entity'
				//
				lua_settable(L, -3);
				//	-1	| object table
				//
				lua_pushstring(state, "GetPos");
				lua_pushcfunction(state, GetPos);
				lua_settable(state, -3);
				//	-1	| Object table
				//
				lua_pushstring(state, "SetPos");
				lua_pushcfunction(state, SetPos);
				lua_settable(state, -3);
				//	-1	| Object table
				//
			}

			void Create(SceneNode* NewObject, const char* Classname)
			{
				PushNewEntity(state, NewObject);
				//	-1	(1)	| object table
				//
				lua_getglobal(state, "Ents");
				//	-2 (1)	| object table
				//	-1 (2)	| main table
				//
				lua_pushstring(state, "ObjectMetatables");
				//	-3 (1)	| object table
				//	-2 (2)	| main table
				//	-1 (3)	| 'ObjectMetatables'
				//
				lua_gettable(state, -2);
				//	-3 (1)	| object table
				//	-2 (2)	| main table
				//	-1 (3)	| ObjectMetatables table
				//
				lua_pushstring(state, Classname);
				//	-4 (1)	| object table
				//	-3 (2)	| main table
				//	-2 (3)	| ObjectMetatables table
				//	-1 (4)	| in_string (classname)
				//
				lua_gettable(state, -2);
				//	-4 (1)	| object table
				//	-3 (2)	| main table
				//	-2 (3)	| ObjectMetatables table
				//	-1 (4)	| (classname) table (or nil)
				//
				if (!lua_istable(state, -1))
				{
					printf("[LUA] Attempted to create entity object with nonexistent classname (%s), falling back to (ent_default).\n", Classname);
					lua_settop(state, -2);
					//	-3 (1)	| object table
					//	-2 (2)	| main table
					//	-1 (3)	| ObjectMetatables table
					//
					lua_pushstring(state, "ent_default");
					//	-4 (1)	| object table
					//	-3 (2)	| main table
					//	-2 (3)	| ObjectMetatables table
					//	-1 (4)	| 'ply_default'
					//
					lua_gettable(state, -2);
					//	-4 (1)	| object table
					//	-3 (2)	| main table
					//	-2 (3)	| ObjectMetatables table
					//	-1 (4)	| ply_default table
					//
				}
				lua_setmetatable(state, -4);
				//	-3 (1)	| object table
				//	-2 (2)	| main table
				//	-1 (3)	| ObjectMetatables table
				//
				lua_settop(state, -2);
				//	-2 (1)	| object table
				//	-1 (2)	| main table
				//
				lua_pushstring(state, "Objects");
				//	-3 (1)	| object table
				//	-2 (2)	| main table
				//	-1 (3)	| 'Objects'
				//
				lua_gettable(state, -2);
				//	-3 (1)	| object table
				//	-2 (2)	| main table
				//	-1 (3)	| Objects table
				//
				lua_pushinteger(state, NewObject->GetNodeID());
				//	-4 (1)	| object table
				//	-3 (2)	| main table
				//	-2 (3)	| Objects table
				//	-1 (4)	| PlayerID integer
				//
				lua_insert(state, 1);
				//	-4 (1)	| PlayerID integer
				//	-3 (2)	| object table
				//	-2 (3)	| main table
				//	-1 (4)	| Objects table
				//
				lua_insert(state, 1);
				//	-4 (1)	| Objects table
				//	-3 (2)	| PlayerID integer
				//	-2 (3)	| object table
				//	-1 (4)	| main table
				//
				lua_insert(state, 1);
				//	-4 (1)	| main table
				//	-3 (2)	| Objects table
				//	-2 (3)	| PlayerID integer
				//	-1 (4)	| object table
				//
				lua_settable(state, -3);
				//	-2 (1)	| main table
				//	-1 (2)	| Objects table
				lua_settop(state, 0);
				//	Stack Empty
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
				//lua_pushstring(state, "Create");			//	-2 | key
				//lua_pushcfunction(state, Create);			//	-1 | value
				//lua_settable(state, -3);					//	(table is now -1)
				
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

			//	Push the object userdata onto the stack (or nil)
			void PushEntity(uintmax_t ObjectID)
			{
				lua_getglobal(state, "Ents");
				//	-1 | main table
				//
				lua_pushstring(state, "Objects");
				//	-2 | main table
				//	-1 | 'Objects'
				//
				lua_gettable(state, -2);
				//	-2 | main table
				//	-1 | Objects table
				//
				lua_remove(state, lua_gettop(state) - 1);
				//	-1 | Objects table
				//
				lua_geti(state, -1, ObjectID);
				//	-2 | Objects table
				//	-1 | object table (or nil)
				//
				if (!lua_istable(state, -1))
				{
					printf("[Lua] PushEntity tried to retrieve a nonexistent object id %i\n", ObjectID);
				}
				lua_remove(state, lua_gettop(state) - 1);
				//	-1 | object table (or nil)
				//
			}
		}
	}
}