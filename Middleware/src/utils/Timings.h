#ifndef MIDDLEWARE__TIMINGS__H
	#define MIDDLEWARE__TIMINGS__H

	#include <chrono>

	namespace Middleware::Utils {
		
		class Timer {
			public:
				Timer() : m_StartTimePoint(std::chrono::high_resolution_clock::now()) {
				}

				void ResetStartTime() {
					m_StartTimePoint = std::chrono::high_resolution_clock::now();
				}

				double GetSecondsSinceStart() const {
					long long nanosecondsSinceStart = GetNanosecondsSinceStart();
					return (nanosecondsSinceStart * 0.000000001);
				}

				long long GetMillisecondsSinceStart() const {
					long long m_StartTimeSinceEpoch = std::chrono::time_point_cast<std::chrono::milliseconds>(m_StartTimePoint).time_since_epoch().count();

					std::chrono::steady_clock::time_point currentTimePoint = std::chrono::high_resolution_clock::now();
					long long currentTimeSinceEpoch = std::chrono::time_point_cast<std::chrono::milliseconds>(currentTimePoint).time_since_epoch().count();
					
					long long timeSinceStart = (currentTimeSinceEpoch - m_StartTimeSinceEpoch);
					return timeSinceStart;
				}

				long long GetMicrosecondsSinceStart() const {
					long long m_StartTimeSinceEpoch = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimePoint).time_since_epoch().count();

					std::chrono::steady_clock::time_point currentTimePoint = std::chrono::high_resolution_clock::now();
					long long currentTimeSinceEpoch = std::chrono::time_point_cast<std::chrono::microseconds>(currentTimePoint).time_since_epoch().count();

					long long timeSinceStart = (currentTimeSinceEpoch - m_StartTimeSinceEpoch);
					return timeSinceStart;
				}

				long long GetNanosecondsSinceStart() const {
					long long m_StartTimeSinceEpoch = std::chrono::time_point_cast<std::chrono::nanoseconds>(m_StartTimePoint).time_since_epoch().count();

					std::chrono::steady_clock::time_point currentTimePoint = std::chrono::high_resolution_clock::now();
					long long currentTimeSinceEpoch = std::chrono::time_point_cast<std::chrono::nanoseconds>(currentTimePoint).time_since_epoch().count();

					long long timeSinceStart = (currentTimeSinceEpoch - m_StartTimeSinceEpoch);
					return timeSinceStart;
				}

			private:
				std::chrono::steady_clock::time_point m_StartTimePoint;
		};

	}

#endif