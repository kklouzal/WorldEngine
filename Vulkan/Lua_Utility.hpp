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
				btCollisionWorld::ClosestRayResultCallback Results(from_, to_);
				Results.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;
				WorldEngine::VulkanDriver::dynamicsWorld->rayTest(from_, to_, Results);
				//lua_remove(L, lua_gettop(L));
				//lua_remove(L, lua_gettop(L));

				lua_newtable(L);
				//	-1	| result table
				//
				lua_pushstring(L, "HasHit");
				lua_pushboolean(L, Results.hasHit());
				lua_settable(L, -3);
				//	-1	| result table
				//
				if (Results.hasHit())
				{
					SceneNode* HitNode = reinterpret_cast<SceneNode*>(Results.m_collisionObject->getUserPointer());
					lua_pushstring(L, "HitEntity");
					//
					//	TODO: Check if is player/entity/item/etc..
					//Ply::PushPlayer(HitNode->GetNodeID());
					Ent::PushEntity(HitNode->GetNodeID());
					lua_settable(L, -3);
					//	-1	| result table
					//
					lua_pushstring(L, "HitPosition");
					size_t DataSize = sizeof(glm::vec3);
					void* Data1 = lua_newuserdata(L, DataSize);
					glm::vec3 HitPos{ Results.m_hitPointWorld.x(), Results.m_hitPointWorld.y(), Results.m_hitPointWorld.z() };
					memcpy(Data1, &HitPos, DataSize);
					//	-3	|	result table
					//	-2	|	'HitPosition'
					//	-1	|	vec3 userdata
					//
					lua_getglobal(L, "WorldEngine_Vector3Metatable");
					//	-4	|	result table
					//	-3	|	'HitPosition'
					//	-2	|	vec3 userdata
					//	-1	|	metatable
					//
					lua_setmetatable(L, -2);
					//	-3	|	result table
					//	-2	|	'HitPosition'
					//	-1	|	vec3 userdata
					//
					lua_settable(L, -3);
					//	-1	| result table
					//
					lua_pushstring(L, "HitNormal");
					void* Data2 = lua_newuserdata(L, DataSize);
					glm::vec3 HitNorm{ Results.m_hitNormalWorld.x(), Results.m_hitNormalWorld.y(), Results.m_hitNormalWorld.z() };
					memcpy(Data2, &HitNorm, DataSize);
					//	-3	|	result table
					//	-2	|	'HitPosition'
					//	-1	|	vec3 userdata
					//
					lua_getglobal(L, "WorldEngine_Vector3Metatable");
					//	-4	|	result table
					//	-3	|	'HitPosition'
					//	-2	|	vec3 userdata
					//	-1	|	metatable
					//
					lua_setmetatable(L, -2);
					//	-3	|	result table
					//	-2	|	'HitPosition'
					//	-1	|	vec3 userdata
					//
					lua_settable(L, -3);
					//	-1	| result table
					//
					lua_pushstring(L, "ClosestHitFraction");
					lua_pushnumber(L, Results.m_closestHitFraction);
					lua_settable(L, -3);
					//	-1	| result table
					//
				}
				
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