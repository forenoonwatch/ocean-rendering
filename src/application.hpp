#pragma once

#include "common.hpp"
#include "monitor.hpp"

class Application {
	public:
		Application();

		void pollEvents();

		inline Monitor& getPrimaryMonitor() { return monitors[0]; }
		inline Monitor& getMonitor(uint32 i) { return monitors[i]; }

		~Application();
	private:
		NULL_COPY_AND_ASSIGN(Application);

		Monitor* monitors;

		static uint32 numInstances;
};
