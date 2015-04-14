#ifndef APPLICATION_H
#define APPLICATION_H

#include "Direct3D.h"
#include "Timer.h"
#include "ComputeBuffer.h"
#include "ComputeShader.h"
#include "Compute.h"
#include "ComputeTexture.h"
#include "Camera.h"
#include "ObjLoader.h"

#include <AntTweakBar.h>

using namespace DirectX;

struct Sphere
{
	XMFLOAT3 spherePos;
	float sphereRadius;	
	XMFLOAT4 sphereColor;
};

struct Matrix
{
	Matrix(){}
	Matrix(XMFLOAT4X4 view, XMFLOAT4X4 proj, XMFLOAT4 cameraPosition)
	{
		viewMat = view;
		projMat = proj;
		camPos	= cameraPosition;
	}
	XMFLOAT4X4 viewMat;
	XMFLOAT4X4 projMat;
	XMFLOAT4 camPos;
};

struct Tri
{
	Tri(XMFLOAT4 t1, XMFLOAT4 t2, XMFLOAT4 t3, XMFLOAT4 col)
	{
		p1 = t1;
		p2 = t2;
		p3 = t2;
		color = col;
	}
	XMFLOAT4 p1, p2, p3, color;
};

struct PointLight
{
	PointLight(){}
	PointLight(XMFLOAT4 pos, XMFLOAT4 amb, XMFLOAT4 dif, XMFLOAT4 spec, XMFLOAT4 att)
	{
		position = pos;
		ambient	= amb;
		diffuse	= dif;
		specular = spec;
		attenuation	= att;
	}

	XMFLOAT4 position, ambient, diffuse, specular, attenuation;
};

struct Ray
{
	//Ray direction
	XMFLOAT4 dir;	
	//Ray origin
	XMFLOAT4 origin;
};

struct Hit
{
	XMFLOAT4 position;
	//ray hit == true, ray miss == false
	bool isHit;
	//Hit distance
	float distance;

	//Colour
	//float3 colour

	//Hit UV
	XMFLOAT2 uv;
	//Hit triangle ID
	int triangleID;
};

struct Global
{
	int width;
	int height;
	int pointlights;
	int vertices;
};
class Application
{
public:

	Application();
	~Application();

	HRESULT Initialize(Direct3D* direct3D, Timer* timer);
	HRESULT Render();
	HRESULT Update(float dt);

	//Setters
	void SetBounces(int value) { m_bounceCount = value; }
	void SetLights(int value) { m_global.pointlights = value; UpdateBuffer(m_globalBuffer, &m_global, sizeof(Global)); }

	void SetThreadGroupSize(int x, int y);

	//Getters
	double RayTimer();
	double IntersectionTimer();
	double ColourTimer();

private:

	void UpdateInput();
	void CreateBuffer();
	void UpdateBuffer(ID3D11Buffer* buffer, void* data, unsigned size);
	void PrintTitle();	
	
	void SetResolution(int width, int height);
	void InitializeGUI( Direct3D* direct3D);
	void RenderGUI();

private:
	Direct3D* m_direct3D;
	Timer* m_timer;
	Compute* m_compute;

	ComputeShader* m_rayCS;
	ComputeShader* m_intersectionCS;
	ComputeShader* m_colorCS;

	ComputeBuffer* m_rayBuffer;
	ComputeBuffer* m_HitBuffer;
	ComputeBuffer* m_colorBuffer;
	ComputeBuffer* m_sphereBuffer;

	Camera*	m_camera;
	ObjLoader* m_modelLoader;

	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_AABBBuffer;
	ID3D11Buffer* m_loopBuffer;
	ID3D11Buffer* m_globalBuffer;

	std::vector<Sphere>	m_sphere;
	Matrix m_mdata;
	Model* m_model;
	PointLight m_pointLights[10];
	Global m_global;
	XMFLOAT2 m_oldMousePos;

	int m_value;
	float m_ftimer;

	int m_bounceCount;
	float lightSpeed;
	
	bool movePointlight;

	float m_lightPositionY;
	XMFLOAT4 m_lightAmbient;
	XMFLOAT4 m_lightDiffuse;
	XMFLOAT4 m_lightSpecular;
	XMFLOAT4 m_lightAttenuation;

	int m_threadsPerGroupX;
	int m_threadsPerGroupY;

	int m_threadGroupX;
	int m_threadGroupY;

	double m_rayCreationTime, m_intersectionTime, m_colourTime;
	
	TwBar* m_gui;
};
#endif
