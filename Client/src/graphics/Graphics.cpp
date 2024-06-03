#include "Graphics.h"

#include "Middleware.h"

namespace Client::Graphics {

	bool InitGraphics() {
		CLIENT_INFO("About to initialise graphics");

		//TODO: When the temporary window output is no longer needed, move GLFW initialisation and window handling to here

		if(glfwInit()) {
			CLIENT_TRACE("Successfully initialised GLFW");
		} else {
			CLIENT_ERROR("Failed to initialise GLFW");
		}

		CLIENT_INFO("Have successfully initialised graphics");
		return true;
	}

	bool CleanupGraphics() {
		CLIENT_INFO("About to cleanup graphics");

		//TODO: When GLFW initialisation and window handling is moved to this source file, handle window destruction, context destruction, etc, here

		glfwTerminate();

		CLIENT_INFO("Have successfully cleaned up graphics");
		return true;
	}

}
