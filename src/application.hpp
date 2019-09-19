#pragma once

#include "common.hpp"

class Application {
	public:
		Application();

		void pollEvents();

		~Application();
	private:
		NULL_COPY_AND_ASSIGN(Application);

		static uint32 numInstances;
};
