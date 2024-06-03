#include "Middleware.h"
#include "Application.h"

int main(int argc, char** args) {
	//
	// Initialisation
	//

	if(!Middleware::Init()) {
		SERVER_FATAL("Failed to initialise Middleware");
		return -1;
	}

	if(glfwInit() != GLFW_TRUE) {
		SERVER_FATAL("Failed to initialise GLFW");
		return -1;
	}


	//
	// Create an Application instance and start running it
	//

	SERVER_INFO("About to create application");
	Server::Application* app = Server::Application::GetInstance();
	SERVER_INFO("About to initialise application");
	if(app->Init(argc, args)) {
		SERVER_INFO("About to start application");
		app->Start();
	} else {
		SERVER_INFO("Failed to initialise application");
	}


	//
	// Cleanup
	//

	glfwTerminate();

	if(!Middleware::Cleanup()) {
		SERVER_FATAL("Failed to cleanup Middleware");
		return -1;
	}

	return 0;
}