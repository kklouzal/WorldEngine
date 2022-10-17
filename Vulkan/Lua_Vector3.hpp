#pragma once

namespace WorldEngine
{
	namespace LUA
	{
		namespace Vector3
		{
			int Distance(lua_State* L)
			{
				//	-2	|	Vector3 userdata (left)
				//	-1	|	Vector3 userdata (right)
				//
				lua_getfield(L, -2, "__name");
				//	-3	|	Vector3 userdata (left)
				//	-2	|	Vector3 userdata (right)
				//	-1	|	__name
				if (strcmp(lua_tostring(L, -1), "Vector3") == 0)
				{
					lua_remove(L, lua_gettop(L));
					//	-2	|	Vector3 userdata (left)
					//	-1	|	Vector3 userdata (right)
					//
					lua_getfield(L, -1, "__name");
					//	-3	|	Vector3 userdata (left)
					//	-2	|	Vector3 userdata (right)
					//	-1	|	__name
					if (strcmp(lua_tostring(L, -1), "Vector3") == 0)
					{
						lua_remove(L, lua_gettop(L));
						//	-2	|	Vector3 userdata (left)
						//	-1	|	Vector3 userdata (right)
						//
						size_t DataSize = sizeof(glm::vec3);
						//
						glm::vec3 VecLhs{};
						memcpy(&VecLhs, lua_touserdata(L, -2), DataSize);
						glm::vec3 VecRhs{};
						memcpy(&VecRhs, lua_touserdata(L, -1), DataSize);
						//
						lua_pushnumber(L, glm::distance(VecLhs, VecRhs));
						//lua_remove(L, lua_gettop(L));
						//lua_remove(L, lua_gettop(L));
						//	Stack Empty
						//
						return 1;
					}
					else {
						printf("[Lua][Vector3] Add Error: Right operand not a Vector3\n");	//	TODO: Allow right operand to be a single number
					}
				}
				else {
					printf("[Lua][Vector3] Add Error: Left operand not a Vector3\n");
				}
				lua_remove(L, lua_gettop(L));
				lua_remove(L, lua_gettop(L));
				//	Stack Empty
				return 0;
			}

			int Mul(lua_State* L)
			{
				//	-2	|	Vector3 userdata (left)
				//	-1	|	Vector3 userdata (right)
				//
				lua_getfield(L, -2, "__name");
				//	-3	|	Vector3 userdata (left)
				//	-2	|	Vector3 userdata (right)
				//	-1	|	__name
				if (strcmp(lua_tostring(L, -1), "Vector3") == 0)
				{
					lua_remove(L, lua_gettop(L));
					//	-2	|	Vector3 userdata (left)
					//	-1	|	Vector3 userdata (right)
					//
					lua_getfield(L, -1, "__name");
					//	-3	|	Vector3 userdata (left)
					//	-2	|	Vector3 userdata (right)
					//	-1	|	__name
					if (strcmp(lua_tostring(L, -1), "Vector3") == 0)
					{
						lua_remove(L, lua_gettop(L));
						//	-2	|	Vector3 userdata (left)
						//	-1	|	Vector3 userdata (right)
						//
						size_t DataSize = sizeof(glm::vec3);
						//
						glm::vec3 VecLhs{};
						memcpy(&VecLhs, lua_touserdata(L, -2), DataSize);
						glm::vec3 VecRhs{};
						memcpy(&VecRhs, lua_touserdata(L, -1), DataSize);
						glm::vec3 Res{ VecLhs * VecRhs };
						//lua_remove(L, lua_gettop(L));
						//lua_remove(L, lua_gettop(L));
						//	Stack Empty
						//
						void* Data = lua_newuserdata(L, DataSize);
						memcpy(Data, &Res, DataSize);
						//	-3	|	Vector3 userdata (left)
						//	-2	|	Vector3 userdata (right)
						//	-1	|	vec3 userdata (result)
						//
						lua_getglobal(L, "WorldEngine_Vector3Metatable");
						//	-4	|	Vector3 userdata (left)
						//	-3	|	Vector3 userdata (right)
						//	-2	|	vec3 userdata (result)
						//	-1	|	metatable
						//
						lua_setmetatable(L, -2);
						//	-3	|	Vector3 userdata (left)
						//	-2	|	Vector3 userdata (right)
						//	-1	|	vec3 userdata (result)
						//
						return 1;
					}
					else {
						printf("[Lua][Vector3] Add Error: Right operand not a Vector3\n");	//	TODO: Allow right operand to be a single number
					}
				}
				else {
					printf("[Lua][Vector3] Add Error: Left operand not a Vector3\n");
				}
				lua_remove(L, lua_gettop(L));
				lua_remove(L, lua_gettop(L));
				//	Stack Empty
				return 0;
			}

