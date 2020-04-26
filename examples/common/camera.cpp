/*
 * Copyright 2013 Dario Manesku. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <bx/timer.h>
#include <bx/math.h>
#include "camera.h"
#include "entry/entry.h"
#include "entry/cmd.h"
#include "entry/input.h"
#include <bx/allocator.h>

int cmdMove(CmdContext* /*_context*/, void* /*_userData*/, int _argc, char const* const* _argv)
{
	if (_argc > 1)
	{
		if (0 == bx::strCmp(_argv[1], "forward") )
		{
			cameraSetKeyState(CAMERA_KEY_FORWARD, true);
			return 0;
		}
		else if (0 == bx::strCmp(_argv[1], "left") )
		{
			cameraSetKeyState(CAMERA_KEY_LEFT, true);
			return 0;
		}
		else if (0 == bx::strCmp(_argv[1], "right") )
		{
			cameraSetKeyState(CAMERA_KEY_RIGHT, true);
			return 0;
		}
		else if (0 == bx::strCmp(_argv[1], "backward") )
		{
			cameraSetKeyState(CAMERA_KEY_BACKWARD, true);
			return 0;
		}
		else if (0 == bx::strCmp(_argv[1], "up") )
		{
			cameraSetKeyState(CAMERA_KEY_UP, true);
			return 0;
		}
		else if (0 == bx::strCmp(_argv[1], "down") )
		{
			cameraSetKeyState(CAMERA_KEY_DOWN, true);
			return 0;
		}
	}

	return 1;
}

static void cmd(const void* _userData)
{
	cmdExec( (const char*)_userData);
}

static const InputBinding s_camBindings[] =
{
	{ entry::Key::KeyW,             entry::Modifier::None, 0, cmd, "move forward"  },
	{ entry::Key::GamepadUp,        entry::Modifier::None, 0, cmd, "move forward"  },
	{ entry::Key::KeyA,             entry::Modifier::None, 0, cmd, "move left"     },
	{ entry::Key::GamepadLeft,      entry::Modifier::None, 0, cmd, "move left"     },
	{ entry::Key::KeyS,             entry::Modifier::None, 0, cmd, "move backward" },
	{ entry::Key::GamepadDown,      entry::Modifier::None, 0, cmd, "move backward" },
	{ entry::Key::KeyD,             entry::Modifier::None, 0, cmd, "move right"    },
	{ entry::Key::GamepadRight,     entry::Modifier::None, 0, cmd, "move right"    },
	{ entry::Key::KeyQ,             entry::Modifier::None, 0, cmd, "move down"     },
	{ entry::Key::GamepadShoulderL, entry::Modifier::None, 0, cmd, "move down"     },
	{ entry::Key::KeyE,             entry::Modifier::None, 0, cmd, "move up"       },
	{ entry::Key::GamepadShoulderR, entry::Modifier::None, 0, cmd, "move up"       },

	INPUT_BINDING_END
};

struct Camera
{
	struct MouseCoords {
		int32_t m_mx;
		int32_t m_my;
	};

	Camera() {
		reset();
		entry::MouseState mouseState;
		update(0.0f, mouseState);

		cmdAdd("move", cmdMove);
		inputAddBindings("camBindings", s_camBindings);
	}

	~Camera() { inputRemoveBindings("camBindings"); }

	void reset() {
		m_mouseNow        = {0, 0};
		m_mouseLast       = {0, 0};
		m_eye             = {0.0f, 0.0f, -35.0f};
		m_at              = {0.0f, 0.0f, -1.0f};
		m_up              = {0.0f, 1.0f, 0.0f};
		m_horizontalAngle = 0.01f;
		m_verticalAngle   = 0.0f;
		m_mouseSpeed      = 0.0020f;
		m_gamepadSpeed    = 0.04f;
		m_moveSpeed       = 30.0f;
		m_keys            = 0;
		m_mouseDown       = false;
	}

	void setKeyState(uint8_t _key, bool _down)
	{
		m_keys &= ~_key;
		m_keys |= _down ? _key : 0;
	}

