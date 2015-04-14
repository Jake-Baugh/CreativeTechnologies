#include "stdafx.h"
#include "Camera.h"

Camera::Camera()
{

	m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_forward = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMStoreFloat4x4(&m_view, XMMatrixIdentity());
	XMStoreFloat4x4(&m_projection, XMMatrixIdentity());

	m_speed = 20.0f;
	m_deltaSpeed = 0.0f;
}

Camera::~Camera()
{
}

void Camera::Initialize(float fieldOfView, float aspectRatio, float nearPlane, float farPlane)
{
	XMStoreFloat4x4(&m_projection, XMMatrixPerspectiveFovLH(fieldOfView, aspectRatio, nearPlane, farPlane));
}

void Camera::SetPitch(float angle)
{
	XMMATRIX rotationMatrix = XMMatrixRotationAxis(XMLoadFloat3(&m_right), angle);

	XMStoreFloat3(&m_up,	XMVector3TransformNormal(XMLoadFloat3(&m_up),rotationMatrix));
	XMStoreFloat3(&m_forward,	XMVector3TransformNormal(XMLoadFloat3(&m_forward), rotationMatrix));
}

void Camera::SetYaw(float p_angle)
{
	XMMATRIX rotationMatrix = XMMatrixRotationY(p_angle);

	XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), rotationMatrix));
	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), rotationMatrix));
	XMStoreFloat3(&m_forward, XMVector3TransformNormal(XMLoadFloat3(&m_forward), rotationMatrix));
}

void Camera::Update( float dt )
{
	m_deltaSpeed = m_speed * dt;

	RebuildView();
}

void Camera::RebuildView()
{
	// Load Float3s into SIMD-friendly XMVECTORs
	XMVECTOR look = XMLoadFloat3(&m_forward);
	XMVECTOR up = XMLoadFloat3(&m_up);
	XMVECTOR right = XMLoadFloat3(&m_right);
	XMVECTOR position = XMLoadFloat3(&m_position);


	// Keep camera's axes orthogonal to each other and of unit length.
	look = XMVector3Normalize(look);
	up = XMVector3Cross(look, right);
	up = XMVector3Normalize(up);
	right = XMVector3Cross(up, look);
	right = XMVector3Normalize(right);

	// Store the changed vectors back to Float3s
	XMStoreFloat3(&m_forward, look);
	XMStoreFloat3(&m_up, up);
	XMStoreFloat3(&m_right, right);

	// Fill in the GetViewMatrix matrix entries.
	float x = -XMVectorGetX(XMVector3Dot(position, right));
	float y = -XMVectorGetX(XMVector3Dot(position, up));
	float z = -XMVectorGetX(XMVector3Dot(position, look));

	m_view(0,0) = m_right.x; 
	m_view(1,0) = m_right.y; 
	m_view(2,0) = m_right.z; 
	m_view(3,0) = x;   

	m_view(0,1) = m_up.x;
	m_view(1,1) = m_up.y;
	m_view(2,1) = m_up.z;
	m_view(3,1) = y;  

	m_view(0,2) = m_forward.x; 
	m_view(1,2) = m_forward.y; 
	m_view(2,2) = m_forward.z; 
	m_view(3,2) = z;   

	m_view(0,3) = 0.0f;
	m_view(1,3) = 0.0f;
	m_view(2,3) = 0.0f;
	m_view(3,3) = 1.0f;
}

void Camera::Move(CAMERA_MOVEMENT_TYPE cameraMovement)
{
	switch(cameraMovement)
	{
	case MOVE_UP:
		m_position.y += m_deltaSpeed;
		break;

	case MOVE_DOWN:
		m_position.y -= m_deltaSpeed;
		break;

	case MOVE_FORWARD:
		XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) + (m_deltaSpeed * XMLoadFloat3(&m_forward)));
		break;

	case MOVE_BACKWARD:
		XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) - (m_deltaSpeed * XMLoadFloat3(&m_forward)));
		break;

	case STRAFE_LEFT:
		XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) - (m_deltaSpeed * XMLoadFloat3(&m_right)));
		break;

	case STRAFE_RIGHT:
		XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) + (m_deltaSpeed * XMLoadFloat3(&m_right)));
		break;

	default:
		m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		break;
	}
}