			int Div(lua_State* L)
			{
				//	-2	|	Vector3 userdata (left)
				//	-1	|	Vector3 userdata (right)
				//
				lua_getfield(L, -2, "__name");
				//	-3	|	Vector3 userdata (left)
				//	-2	|	Vector3 userdata (right)
				//	-1	|	__name
				if (strcmp(lua_tostring(L, -1), "Vector3") == 0)
				{
					lua_remove(L, lua_gettop(L));
					//	-2	|	Vector3 userdata (left)
					//	-1	|	Vector3 userdata (right)
					//
					lua_getfield(L, -1, "__name");
					//	-3	|	Vector3 userdata (left)
					//	-2	|	Vector3 userdata (right)
					//	-1	|	__name
					if (strcmp(lua_tostring(L, -1), "Vector3") == 0)
					{
						lua_remove(L, lua_gettop(L));
						//	-2	|	Vector3 userdata (left)
						//	-1	|	Vector3 userdata (right)
						//
						size_t DataSize = sizeof(glm::vec3);
						//
						glm::vec3 VecLhs{};
						memcpy(&VecLhs, lua_touserdata(L, -2), DataSize);
						glm::vec3 VecRhs{};
						memcpy(&VecRhs, lua_touserdata(L, -1), DataSize);
						glm::vec3 Res{ VecLhs / VecRhs };
						//lua_remove(L, lua_gettop(L));
						//lua_remove(L, lua_gettop(L));
						//	Stack Empty
						//
						void* Data = lua_newuserdata(L, DataSize);
						memcpy(Data, &Res, DataSize);
						//	-3	|	Vector3 userdata (left)
						//	-2	|	Vector3 userdata (right)
						//	-1	|	vec3 userdata (result)
						//
						lua_getglobal(L, "WorldEngine_Vector3Metatable");
						//	-4	|	Vector3 userdata (left)
						//	-3	|	Vector3 userdata (right)
						//	-2	|	vec3 userdata (result)
						//	-1	|	metatable
						//
						lua_setmetatable(L, -2);
						//	-3	|	Vector3 userdata (left)
						//	-2	|	Vector3 userdata (right)
						//	-1	|	vec3 userdata (result)
						//
						return 1;
					}
					else {
						printf("[Lua][Vector3] Add Error: Right operand not a Vector3\n");	//	TODO: Allow right operand to be a single number
					}
				}
				else {
					printf("[Lua][Vector3] Add Error: Left operand not a Vector3\n");
				}
				lua_remove(L, lua_gettop(L));
				lua_remove(L, lua_gettop(L));
				//	Stack Empty
				return 0;
			}

