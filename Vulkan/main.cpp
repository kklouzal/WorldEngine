#define WIN32_LEAN_AND_MEAN

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#ifdef _DEBUG
#include <stdexcept>
#include <iostream>
#endif

#include "lodepng.h"

#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Input/Windows.h"

#include "Vulkan.hpp"



int main() {
	std::system("PAUSE");
	//
	//	VulkanDriver Initialization
	VulkanDriver* app = new VulkanDriver;
	//
	CustomEventReceiver events(app);
	app->setEventReceiver(&events);

	app->_SceneGraph->createTriangleMeshSceneNode("media/lua.fbx");

#ifdef _DEBUG
	try {
		//
		//	Loop Main Logic
		app->mainLoop();
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
#else
	//
	//	Loop Main Logic
	//	ToDo: Need access to main loop from main.cpp
	app->mainLoop();
#endif
	//
	//	VulkanDriver deinitializes upon destruction
	delete app;
	std::system("PAUSE");
	return EXIT_SUCCESS;
}