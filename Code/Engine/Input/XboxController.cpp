#include "Engine/Input/XboxController.hpp"
#include "Engine/Math/MathUtils.hpp"
#include <Windows.h>
#include <Xinput.h>
#pragma comment( lib, "xinput" ) 

XboxController::XboxController()
{
	
}

XboxController::~XboxController()
{
}

bool XboxController::IsConnected() const
{
	return m_isConnected;
}

int XboxController::GetControllerID() const
{
	return m_id;
}

AnalogJoystick const& XboxController::GetLeftStick() const
{
	return m_leftStick;
}

AnalogJoystick const& XboxController::GetRightStick() const
{
	return m_rightStick;
}

float XboxController::GetLeftTrigger() const
{
	return m_leftTrigger;
}

float XboxController::GetRightTrigger() const
{
	return m_rightTrigger;
}

KeyButtonState const& XboxController::GetButton(XboxButtonID buttonID) const
{
	return m_buttons[buttonID];
}

bool XboxController::IsButtonDown(XboxButtonID buttonID) const
{
	return m_buttons[buttonID].isKeyDown;
}

bool XboxController::WasButtonJustPressed(XboxButtonID buttonID) const
{
	return !m_buttons[buttonID].wasKeyJustPressed && m_buttons[buttonID].isKeyDown;
}

bool XboxController::WasButtonJustReleased(XboxButtonID buttonID) const
{
	return !m_buttons[buttonID].isKeyDown && !m_buttons[buttonID].wasKeyJustPressed;
}

void XboxController::Update()
{
	XINPUT_STATE xboxControllerState;
	memset(&xboxControllerState, 0, sizeof(xboxControllerState));

	DWORD errorStatus = XInputGetState(m_id, &xboxControllerState);
	if (errorStatus != ERROR_SUCCESS)
	{
		Reset();
		m_isConnected = false;
		return;
	}
	m_isConnected = true;
	XINPUT_GAMEPAD const& state = xboxControllerState.Gamepad;
	
	UpdateJoystick(m_leftStick, state.sThumbLX, state.sThumbLY);
	UpdateJoystick(m_rightStick, state.sThumbRX, state.sThumbRY);

	UpdateTrigger(m_leftTrigger, state.bLeftTrigger);
	UpdateTrigger(m_rightTrigger, state.bRightTrigger);

	UpdateButton(XBOX_BUTTON_A, state.wButtons, XINPUT_GAMEPAD_A);
	UpdateButton(XBOX_BUTTON_B, state.wButtons, XINPUT_GAMEPAD_B);
	UpdateButton(XBOX_BUTTON_X, state.wButtons, XINPUT_GAMEPAD_X);
	UpdateButton(XBOX_BUTTON_Y, state.wButtons, XINPUT_GAMEPAD_Y);
	UpdateButton(XBOX_BUTTON_LB, state.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
	UpdateButton(XBOX_BUTTON_RB, state.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);
	UpdateButton(XBOX_BUTTON_BACK, state.wButtons, XINPUT_GAMEPAD_BACK);
	UpdateButton(XBOX_BUTTON_START, state.wButtons, XINPUT_GAMEPAD_START);
	UpdateButton(XBOX_BUTTON_LS, state.wButtons, XINPUT_GAMEPAD_LEFT_THUMB);
	UpdateButton(XBOX_BUTTON_RS, state.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB);

	UpdateButton(XBOX_BUTTON_DPAD_UP, state.wButtons, XINPUT_GAMEPAD_DPAD_UP);
	UpdateButton(XBOX_BUTTON_DPAD_DOWN, state.wButtons, XINPUT_GAMEPAD_DPAD_DOWN);
	UpdateButton(XBOX_BUTTON_DPAD_LEFT, state.wButtons, XINPUT_GAMEPAD_DPAD_LEFT);
	UpdateButton(XBOX_BUTTON_DPAD_RIGHT, state.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT);
}

void XboxController::Reset()
{
	m_isConnected = false;
	m_leftTrigger = 0.f;
	m_rightTrigger = 0.f;

	for (int i = 0; i < NUM_XBOX_BUTTONS; ++i) {
		m_buttons[i] = KeyButtonState();
	}

	m_leftStick.Reset();
	m_rightStick.Reset();
}


void XboxController::UpdateJoystick(AnalogJoystick& out_joystick, short rawX, short rawY)
{
	float normalizedX = RangeMapClamped((float)rawX, XBOX_JOYSTICK_MIN_VALUE, XBOX_JOYSTICK_MAX_VALUE, -1.0f, 1.0f);
	float normalizedY = RangeMapClamped((float)rawY, XBOX_JOYSTICK_MIN_VALUE, XBOX_JOYSTICK_MAX_VALUE, -1.0f, 1.0f);
	out_joystick.UpdatePosition(normalizedX, normalizedY);
}

void XboxController::UpdateTrigger(float& out_triggerValue, unsigned char rawValue)
{
	out_triggerValue = RangeMapClamped((float)rawValue, 0.f, 255.f, 0.f, 1.f);
}

void XboxController::UpdateButton(XboxButtonID buttonID, unsigned short buttonFlags, unsigned short buttonFlag)
{
	m_buttons[buttonID].wasKeyJustPressed = m_buttons[buttonID].isKeyDown;
	m_buttons[buttonID].isKeyDown = (buttonFlags & buttonFlag) == buttonFlag;
}
