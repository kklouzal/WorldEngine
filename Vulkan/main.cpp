#ifdef _DEBUG
#include <stdexcept>
#include <iostream>
#else
#define BT_NO_PROFILE 1
#endif

#include "Vulkan.hpp"

int main() {
#ifdef _DEBUG
	std::system("PAUSE");
#endif
	//
	//	VulkanDriver Initialization
	VulkanDriver* app = new VulkanDriver;
	//
	CustomEventReceiver events(app);
	app->setEventReceiver(&events);

	app->_SceneGraph->createTriangleMeshSceneNode("media/cube.fbx");

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
#ifdef _DEBUG
	std::system("PAUSE");
#endif
	return EXIT_SUCCESS;
}