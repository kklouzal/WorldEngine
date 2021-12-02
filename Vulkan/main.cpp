#ifndef _DEBUG
#define BT_NO_PROFILE 1
#endif

#include "Vulkan.hpp"
//
//	Client Implementations
#include "MyEventReceiver.hpp"
//

int main() {
#ifdef _DEBUG
	std::system("PAUSE");
#endif
	//
	//	VulkanDriver Initialization
	VulkanDriver* app = new VulkanDriver;
	//
	//	Create our custom event receiver
	//	Will be cleaned up by VulkanDriver
	CustomEventReceiver* events = new CustomEventReceiver(app);
	app->setEventReceiver(events);

#ifdef _DEBUG
	try {
		//
		//	Loop Main Logic
		app->mainLoop();
	}
	catch (const std::exception & e) {
		printf("\nFATAL ERROR:\n\t%s\n", e.what());
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