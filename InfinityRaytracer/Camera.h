#ifndef CAMERA_H
#define CAMERA_H

#include "stdafx.h"
using namespace DirectX;

enum CAMERA_MOVEMENT_TYPE
{
	MOVE_UP,
	MOVE_DOWN,
	MOVE_FORWARD,
	MOVE_BACKWARD,
	STRAFE_LEFT,
	STRAFE_RIGHT
};

class Camera
{
public:
	Camera();
	~Camera();

	XMFLOAT3 GetPosition() { return m_position; }
	XMFLOAT3 GetForward() { return m_forward; }
	XMFLOAT3 GetRight() { return m_right; }
	float GetSpeed() { return m_speed; }

	const XMFLOAT4X4 GetViewMatrix() { return m_view; }
	const XMFLOAT4X4 GetProjMatrix() { return m_projection; }

	void Initialize(float fieldOfView, float aspectRatio, float nearPlane, float farPlane);

	void Move(CAMERA_MOVEMENT_TYPE cameraMovement);

	void SetYPosition(float y) { m_position.y = y;}
	void SetPosition(float x, float y, float z) { m_position = XMFLOAT3(x, y, z); }

	void SetPitch(float angle);
	void SetYaw(float angle);

	void SetCamSpeed(float speed);

	void Update(float dt);

private:
	void RebuildView();

	XMFLOAT3 m_position;
	XMFLOAT3 m_right;
	XMFLOAT3 m_up;
	XMFLOAT3 m_forward;

	XMFLOAT4X4 m_view;
	XMFLOAT4X4 m_projection;

	float m_speed;
	float m_deltaSpeed;
};
#endif 