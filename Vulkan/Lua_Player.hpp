#pragma once

namespace WorldEngine
{
	namespace LUA
	{
		namespace Ply
		{
			namespace
			{

			}

			//	TODO: NO SAFETY CHECKS HERE YET...
			//	TODO: I still think players should technically be able to be any scenenode.. need to look into this..
			int GetPos(lua_State* L)
			{
				//	-1	(1)	|	Item table
				//
				lua_getfield(L, -1, "__pointer");
				//	-2	(1)	|	Item table
				//	-1	(2)	|	Item pointer
				//
				CharacterSceneNode* Itm = reinterpret_cast<CharacterSceneNode*>(lua_touserdata(L, -1));
				size_t DataSize = sizeof(glm::vec3);
				void* Data = lua_newuserdata(L, DataSize);
				//glm::vec3(Itm->Model[3]);
				memcpy(Data, &Itm->GetPosition(), DataSize);
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
			//	TODO: NO SAFETY CHECKS HERE YET...
			int GetFireAng(lua_State* L)
			{
				//	-1	(1)	|	Item table
				//
				lua_getfield(L, -1, "__pointer");
				//	-2	(1)	|	Item table
				//	-1	(2)	|	Item pointer
				//
				CharacterSceneNode* Itm = reinterpret_cast<CharacterSceneNode*>(lua_touserdata(L, -1));
				size_t DataSize = sizeof(glm::vec3);
				void* Data = lua_newuserdata(L, DataSize);
				memcpy(Data, &Itm->GetAimVector(), DataSize);
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
			//	TODO: NO SAFETY CHECKS HERE YET...
			int GetFirePos(lua_State* L)
			{
				//	-1	(1)	|	Item table
				//
				lua_getfield(L, -1, "__pointer");
				//	-2	(1)	|	Item table
				//	-1	(2)	|	Item pointer
				//
				CharacterSceneNode* Itm = reinterpret_cast<CharacterSceneNode*>(lua_touserdata(L, -1));
				size_t DataSize = sizeof(glm::vec3);
				void* Data = lua_newuserdata(L, DataSize);
				memcpy(Data, &Itm->GetAimPosition(), DataSize);
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
				printf("[Lua:Register Base] Ply\n");
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
					lua_getglobal(state, "Ply");
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
				printf("[Lua:Register] Ply\n");
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
						lua_getglobal(L, "Ply");
						//	-3 (1)	| in_string
						//	-2 (2)	| in_table
						//	-1 (3)	| Ply table
						//
						lua_pushstring(L, "BaseMetatable");
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ply table
						//	-1 (4)	| 'BaseMetatable'
						//
						lua_gettable(L, -2);
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ply table
						//	-1 (4)	| BaseMetatable table
						//
						lua_setmetatable(L, -3);
						//	-3 (1)	| in_string
						//	-2 (2)	| in_table
						//	-1 (3)	| Ply table
						//
						lua_pushstring(L, "ObjectMetatables");
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ply table
						//	-1 (4)	| 'ObjectMetatables'
						//
						lua_gettable(L, -2);
						//	-4 (1)	| in_string
						//	-3 (2)	| in_table
						//	-2 (3)	| Ply table
						//	-1 (4)	| ObjectMetatables table
						//
						lua_insert(L, 1);
						//	-4 (1)	| ObjectMetatables table
						//	-3 (2)	| in_string
						//	-2 (3)	| in_table
						//	-1 (4)	| Ply table
						//
						lua_insert(L, 1);
						//	-4 (1)	| Ply table
						//	-3 (2)	| ObjectMetatables table
						//	-2 (3)	| in_string
						//	-1 (4)	| in_table
						//
						lua_settable(L, -3);
						//	-2 (1)	| Ply table
						//	-1 (2)	| ObjectMetatables table
						//
						lua_settop(L, 0);
						//	Stack Empty
					}
				}
				return 0;
			}