			int Add(lua_State* L)
			{
				//	-2	|	Vector3 userdata (left)
				//	-1	|	Vector3 userdata (right)

				lua_getfield(L, -2, "__name");
				//	-1	|	__name
				if (strcmp(lua_tostring(L, -1), "Vector3") == 0)
				{
					lua_remove(L, lua_gettop(L));
					//	Stack Returned
					//
					lua_getfield(L, -1, "__name");
					//	-1	|	__name
					if (strcmp(lua_tostring(L, -1), "Vector3") == 0)
					{
						lua_remove(L, lua_gettop(L));
						//	Stack Returned
						//
						size_t DataSize = sizeof(glm::vec3);
						//
						glm::vec3 VecLhs{};
						memcpy(&VecLhs, lua_touserdata(L, -2), DataSize);
						glm::vec3 VecRhs{};
						memcpy(&VecRhs, lua_touserdata(L, -1), DataSize);
						glm::vec3 Res{ VecLhs + VecRhs };
						//lua_remove(L, lua_gettop(L));
						//lua_remove(L, lua_gettop(L));
						//	Stack Empty
						//
						void* Data = lua_newuserdata(L, DataSize);
						memcpy(Data, &Res, DataSize);
						//	-1	(1)	|	vec3 userdata
						//
						lua_getglobal(L, "WorldEngine_Vector3Metatable");
						//	-2	(1)	|	vec3 userdata
						//	-1	(2)	|	metatable
						//
						lua_setmetatable(L, -2);
						//	-1	(1)	|	
						return 1;
					}
					else {
						printf("[Lua][Vector3] Add Error: Right operand not a Vector3\n");	//	TODO: Allow right operand to be a single number
					}
				}
				else {
					printf("[Lua][Vector3] Add Error: Left operand not a Vector3\n");
				}
				lua_remove(L, lua_gettop(L));
				lua_remove(L, lua_gettop(L));
				//	Stack Empty
				return 0;
			}
			int Sub(lua_State* L)
			{
				//	-2	|	Vector3 userdata (left)
				//	-1	|	Vector3 userdata (right)

				lua_getfield(L, -2, "__name");
				//	-1	|	__name
				if (strcmp(lua_tostring(L, -1), "Vector3") == 0)
				{
					lua_remove(L, lua_gettop(L));
					//	Stack Returned
					//
					lua_getfield(L, -1, "__name");
					//	-1	|	__name
					if (strcmp(lua_tostring(L, -1), "Vector3") == 0)
					{
						lua_remove(L, lua_gettop(L));
						//	Stack Returned
						//
						size_t DataSize = sizeof(glm::vec3);
						//
						glm::vec3 VecLhs{};
						memcpy(&VecLhs, lua_touserdata(L, -2), DataSize);
						glm::vec3 VecRhs{};
						memcpy(&VecRhs, lua_touserdata(L, -1), DataSize);
						glm::vec3 Res{ VecLhs - VecRhs };
						//lua_remove(L, lua_gettop(L));
						//lua_remove(L, lua_gettop(L));
						//	Stack Empty
						//
						void* Data = lua_newuserdata(L, DataSize);
						memcpy(Data, &Res, DataSize);
						//	-1	(1)	|	vec3 userdata
						//
						lua_getglobal(L, "WorldEngine_Vector3Metatable");
						//	-2	(1)	|	vec3 userdata
						//	-1	(2)	|	metatable
						//
						lua_setmetatable(L, -2);
						//	-1	(1)	|	
						return 1;
					}
					else {
						printf("[Lua][Vector3] Add Error: Right operand not a Vector3\n");	//	TODO: Allow right operand to be a single number
					}
				}
				else {
					printf("[Lua][Vector3] Add Error: Left operand not a Vector3\n");
				}
				lua_remove(L, lua_gettop(L));
				lua_remove(L, lua_gettop(L));
				//	Stack Empty
				return 0;
			}

			int ToString(lua_State* L)
			{
				glm::vec3* Vec = reinterpret_cast<glm::vec3*>(lua_touserdata(L, -1));
				std::string Out("Vector3 X:" + std::to_string(Vec->x) + " Y:" + std::to_string(Vec->y) + " Z:" + std::to_string(Vec->z));
				lua_remove(L, lua_gettop(L)-1);
				lua_pushstring(L, Out.c_str());
				return 1;
			}

