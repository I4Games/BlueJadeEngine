#pragma once
#include "InputInterfaces.h"

class InputManager {
public:
	IKeyboardHandler* m_keyboardHandler;
	IPointerHandler* m_pointerHandler;
	IJoystickHandler* m_joystickHandler;
	IGamepadHandler* m_gamepadHandler;
};