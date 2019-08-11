#pragma once

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

class Display {
	public:
		Display(const char* title, int width, int height);

		bool isCloseRequested() const;
		void pollEvents();
		void render();

		inline int getWidth() const { return width; }
		inline int getHeight() const { return height; }

		inline GLFWwindow* getWindow() { return window; }

		~Display();
	private:
		GLFWwindow* window;
		int width;
		int height;

		static int numInstances;
};
