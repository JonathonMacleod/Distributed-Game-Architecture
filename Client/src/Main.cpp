#include "Middleware.h"

#include "Application.h"
#include "graphics/Graphics.h"



//
// Main function
//

int main(int argc, char** args) {
	//
	// Initialisation
	//

	if(!Middleware::Init()) {
		CLIENT_FATAL("Failed to initialise Middleware");
		return -1;
	}

	if(!glfwInit()) {
		CLIENT_FATAL("Failed to initialise GLFW");
		return -1;
	}


	//
	// Create an Application instance and start running it
	//

	Client::Application* application = Client::Application::GetInstance();
	if(application->Init(argc, args))
		application->Start();


	//
	// Cleanup
	//

	CLIENT_INFO("About to terminate GLFW");
	glfwTerminate();

	if(!Middleware::Cleanup()) {
		CLIENT_FATAL("Failed to cleanup Middleware");
		return -1;
	}

	return 0;
}