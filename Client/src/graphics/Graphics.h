#ifndef SERVER__GRAPHICS__H
	#define SERVER__GRAPHICS__H
	
	#include <GL/glew.h>
	#define GLFW_INCLUDE_NONE
	#include <GLFW/glfw3.h>

	namespace Client::Graphics {
		
		bool InitGraphics();
		bool CleanupGraphics();

	}

#endif