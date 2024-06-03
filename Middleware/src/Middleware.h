#ifndef MIDDLEWARE__MIDDLEWARE__H
	#define MIDDLEWARE__MIDDLEWARE__H

		#include "networking/ClientSocket.h"
		#include "networking/Messaging.h"
		#include "networking/ServerSocket.h"
		#include "networking/Sockets.h"

		#include "utils/Analytics.h"
		#include "utils/FileUtils.h"
		#include "utils/Logging.h"
		#include "utils/Timings.h"

		namespace Middleware {

			inline bool Init() {
				MIDDLEWARE_INFO("About to initialise Middleware...");

				if(!StartSockets()) {
					MIDDLEWARE_FATAL("Middleware failed to initialise. Failed to initialise sockets.");
					return false;
				}

				MIDDLEWARE_INFO("Have successfully finished initialising Middleware...");
				return true;
			}

			inline bool Cleanup() {
				MIDDLEWARE_INFO("About to cleanup Middleware...");

				if(!StopSockets()) {
					MIDDLEWARE_FATAL("Failed to cleanup Middleware. Failed to cleanup sockets.");
					return false;
				}

				MIDDLEWARE_INFO("Have successfully cleaned up Middleware...");
				return true;
			}

		}

#endif