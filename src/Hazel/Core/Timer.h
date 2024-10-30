#pragma once

#include "chrono"

namespace Hazel {
	
#define PROFILE_SCOPE(name) Timer timer##__LINE__(name, [&](Hazel::ProfileResult profileResult){ m_ProfileResults.push_back(profileResult); })


	template<class Fn>
	class Timer {
	public:
		Timer(const char* name, Fn&& function) 
			: m_Stopped(false), m_Name(name), func(function)
		{
			m_StartTimepoint = std::chrono::high_resolution_clock::now();
		}
		~Timer() {
			if (!m_Stopped)
				Stop();
		}
		void Stop() {
			auto endTimepoint = std::chrono::high_resolution_clock::now();

			long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
			long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

			m_Stopped = true;

			float duration = (end - start) * 0.001f;
			func({ m_Name, duration });
		}

	private:
		const char* m_Name;
		std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
		bool m_Stopped;
		Fn func;
	};

	struct ProfileResult{
		const char* name;
		float time;
	};



}