			//
			//	Returns a Player* from a player table index on the stack
			//	returns nullptr if the function fails in any way
			CharacterSceneNode* GetPlayer(lua_State* L, const int index)
			{
				//	?		|	?
				//	index	|	player object table
				//	?		|	?
				if (lua_istable(L, index)) {
					if (lua_getfield(L, index, "__name") == LUA_TSTRING) {
						//	?		|	?
						//	index	|	player object table
						//	?		|	?
						//	-1		|	__name string
						//
						if (strcmp(lua_tostring(L, -1), "Player") == 0) {
							lua_remove(L, -1);
							//	?		|	?
							//	index	|	player object table
							//	?		|	?
							//
							if (lua_getfield(L, index, "__pointer") == LUA_TLIGHTUSERDATA) {
								//	?		|	?
								//	index	|	player object table
								//	?		|	?
								//	-1		|	__pointer lightuserdata
								//
								CharacterSceneNode* Player_ = reinterpret_cast<CharacterSceneNode*>(lua_touserdata(L, -1));
								lua_remove(L, lua_gettop(L));
								//	?		|	?
								//	index	|	player object table
								//	?		|	?
								//
								return Player_;
							}
							else {
								printf("[LUA:Cerr] GetPlayer (code 4)\n");
							}
						}
						else {
							printf("[LUA:Cerr] GetPlayer (code 3)\n");
						}
					}
					else {
						printf("[LUA:Cerr] GetPlayer (code 2)\n");
					}
				}
				else {
					printf("[LUA:Cerr] GetPlayer (code 1)\n");
				}
				return nullptr;
			}

			void PushNewPlayer(lua_State* L, CharacterSceneNode* NewPlayer)
			{
				lua_newtable(L);
				//	-1	| Player table
				//
				lua_pushstring(L, "__pointer");
				//	-2	| Player table
				//	-1	| '__object'
				//
				lua_pushlightuserdata(L, NewPlayer);
				//	-3	| Player table
				//	-2	| '__object'
				//	-1	| NewPlayer lightuserdata
				//
				lua_settable(L, -3);
				//	-1	| Player table
				//
				lua_pushstring(L, "__name");
				//	-2	| Player table
				//	-1	| '__name'
				//
				lua_pushstring(L, "Player");
				//	-3	| Player table
				//	-2	| '__name'
				//	-1	| 'Player'
				//
				lua_settable(L, -3);
				//	-1	| Player table
				//
				lua_pushstring(state, "GetPos");
				lua_pushcfunction(state, GetPos);
				lua_settable(state, -3);
				//	-1	| Object table
				//
				lua_pushstring(state, "GetFireAng");
				lua_pushcfunction(state, GetFireAng);
				lua_settable(state, -3);
				//	-1	| Object table
				//
				lua_pushstring(state, "GetFirePos");
				lua_pushcfunction(state, GetFirePos);
				lua_settable(state, -3);
				//	-1	| Object table
				//
			}

			void Create(CharacterSceneNode* NewPlayer, const char* Classname)
			{
				PushNewPlayer(state, NewPlayer);
				//	-1	(1)	| Player table
				//
				lua_getglobal(state, "Ply");
				//	-2 (1)	| Player table
				//	-1 (2)	| Ply table
				//
				lua_pushstring(state, "ObjectMetatables");
				//	-3 (1)	| Player table
				//	-2 (2)	| Ply table
				//	-1 (3)	| 'ObjectMetatables'
				//
				lua_gettable(state, -2);
				//	-3 (1)	| Player table
				//	-2 (2)	| Ply table
				//	-1 (3)	| ObjectMetatables table
				//
				lua_pushstring(state, Classname);
				//	-4 (1)	| Player table
				//	-3 (2)	| Ply table
				//	-2 (3)	| ObjectMetatables table
				//	-1 (4)	| in_string (classname)
				//
				lua_gettable(state, -2);
				//	-4 (1)	| Player table
				//	-3 (2)	| Ply table
				//	-2 (3)	| ObjectMetatables table
				//	-1 (4)	| (classname) table (or nil)
				//
				if (!lua_istable(state, -1))
				{
					printf("[LUA] Attempted to create player object with nonexistent classname (%s), falling back to (ply_default).\n", Classname);
					lua_settop(state, -2);
					//	-3 (1)	| Player table
					//	-2 (2)	| Ply table
					//	-1 (3)	| ObjectMetatables table
					//
					lua_pushstring(state, "ply_default");
					//	-4 (1)	| Player table
					//	-3 (2)	| Ply table
					//	-2 (3)	| ObjectMetatables table
					//	-1 (4)	| 'ply_default'
					//
					lua_gettable(state, -2);
					//	-4 (1)	| Player table
					//	-3 (2)	| Ply table
					//	-2 (3)	| ObjectMetatables table
					//	-1 (4)	| ply_default table
					//
				}
				lua_setmetatable(state, -4);
				//	-3 (1)	| Player table
				//	-2 (2)	| Ply table
				//	-1 (3)	| ObjectMetatables table
				//
				lua_settop(state, -2);
				//	-2 (1)	| Player table
				//	-1 (2)	| Ply table
				//
				lua_pushstring(state, "Objects");
				//	-3 (1)	| Player table
				//	-2 (2)	| Ply table
				//	-1 (3)	| 'Objects'
				//
				lua_gettable(state, -2);
				//	-3 (1)	| Player table
				//	-2 (2)	| Ply table
				//	-1 (3)	| Objects table
				//
				lua_pushinteger(state, NewPlayer->GetNodeID());
				//	-4 (1)	| Player table
				//	-3 (2)	| Ply table
				//	-2 (3)	| Objects table
				//	-1 (4)	| PlayerID integer
				//
				lua_insert(state, 1);
				//	-4 (1)	| PlayerID integer
				//	-3 (2)	| Player table
				//	-2 (3)	| Ply table
				//	-1 (4)	| Objects table
				//
				lua_insert(state, 1);
				//	-4 (1)	| Objects table
				//	-3 (2)	| PlayerID integer
				//	-2 (3)	| Player table
				//	-1 (4)	| Ply table
				//
				lua_insert(state, 1);
				//	-4 (1)	| Ply table
				//	-3 (2)	| Objects table
				//	-2 (3)	| PlayerID integer
				//	-1 (4)	| Player table
				//
				lua_settable(state, -3);
				//	-2 (1)	| Ply table
				//	-1 (2)	| Objects table
				lua_settop(state, 0);
				//	Stack Empty
			}