			int SetValue(lua_State* L)
			{
				//	-4	(1)	|	Vector3 userdata
				//	-3	(2)	|	X number
				//	-2	(3)	|	Y number
				//	-1	(4)	|	Z number
				if (lua_gettop(L) == 4)
				{
					if (lua_isuserdata(L, -4))
					{
						if (lua_isnumber(L, -3))
						{
							if (lua_isnumber(L, -2))
							{
								if (lua_isnumber(L, -1))
								{
									lua_getfield(L, -4, "__name");
									//	-1	(5)	|	__name
									if (strcmp(lua_tostring(L, -1), "Vector3") == 0)
									{
										lua_remove(L, lua_gettop(L));
										//	Stack Returned
										glm::vec3* Vec = reinterpret_cast<glm::vec3*>(lua_touserdata(L, -4));
										Vec->x = static_cast<float>(lua_tonumber(L, -3));
										Vec->y = static_cast<float>(lua_tonumber(L, -2));
										Vec->z = static_cast<float>(lua_tonumber(L, -1));
									}
									else {
										printf("[Lua][Vector3:SetValue()] Not a Vector3\n");
									}
								}
							}
						}
					}
				}
				else {
					printf("[Lua][Vector3:SetValue(X, Y, Z)] Error: Invalid function arguments\n");
				}
				return 0;
			}

			int Vector3(lua_State* L)
			{
				glm::vec3 Init{};
				//
				if (lua_gettop(L) == 3) {
					if (lua_isnumber(L, -3))
					{
						if (lua_isnumber(L, -2))
						{
							if (lua_isnumber(L, -1))
							{
								Init.x = static_cast<float>(lua_tonumber(L, -3));
								Init.y = static_cast<float>(lua_tonumber(L, -2));
								Init.z = static_cast<float>(lua_tonumber(L, -1));
							}
						}
					}
				}
				//
				size_t DataSize = sizeof(glm::vec3);
				void* Data = lua_newuserdata(L, DataSize);
				memcpy(Data, &Init, DataSize);
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

			void Initialize()
			{

				lua_newtable(state);
				//	-1	(1)	|	metatable
				//
				lua_pushstring(state, "__index");
				//	-2 (1)	|	metatable
				//	-1 (2)	|	'__index'
				//
				lua_pushvalue(state, -2);
				//	-3 (1)	|	metatable
				//	-2 (2)	|	'__index'
				//	-1 (3)	|	metatable copy
				//
				lua_settable(state, -3);
				//	-1 (1)	|	metatable
				//
				lua_pushstring(state, "SetValue");
				lua_pushcfunction(state, SetValue);
				lua_settable(state, -3);
				//	-1	(1)	|	metatable
				//
				lua_pushstring(state, "__tostring");
				lua_pushcfunction(state, ToString);
				lua_settable(state, -3);
				//	-1	(1)	|	metatable
				//
				lua_pushstring(state, "__name");
				lua_pushstring(state, "Vector3");
				lua_settable(state, -3);
				//	-1	(1)	|	metatable
				//
				lua_pushstring(state, "__add");
				lua_pushcfunction(state, Add);
				lua_settable(state, -3);
				//	-1	(1)	|	metatable
				//
				lua_pushstring(state, "__sub");
				lua_pushcfunction(state, Sub);
				lua_settable(state, -3);
				//	-1	(1)	|	metatable
				//
				lua_pushstring(state, "__mul");
				lua_pushcfunction(state, Mul);
				lua_settable(state, -3);
				//	-1	(1)	|	metatable
				//
				lua_pushstring(state, "__div");
				lua_pushcfunction(state, Div);
				lua_settable(state, -3);
				//	-1	(1)	|	metatable
				//
				lua_pushstring(state, "Distance");
				lua_pushcfunction(state, Distance);
				lua_settable(state, -3);
				//	-1	(1)	|	metatable
				//
				lua_setglobal(state, "WorldEngine_Vector3Metatable");
				//	Stack Empty
				//
				lua_pushcfunction(state, Vector3);
				//	-1	(1)	|	Vector3 function
				//
				lua_setglobal(state, "Vector3");
				//	Stack Empty
			}
		}
	}
}