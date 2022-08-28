#include "Vulkan.hpp"
//
//	Client Implementation
#include "MyEventReceiver.hpp"
//

int main() {
#ifdef _DEBUG
	//std::system("PAUSE");
#endif
	//
	//	Vulkan Initialization
	WorldEngine::VulkanDriver::Initialize();
	//
	//	Create our custom event receiver (must be done AFTER VulkanDriver Initialize)
	//	Will be cleaned up by WorldEngine::VulkanDriver::Deinitialize()
	CustomEventReceiver events;
	WorldEngine::VulkanDriver::setEventReceiver(&events);

#ifdef _DEBUG
	try {
		//
		//	Force world load for testing purposes if gui malfunctioning
		//WorldEngine::VulkanDriver::_SceneGraph->initWorld();
		//
		//	Loop Main Logic
		WorldEngine::VulkanDriver::mainLoop();
	}
	catch (const std::exception & e) {
		printf("\nFATAL ERROR:\n\t%s\n", e.what());
		return EXIT_FAILURE;
	}
#else
	//
	//	Loop Main Logic
	//	ToDo: Need access to main loop from main.cpp
	WorldEngine::VulkanDriver::mainLoop();
#endif
	//
	//	Vulkan deinitialization
	WorldEngine::VulkanDriver::Deinitialize();
#ifdef _DEBUG
	std::system("PAUSE");
#endif
	return EXIT_SUCCESS;
}