			void Load()
			{
				printf("[Lua:Load] Ply Base\n");
				std::filesystem::path BasePath = BaseLevel / "base_ply";
				if (std::filesystem::exists(BasePath) && std::filesystem::is_directory(BasePath))
				{
					std::filesystem::path InitLua = BasePath / "init.lua";
					if (std::filesystem::exists(InitLua))
					{
						printf("\tinit.lua\n");
						LoadFile(InitLua.generic_string().c_str());
					}

				}
				printf("[Lua:Load] Ply Derived\n");
				for (auto const& sub_dir : std::filesystem::directory_iterator{ SPlyLevel })
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
				//	Ply Table
				lua_newtable(state);						//	-3 | Table
				//
				//	Ply.BaseMetatable table
				//	Base metatable for all object metatables
				lua_pushstring(state, "BaseMetatable");		//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Ply.ObjectMetatables table
				//	Unique object type metatables
				lua_pushstring(state, "ObjectMetatables");	//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Ply.Objects table
				//	All created objects
				lua_pushstring(state, "Objects");			//	-2 | key
				lua_newtable(state);						//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Ply.RegisterBase function
				//	Register/overwrite base metatable
				lua_pushstring(state, "RegisterBase");		//	-2 | key
				lua_pushcfunction(state, RegisterBase);		//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)
				//
				//	Ply.Register function
				//	Register/overwrite unique object metatable
				lua_pushstring(state, "Register");			//	-2 | key
				lua_pushcfunction(state, Register);			//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)

				//
				//	Finish table creation
				lua_setglobal(state, "Ply");				//	Stack Empty
				//
				//	[Result]
				//	Ply.BaseMetatable = {}					|	Single Table
				//	Ply.ObjectMetatables = {}				|	Table of tables
				//	Ply.Objects = {}						|	LightUserdata objects
				//	Ply.RegisterBase(table)					|	CFunction - table metatable
				//	Ply.Register(string, table)				|	CFunction - string classname, table metatable
			}

			//	Push the player userdata onto the stack (or nil)
			void PushPlayer(uintmax_t ObjectID)
			{
				lua_getglobal(state, "Ply");
				//	-1 | Ply table
				//
				lua_pushstring(state, "Objects");
				//	-2 | Ply table
				//	-1 | 'Objects'
				//
				lua_gettable(state, -2);
				//	-2 | Ply table
				//	-1 | Objects table
				//
				lua_remove(state, lua_gettop(state) - 1);
				//	-1 | Objects table
				//
				lua_geti(state, -1, ObjectID);
				//	-2 | Objects table
				//	-1 | player table (or nil)
				//
				if (!lua_istable(state, -1))
				{
					printf("[Lua] PushPlayer tried to retrieve a nonexistent player id %i\n", ObjectID);
				}
				lua_remove(state, lua_gettop(state) - 1);
				//	-1 | player table (or nil)
				//
			}

			void Deinitialize()
			{

			}
		}
	}
}