#ifndef MIDDLEWARE__SOCKETS__H
	#define MIDDLEWARE__SOCKETS__H

	#include <ws2tcpip.h>
	#include <WinSock2.h>
	#pragma comment(lib, "ws2_32.lib")

	#include "../utils/Logging.h"

	namespace Middleware {

		inline bool StartSockets() {
			MIDDLEWARE_TRACE("Starting socket networking");

			WSADATA windowsSocketData;
			if(WSAStartup(MAKEWORD(2, 2), &windowsSocketData) != 0) {
				MIDDLEWARE_ERROR("Failed to start WinSock");
				return false;
			}

			return true;
		}

		inline bool StopSockets() {
			MIDDLEWARE_TRACE("Stopping socket networking");

			if(WSACleanup() != 0) {
				MIDDLEWARE_ERROR("Failed to stop WinSock");
				return false;
			}

			return true;
		}

	}

#endif