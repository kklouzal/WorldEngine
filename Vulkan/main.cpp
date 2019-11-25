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

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Gwen/Gwen.h"
#include "Gwen/Skins/Simple.h"
#include "Gwen/Skins/TexturedBase.h"
#include "Gwen/UnitTest/UnitTest.h"
#include "Gwen/Input/Windows.h"

#include "Vulkan.hpp"



int main() {
	//
	//	VulkanDriver Initialization
	VulkanDriver app;
	//
	CustomEventReceiver events(&app);
	app.setEventReceiver(&events);


#ifdef _DEBUG
	try {
		//
		//	Loop Main Logic
		app.mainLoop();
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
#else
	//
	//	Loop Main Logic
	//	ToDo: Need access to main loop from main.cpp
	app.mainLoop();
#endif
	//
	//	VulkanDriver deinitializes upon destruction
	return EXIT_SUCCESS;
}