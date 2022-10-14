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
					lua_getglobal(L, "Ply");
					//	-2 (1)	| in_table
					//	-1 (2)	| Ply table
					//
					lua_pushstring(L, "BaseMetatable");
					//	-3 (1)	| in_table
					//	-2 (2)	| Ply table
					//	-1 (3)	| 'BaseMetatable'
					//
					lua_insert(L, 1);
					//	-3 (1)	| 'BaseMetatable'
					//	-2 (2)	| table_in
					//	-1 (3)	| Ply table
					//
					lua_insert(L, 1);
					//	-3 (1)	| Ply table
					//	-2 (2)	| 'BaseMetatable'
					//	-1 (3)	| table_in
					//
					lua_settable(L, -3);
					//	-1 (1)	| Ply table
					//
					lua_pushstring(L, "BaseMetatable");
					//	-2 (1)	| Ply table
					//	-1 (2)	| 'BaseMetatable'
					//
					lua_gettable(L, -2);
					//	-2 (1)	| Ply table
					//	-1 (2)	| BaseMetatable table
					//
					lua_pushstring(L, "__index");
					//	-3 (1)	| Ply table
					//	-2 (2)	| BaseMetatable table
					//	-1 (3)	| "__index"
					//
					lua_pushstring(L, "BaseMetatable");
					//	-4 (1)	| Ply table
					//	-3 (2)	| BaseMetatable table
					//	-2 (3)	| "__index"
					//	-1 (4)	| "BaseMetatable"
					//
					lua_gettable(L, -4);
					//	-4 (1)	| Ply table
					//	-3 (2)	| BaseMetatable table
					//	-2 (3)	| "__index"
					//	-1 (4)	| BaseMetatable table
					//
					lua_settable(L, -3);
					//	-2 (1)	| Ply table
					//	-1 (2)	| BaseMetatable table
					//
					lua_settop(L, 0);
					//	Stack Empty
				}
				return 1;
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
				return 1;
			}

			//
			//	Returns a Player* from a player table index on the stack
			//	returns nullptr if the function fails in any way
			Player* GetPlayer(lua_State* L, const int index)
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
								Player* Player_ = reinterpret_cast<Player*>(lua_touserdata(L, -1));
								lua_remove(L, lua_gettop(L));
								//	?		|	?
								//	index	|	player object table
								//	?		|	?
								//
								return Player_;
							}
							else {
								wxLogMessage("[LUA:Cerr] GetPlayer (code 4)");
							}
						}
						else {
							wxLogMessage("[LUA:Cerr] GetPlayer (code 3)");
						}
					}
					else {
						wxLogMessage("[LUA:Cerr] GetPlayer (code 2)");
					}
				}
				else {
					wxLogMessage("[LUA:Cerr] GetPlayer (code 1)");
				}
				return nullptr;
				//if (lua_istable(L, -2)) {
				//	//	-2	(1)	|	player object table
				//	//	-1	(2)	|	item_class string
				//	//
				//	if (lua_getfield(L, -2, "__name") == LUA_TSTRING) {
				//		//	-3	(1)	|	player object table
				//		//	-2	(2)	|	item_class string
				//		//	-1	(3)	|	__name string
				//		//
				//		if (strcmp(lua_tostring(L, -1), "Player") == 0) {
				//			lua_remove(L, -1);
				//			//	-2	(1)	|	player object table
				//			//	-1	(2)	|	item_class string
				//			//
				//			if (lua_getfield(L, -2, "__pointer") == LUA_TLIGHTUSERDATA) {
				//				//	-3	(1)	|	player object table
				//				//	-2	(2)	|	item_class string
				//				//	-1	(3)	|	__pointer lightuserdata
				//				//
				//				Player* Player_ = reinterpret_cast<Player*>(lua_touserdata(L, -1));
				//				lua_remove(L, lua_gettop(L));
				//				//	-2	(1)	|	player object table
				//				//	-1	(2)	|	item_class string
				//				//
				//				wxLogMessage("Player Object Name %s", Player_->Name.c_str());
				//			}
				//			else {
				//				wxLogMessage("Player:Give() must be called on a valid player object (code 4)");
				//				lua_settop(L, 0);	//	Empty Stack
				//			}
				//		}
				//		else {
				//			wxLogMessage("Player:Give() must be called on a valid player object (code 3) %s");
				//			lua_settop(L, 0);	//	Empty Stack
				//		}
				//	} else {
				//		wxLogMessage("Player:Give() must be called on a valid player object (code 2)");
				//		lua_settop(L, 0);	//	Empty Stack
				//	}
				//}
				//else {	//	This branch should be impossible to reach?
				//	wxLogMessage("Player:Give() must be called on a valid player object (code 1)");
				//	lua_settop(L, 0);	//	Empty Stack
				//}
			}

			//
			//	Player:Give(item_class (string))
			//	-2	(1)	|	player object table
			//	-1	(2)	|	item_class string
			int Player_Give(lua_State* L)
			{
				const int args = lua_gettop(L);
				if (args == 2) {
					if (lua_isstring(L, -1)) {
						Player* Ply_ = GetPlayer(L, -2);
						Item* NewItm_ = Itm::Create(lua_tostring(L, -1));
						Ply_->GiveItem(NewItm_);
						wxLogMessage("Gave player (%u) item (%u)", Ply_->GetNodeID(), NewItm_->GetNodeID());
					}
					else {
						wxLogMessage("Player:Give() argument 1 must be a valid item type string");
						lua_settop(L, 0);	//	Empty Stack
					}
				}
				else {
					wxLogMessage("Player:Give() invalid argument count");
					lua_settop(L, 0);	//	Empty Stack
				}
				return 0;
			}

			void PushNewPlayer(lua_State* L, Player* NewPlayer)
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
				lua_pushstring(L, "Give");
				//	-2	| Player table
				//	-1	| 'Give'
				//
				lua_pushcfunction(L, Player_Give);
				//	-3	| Player table
				//	-2	| 'Give'
				//	-1	| Player_Give function
				//
				lua_settable(L, -3);
				//	-1	| Player table
				//
			}

			void Create(Player* NewPlayer, const char* Classname)
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
					wxLogMessage("[LUA] Attempted to create player object with nonexistent classname (%s), falling back to (ply_default).", Classname);
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
					wxLogMessage("[Lua] PushPlayer tried to retrieve a nonexistent player id %i", ObjectID);
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