#pragma once

namespace WorldEngine
{
	namespace LUA
	{
		namespace Util
		{
			namespace
			{

			}

			int CastRay(lua_State* L)
			{

				return 1;
			}

			void Initialize()
			{
				//
				//	Util Table
				lua_newtable(state);						//	-3 | Table
				//
				//	GM.Create function
				//	Create gamemode object
				lua_pushstring(state, "CastRay");			//	-2 | key
				lua_pushcfunction(state, CastRay);			//	-1 | value
				lua_settable(state, -3);					//	(table is now -1)

				//
				//	Finish table creation
				lua_setglobal(state, "Util");				//	Stack Empty
				//
				//	[Result]
				//	local Result = Util.CastRay(From, To)	|	CFunction - returns raycast results table
			}

			void Deinitialize()
			{

			}
		}
	}
}