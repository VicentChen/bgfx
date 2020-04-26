/*
 * Copyright 2013 Dario Manesku. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#ifndef CAMERA_H_HEADER_GUARD
#define CAMERA_H_HEADER_GUARD

#include "entry/entry.h"

#define CAMERA_KEY_FORWARD   UINT8_C(0x01)
#define CAMERA_KEY_BACKWARD  UINT8_C(0x02)
#define CAMERA_KEY_LEFT      UINT8_C(0x04)
#define CAMERA_KEY_RIGHT     UINT8_C(0x08)
#define CAMERA_KEY_UP        UINT8_C(0x10)
#define CAMERA_KEY_DOWN      UINT8_C(0x20)

///
void cameraCreate();

///
void cameraDestroy();

///
void cameraSetPosition(const bx::Vec3& pos);

///
void cameraSetHorizontalAngle(float horizontal_angle);

///
void cameraSetVerticalAngle(float vertical_angle);

///
void cameraSetKeyState(uint8_t key, bool down);

///
void cameraSetAt(const bx::Vec3& at);

///
void cameraGetViewMtx(float* view_mtx);

///
bx::Vec3 cameraGetPosition();

///
bx::Vec3 cameraGetAt();

///
void cameraUpdate(float delta_time, const entry::MouseState& mouse_state);

#endif // CAMERA_H_HEADER_GUARD
