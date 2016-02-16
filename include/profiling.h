#pragma once
#include "common.h"
#include <chrono>

namespace Basilisk
{
	DYNAMIC class Profiler
	{
	public:
		Profiler();// :
			//m_numInvocations(0),
			//m_totalTime(0),
			//m_startTime(0) {}

		void startBlock();
		void endBlock();
	private:
		std::chrono::high_resolution_clock m_clock;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
		std::chrono::duration<double> m_totalTime;
		unsigned long long m_numInvocations;
	};
}