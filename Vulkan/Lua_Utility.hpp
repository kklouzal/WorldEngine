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

			int TraceLine(lua_State* L)
			{
				//	-2	(1)	|	from vector3
				//	-1	(2)	|	to vector3
				//
				glm::vec3* from = reinterpret_cast<glm::vec3*>(lua_touserdata(L, -2));
				glm::vec3* to = reinterpret_cast<glm::vec3*>(lua_touserdata(L, -1));
				btVector3 from_(from->x, from->y, from->z);
				btVector3 to_(to->x, to->y, to->z);
				btCollisionWorld::ClosestRayResultCallback closestResults(from_, to_);
				closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
				WorldEngine::VulkanDriver::dynamicsWorld->rayTest(from_, to_, closestResults);
				//lua_remove(L, lua_gettop(L));
				//lua_remove(L, lua_gettop(L));

				lua_newtable(L);
				//	-1	| result table
				//
				lua_pushstring(L, "HasHit");
				lua_pushboolean(L, closestResults.hasHit());
				lua_settable(L, -3);
				
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
				lua_pushstring(state, "TraceLine");			//	-2 | key
				lua_pushcfunction(state, TraceLine);			//	-1 | value
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