	void update(float _deltaTime, const entry::MouseState& _mouseState)
	{
		if (!m_mouseDown)
		{
			m_mouseLast.m_mx = _mouseState.m_mx;
			m_mouseLast.m_my = _mouseState.m_my;
		}

		m_mouseDown = !!_mouseState.m_buttons[entry::MouseButton::Right];

		if (m_mouseDown)
		{
			m_mouseNow.m_mx = _mouseState.m_mx;
			m_mouseNow.m_my = _mouseState.m_my;
		}

		if (m_mouseDown)
		{
			int32_t deltaX = m_mouseNow.m_mx - m_mouseLast.m_mx;
			int32_t deltaY = m_mouseNow.m_my - m_mouseLast.m_my;

			m_horizontalAngle += m_mouseSpeed * float(deltaX);
			m_verticalAngle   -= m_mouseSpeed * float(deltaY);

			m_mouseLast.m_mx = m_mouseNow.m_mx;
			m_mouseLast.m_my = m_mouseNow.m_my;
		}

		entry::GamepadHandle handle = { 0 };
		m_horizontalAngle += m_gamepadSpeed * inputGetGamepadAxis(handle, entry::GamepadAxis::RightX)/32768.0f;
		m_verticalAngle   -= m_gamepadSpeed * inputGetGamepadAxis(handle, entry::GamepadAxis::RightY)/32768.0f;
		const int32_t gpx = inputGetGamepadAxis(handle, entry::GamepadAxis::LeftX);
		const int32_t gpy = inputGetGamepadAxis(handle, entry::GamepadAxis::LeftY);
		m_keys |= gpx < -16834 ? CAMERA_KEY_LEFT     : 0;
		m_keys |= gpx >  16834 ? CAMERA_KEY_RIGHT    : 0;
		m_keys |= gpy < -16834 ? CAMERA_KEY_FORWARD  : 0;
		m_keys |= gpy >  16834 ? CAMERA_KEY_BACKWARD : 0;

		const bx::Vec3 direction =
		{
			bx::cos(m_verticalAngle) * bx::sin(m_horizontalAngle),
			bx::sin(m_verticalAngle),
			bx::cos(m_verticalAngle) * bx::cos(m_horizontalAngle),
		};

		const bx::Vec3 right =
		{
			bx::sin(m_horizontalAngle - bx::kPiHalf),
			0,
			bx::cos(m_horizontalAngle - bx::kPiHalf),
		};

		const bx::Vec3 up = bx::cross(right, direction);

		if (m_keys & CAMERA_KEY_FORWARD)
		{
			const bx::Vec3 pos = m_eye;
			const bx::Vec3 tmp = bx::mul(direction, _deltaTime * m_moveSpeed);

			m_eye = bx::add(pos, tmp);
			setKeyState(CAMERA_KEY_FORWARD, false);
		}

		if (m_keys & CAMERA_KEY_BACKWARD)
		{
			const bx::Vec3 pos = m_eye;
			const bx::Vec3 tmp = bx::mul(direction, _deltaTime * m_moveSpeed);

			m_eye = bx::sub(pos, tmp);
			setKeyState(CAMERA_KEY_BACKWARD, false);
		}

		if (m_keys & CAMERA_KEY_LEFT)
		{
			const bx::Vec3 pos = m_eye;
			const bx::Vec3 tmp = bx::mul(right, _deltaTime * m_moveSpeed);

			m_eye = bx::add(pos, tmp);
			setKeyState(CAMERA_KEY_LEFT, false);
		}

		if (m_keys & CAMERA_KEY_RIGHT)
		{
			const bx::Vec3 pos = m_eye;
			const bx::Vec3 tmp = bx::mul(right, _deltaTime * m_moveSpeed);

			m_eye = bx::sub(pos, tmp);
			setKeyState(CAMERA_KEY_RIGHT, false);
		}

		if (m_keys & CAMERA_KEY_UP)
		{
			const bx::Vec3 pos = m_eye;
			const bx::Vec3 tmp = bx::mul(up, _deltaTime * m_moveSpeed);

			m_eye = bx::add(pos, tmp);
			setKeyState(CAMERA_KEY_UP, false);
		}

		if (m_keys & CAMERA_KEY_DOWN)
		{
			const bx::Vec3 pos = m_eye;
			const bx::Vec3 tmp = bx::mul(up, _deltaTime * m_moveSpeed);

			m_eye = bx::sub(pos, tmp);
			setKeyState(CAMERA_KEY_DOWN, false);
		}

		m_at = bx::add(m_eye, direction);
		m_up = bx::cross(right, direction);
	}

	void getViewMtx(float* _viewMtx) {
		bx::mtxLookAt(_viewMtx, bx::load<bx::Vec3>(&m_eye.x), bx::load<bx::Vec3>(&m_at.x), bx::load<bx::Vec3>(&m_up.x) );
	}

	void setPosition(const bx::Vec3& pos) { m_eye = pos; }
	void setAt(const bx::Vec3& at) { m_at = at; }
	void setVerticalAngle(float verticalAngle) { m_verticalAngle = verticalAngle; }
	void setHorizontalAngle(float horizontalAngle) { m_horizontalAngle = horizontalAngle; }

	MouseCoords m_mouseNow;
	MouseCoords m_mouseLast;

	bx::Vec3 m_eye;
	bx::Vec3 m_at;
	bx::Vec3 m_up;
	float m_horizontalAngle;
	float m_verticalAngle;

	float m_mouseSpeed;
	float m_gamepadSpeed;
	float m_moveSpeed;

	uint8_t m_keys;
	bool m_mouseDown;
};

static Camera* s_camera = NULL;

void cameraCreate() { s_camera = BX_NEW(entry::getAllocator(), Camera); }

void cameraDestroy()
{
	BX_DELETE(entry::getAllocator(), s_camera);
	s_camera = NULL;
}

void     cameraSetPosition(const bx::Vec3& pos) { s_camera->setPosition(pos); }
void     cameraSetHorizontalAngle(float horizontal_angle) { s_camera->setHorizontalAngle(horizontal_angle); }
void     cameraSetVerticalAngle(float vertical_angle) { s_camera->setVerticalAngle(vertical_angle); }
void     cameraSetKeyState(uint8_t key, bool down) { s_camera->setKeyState(key, down); }
void     cameraSetAt(const bx::Vec3& at) { s_camera->setAt(at); }
void     cameraGetViewMtx(float* view_mtx) { s_camera->getViewMtx(view_mtx); }
bx::Vec3 cameraGetPosition() { return s_camera->m_eye; }
bx::Vec3 cameraGetAt() { return s_camera->m_at; }

void cameraUpdate(float delta_time, const entry::MouseState& mouse_state) {
	s_camera->update(delta_time, mouse_state);
}
