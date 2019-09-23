#pragma once

#include "input.hpp"
#include "window.hpp"

class Application {
	public:
		static void init();

		static void pollEvents();

		static bool isKeyDown(uint32 keyCode);
		static bool getKeyPressed(uint32 keyCode);
		static bool getKeyReleased(uint32 keyCode);

		static bool isMouseDown(uint32 mouseButton);
		static bool getMousePressed(uint32 mouseButton);
		static bool getMouseReleased(uint32 mouseButton);

		inline static Monitor& getPrimaryMonitor() { return monitors[0]; }
		inline static Monitor& getMonitor(uint32 i) { return monitors[i]; }

		inline static double getMouseX() { return mouseX; }
		inline static double getMouseY() { return mouseY; }

		static void destroy();
	protected:
		static void bindInputCallbacks(WindowHandle windowHandle);

		friend class Window;
	private:
		NULL_COPY_AND_ASSIGN(Application);
		Application() = delete;

		static Monitor* monitors;

		static bool keys[Input::KEY_LAST + 1];
		static bool mouseButtons[Input::MOUSE_BUTTON_LAST + 1];

		static bool lastKeys[Input::KEY_LAST + 1];
		static bool lastMouseButtons[Input::MOUSE_BUTTON_LAST + 1];

		static double mouseX;
		static double mouseY;

		static void onKeyEvent(WindowHandle, int, int, int, int);
		static void onMouseClickEvent(WindowHandle, int, int, int);
		static void onMouseMoveEvent(WindowHandle, double, double);
};
