#include "stdafx.h"
#include "Application.h"

Application::Application()
{
}


Application::~Application()
{
}

HRESULT Application::Initialize( Direct3D* direct3D, Timer* timer)
{

	m_direct3D = direct3D;
	m_timer	= timer;
	m_oldMousePos = m_direct3D->GetMousePos();
	m_ftimer = 0;
	m_bounceCount = 1;
	m_value = 1;
	lightSpeed = 10.0f;

	m_camera = new Camera();
	m_camera->Initialize(XM_PI / 4, (float)m_direct3D->GetWindowWidth()/(float)m_direct3D->GetWindowHeight(), 0.1f, 1000.0f);
	m_camera->SetPosition(0.0f, 3.0f,-10.0f);
	m_camera->SetPitch(0.3f);
	
	//m_sphere.resize(2);
	//m_sphere[0].spherePos = XMFLOAT3(0.0f, 0.0f, 10.0f);
	//m_sphere[0].sphereRadius = 5.0f;
	//m_sphere[0].sphereColor = XMFLOAT4(0.4f, 0.6f, 1.0f, 1.0f);

	//m_sphere[1].spherePos = XMFLOAT3(0.0f, 0.0f, 2.0f);
	//m_sphere[1].sphereRadius = 1.0f;
	//m_sphere[1].sphereColor = XMFLOAT4(0.2f, 1.0f, 0.0f, 1.0f);

	//----------------------------------------------w--
	//	Set up each pointlight through constructor
	//------------------------------------------------

	m_lightAmbient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_lightDiffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_lightSpecular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_lightAttenuation = XMFLOAT4(0.0f, 1.0f, 0.0f, 10.0f);

	for(int i = 0; i < 10; i++)
	{
		m_pointLights[i] = PointLight(XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), m_lightAmbient, m_lightDiffuse, m_lightSpecular, m_lightAttenuation);
	}

	m_compute = new Compute(m_direct3D->GetDevice(), m_direct3D->GetDeviceContext());
	m_modelLoader = new ObjLoader(m_compute, m_direct3D->GetDevice());

	CreateBuffer();

	InitializeGUI(m_direct3D);

	m_global.pointlights = m_value;
	m_global.vertices = 800;
	m_global.width = m_direct3D->GetWindowWidth();
	m_global.height = m_direct3D->GetWindowWidth();

	SetThreadGroupSize(16,16);
	UpdateInput();

	return S_OK;
}

HRESULT Application::Render()
{
	m_rayCreationTime = m_intersectionTime = m_colourTime = 0.0f;

	ID3D11UnorderedAccessView* clearUAV[] = { 0,0,0,0,0,0,0 };
	ID3D11UnorderedAccessView* ray[] = { m_rayBuffer->GetUnorderedAccessView(), m_direct3D->GetUnorderedAccessView() };
	ID3D11UnorderedAccessView* intersection[] = { m_rayBuffer->GetUnorderedAccessView(), m_HitBuffer->GetUnorderedAccessView()};
	ID3D11UnorderedAccessView* uav[] = { m_direct3D->GetUnorderedAccessView(), m_colorBuffer->GetUnorderedAccessView(), m_HitBuffer->GetUnorderedAccessView() };

	m_direct3D->GetDeviceContext()->CSSetConstantBuffers(0, 1, &m_cameraBuffer);
	m_direct3D->GetDeviceContext()->CSSetConstantBuffers(1, 1, &m_lightBuffer);
	m_direct3D->GetDeviceContext()->CSSetConstantBuffers(2, 1, &m_model->vertexBuffer);
	m_direct3D->GetDeviceContext()->CSSetConstantBuffers(3, 1, &m_AABBBuffer);
	m_direct3D->GetDeviceContext()->CSSetConstantBuffers(4, 1, &m_loopBuffer);
	m_direct3D->GetDeviceContext()->CSSetConstantBuffers(5, 1, &m_globalBuffer); 

	m_direct3D->GetDeviceContext()->CSSetShaderResources(0, 1, &m_model->material->m_textureResource);

	//-------------------------
	//	Ray generation stage
	//-------------------------

	m_direct3D->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, ray, NULL);

	m_rayCS->Set();
	m_timer->Start();
	m_direct3D->GetDeviceContext()->Dispatch( m_threadGroupX, m_threadGroupY, 1 );
	m_timer->Stop();
	m_rayCS->Unset();

	m_rayCreationTime = m_timer->GetTime();

	m_direct3D->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, clearUAV, NULL);

 	for(int i = 0; i < m_bounceCount; ++i)
	{
		UpdateBuffer(m_loopBuffer, &i, sizeof(int));

		//---------------------
		//	Intersection Stage
		//---------------------

		m_direct3D->GetDeviceContext()->CSSetUnorderedAccessViews(0, 2, intersection, NULL);

		m_intersectionCS->Set();
		m_timer->Start();
		m_direct3D->GetDeviceContext()->Dispatch( m_threadGroupX, m_threadGroupY, 1 );
		m_timer->Stop();
		m_intersectionCS->Unset();

		m_intersectionTime += m_timer->GetTime();

		m_direct3D->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, clearUAV, NULL);

		//---------------
		//	Color Stage
		//---------------

		m_direct3D->GetDeviceContext()->CSSetUnorderedAccessViews(0, 3, uav, NULL);

		m_colorCS->Set();
		m_timer->Start();
		m_direct3D->GetDeviceContext()->Dispatch( m_threadGroupX, m_threadGroupY, 1 );
		m_timer->Stop();
		m_colorCS->Unset();

		m_colourTime += m_timer->GetTime();
		m_direct3D->GetDeviceContext()->CSSetUnorderedAccessViews(0, 1, clearUAV, NULL);
	
	}
	
	ID3D11RenderTargetView* rentar = m_direct3D->GetRenderTarget();
	m_direct3D->GetDeviceContext()->OMSetRenderTargets(1, &rentar, m_direct3D->GetDepthBuffer());

	RenderGUI();

	if(FAILED(m_direct3D->GetSwapChain()->Present( 0, 0 )))
		return E_FAIL;

	PrintTitle();

	return S_OK;
}

HRESULT Application::Update( float dt )
{
	m_ftimer += dt;

	UpdateInput();

	m_camera->Update(dt);

	//-------------------------------------------------------------
	//	Set inverse camera matrices to project viewport correctly
	//-------------------------------------------------------------
	
	XMStoreFloat4x4(&m_mdata.viewMat, XMMatrixInverse(0, XMLoadFloat4x4(&m_camera->GetViewMatrix())));
	XMStoreFloat4x4(&m_mdata.projMat, XMMatrixInverse(0, XMLoadFloat4x4(&m_camera->GetProjMatrix())));
	m_mdata.camPos	= XMFLOAT4(m_camera->GetPosition().x,m_camera->GetPosition().y, m_camera->GetPosition().z, 0.0f);

	//----------------------------------------------
	// Distribute each point light around the scene 
	//----------------------------------------------

	if (movePointlight)
	{
		for(int i = 0; i < 10; i++)
		{
			m_pointLights[i].position = XMFLOAT4(sin(XM_PI * m_ftimer / lightSpeed )*(i+1) * 2.0f, 1.0f, cos(XM_PI * m_ftimer / lightSpeed )*(i+1) * 2.0f, 4.0f);
			//m_pointLights[1].pos = XMFLOAT4(3.0f, 0.0f, cos(XM_PI*3.0f), 4.0f);
		}
	}

	//-----------------------------
	//	Update the camera buffer
	//-----------------------------

	UpdateBuffer(m_cameraBuffer, &m_mdata, sizeof(Matrix));

	//------------------------------------------------
	//	Update the light buffer for each in array
	//------------------------------------------------

	for (int i = 0; i < 10; i++)
	{
		m_pointLights[i].ambient = m_lightAmbient;
		m_pointLights[i].diffuse = m_lightDiffuse;
		m_pointLights[i].specular = m_lightSpecular;
		m_pointLights[i].attenuation = m_lightAttenuation;
	}

	UpdateBuffer(m_lightBuffer, &m_pointLights[0], (sizeof(PointLight) * 10));
	//-------------------------------------------------------------------
	//	Update global buffer for GUI to sync and update amount of Lights
	//-------------------------------------------------------------------

	for (int i = 0; i < 10; i++)
	{
		m_global.pointlights = m_value;
		UpdateBuffer(m_globalBuffer, &m_global, sizeof(Global));
	}

	return S_OK;
}

void Application::UpdateInput()
{
	if(m_direct3D->GetLeftMousePressed())
	{
		float mx = m_direct3D->GetMousePos().x;
		float my = m_direct3D->GetMousePos().y;

		POINT p;
		p.x = (LONG)m_direct3D->GetWindowWidth()*0.5;
		p.y = (LONG)m_direct3D->GetWindowHeight()*0.5;

		m_camera->SetPitch((my - p.y) * 0.00087266f);
		m_camera->SetYaw((mx - p.x) * 0.00087266f);

		ClientToScreen(m_direct3D->GetMainWindowHandle(), &p);
		SetCursorPos(p.x, p.y);	
	}
	//----------------------------
	// Input for camera movement
	//----------------------------

	if(GetAsyncKeyState('W') & 0x8000)
		m_camera->Move(MOVE_FORWARD);
	if(GetAsyncKeyState('S') & 0x8000)
		m_camera->Move(MOVE_BACKWARD);
	if(GetAsyncKeyState('A') & 0x8000)
		m_camera->Move(STRAFE_LEFT);
	if(GetAsyncKeyState('D') & 0x8000)
		m_camera->Move(STRAFE_RIGHT);
	if(GetAsyncKeyState('X') & 0x8000)
		m_camera->Move(MOVE_DOWN);
	if(GetAsyncKeyState('Z') & 0x8000)
		m_camera->Move(MOVE_UP);

	/* Early Input

	//for(int i = 1; i < 10; ++i)
	//{
		//if(GetAsyncKeyState(i + 48) & 0x8000)
		//{
			//------------------------------------------------------------
			// Hold the associated key and press a number on the keypad 
			// to register the amount of bounces or lights in the scene
			//------------------------------------------------------------

			//if(GetAsyncKeyState(VK_LEFT) & 0x8000)
			//	SetBounces(i); 	
			//if(GetAsyncKeyState(VK_RIGHT) & 0x8000)
			//	SetLights(i);

			//UpdateBuffer(m_globalBuffer, &m_global, sizeof(Global));
		//}
	//} */

}

void Application::CreateBuffer()
{
	//---------------------------------------------
	//	Create dynamic buffer for camera matrices
	//---------------------------------------------

	m_cameraBuffer = m_compute->CreateDynamicBuffer(sizeof(Matrix), 0); 
	
	//----------------------
	//	Create light buffer
	//----------------------

	m_lightBuffer = m_compute->CreateDynamicBuffer(sizeof(PointLight) * 10, &m_pointLights[0]);
	
	//--------------------------------
	//	Load a model into the scene
	//--------------------------------

	//	TO SEE AFFECT ON DIFFERENT MODELS CHANGE MODEL TO 
	//
	//	->AddStaticModel("bunny.obj", "model");
	//	->AddStaticModel("bunnyRoom.obj", "model");
	//	->AddStaticModel("bunnyRoom2.obj", "model");
	//	->AddStaticModel("treeModel.obj", "model");
	//	->AddStaticModel("treeModel2.obj", "model");
	
		m_model	= m_modelLoader->AddStaticModel("bunny.obj", "model");

	//-------------------------------------------
	//	Create a constant buffer for model AABB
	//-------------------------------------------

	m_AABBBuffer = m_compute->CreateConstantBuffer(sizeof(AABB), &AABB(m_model->bottomBoundingCorner, m_model->topBoundingCorner));

	//-------------------------------------------------------------------------
	//	Structured buffers for generating rays, calculating intersection data 
	//	and producing final output colour
	//-------------------------------------------------------------------------

	m_rayBuffer	= m_compute->CreateBuffer(STRUCTURED_BUFFER, sizeof(Ray),m_direct3D->GetWindowHeight() * m_direct3D->GetWindowWidth(), true, true, NULL, true);
	m_HitBuffer = m_compute->CreateBuffer(STRUCTURED_BUFFER, sizeof(Hit),m_direct3D->GetWindowHeight() * m_direct3D->GetWindowWidth(), true, true, NULL, true);
	m_colorBuffer = m_compute->CreateBuffer(STRUCTURED_BUFFER, sizeof(XMFLOAT4),m_direct3D->GetWindowHeight() * m_direct3D->GetWindowWidth(), true, true, NULL, true);

	m_loopBuffer = m_compute->CreateDynamicBuffer(sizeof(int), 0); 
	m_globalBuffer= m_compute->CreateDynamicBuffer(sizeof(Global), &m_global); 
}

void Application::UpdateBuffer( ID3D11Buffer* buffer, void* data, unsigned size )
{
	void* verticesPtr;

	D3D11_MAPPED_SUBRESOURCE l_mapped;
	m_direct3D->GetDeviceContext()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &l_mapped);

	//--------------------------------------------------
	// Get a pointer to the data in the vertex buffer.
	//--------------------------------------------------

	verticesPtr = (void*)l_mapped.pData;

	//--------------------------------------
	// Copy the data into the vertex buffer.
	//--------------------------------------

	memcpy(verticesPtr, data, size);

	//----------------------------
	// Unlock the vertex buffer.
	//----------------------------

	m_direct3D->GetDeviceContext()->Unmap(buffer, 0);
}

void Application::SetResolution( int width, int height )
{
	m_global.width = width;
	m_global.height = height;

	m_threadGroupX = m_global.width / m_threadsPerGroupX;
	m_threadGroupY = m_global.height / m_threadsPerGroupY;

	UpdateBuffer(m_globalBuffer, &m_global, sizeof(Global));
}

double Application::RayTimer()
{
	return m_rayCreationTime;
}

double Application::IntersectionTimer()
{
	return m_intersectionTime;
}

double Application::ColourTimer()
{
	return m_colourTime;
}

void Application::PrintTitle()
{
	char title[256];
	sprintf_s(
		title,
		sizeof(title),
		"Infinity Raytracer: Ray: %f ms, Intersection: %f ms, Colour: %f ms, X: %.2f, Y: %.2f, Z: %.2f",
		m_rayCreationTime, m_intersectionTime, m_colourTime, m_camera->GetPosition().x,m_camera->GetPosition().y, m_camera->GetPosition().z
		);
	SetWindowTextA(m_direct3D->GetMainWindowHandle(), title);
}

void Application::SetThreadGroupSize( int x, int y )
{
	//------------------------------------
	//	Create new shaders and compile
	//------------------------------------

	m_rayCS	= m_compute->CreateComputeShader(_T("../Shaders/Ray.hlsl"), NULL, "main", NULL);
	m_intersectionCS = m_compute->CreateComputeShader(_T("../Shaders/Intersection.hlsl"), NULL, "main", NULL);
	m_colorCS = m_compute->CreateComputeShader(_T("../Shaders/Colour.hlsl"), NULL, "main", NULL);

	//------------------
	//	Set other data
	//------------------

	m_threadsPerGroupX = x;
	m_threadsPerGroupY = y;

	m_threadGroupX = m_global.width / m_threadsPerGroupX;
	m_threadGroupY = m_global.height / m_threadsPerGroupY;

	UpdateBuffer(m_globalBuffer, &m_global, sizeof(Global));
}

void Application::InitializeGUI( Direct3D* direct3D)
{

	//-------------------------------
	//	Initialise AntTweakBar GUI
	//-------------------------------

	int error = TwInit(TW_DIRECT3D11, direct3D->GetDevice());
	if(error == 0)
		std::cout << "TwInit Error" << std::endl;

	error = TwWindowSize(m_direct3D->GetWindowWidth(), direct3D->GetWindowWidth());
	if(error == 0)
		std::cout << "TwWindowSize Error" << std::endl;

	float ambientMaterial[4] = { m_lightAmbient.x, m_lightAmbient.y, m_lightAmbient.z, m_lightAmbient.w };

	m_gui = TwNewBar("Infinity Raytracer");
	int barSize[2] = {200, 300};
	TwSetParam(m_gui, NULL, "size", TW_PARAM_INT32, 2, barSize);
	TwAddVarRW(m_gui, "Lights", TW_TYPE_INT32, &m_value, "min=0 max=9" );
	TwAddVarRW(m_gui, "Reflection", TW_TYPE_INT32, &m_bounceCount, "min=1 max=5" );
	TwAddVarRW(m_gui, "Speed", TW_TYPE_FLOAT, &lightSpeed, "min=1 max=10 step=0.1" );
	TwAddVarRW(m_gui, "Move", TW_TYPE_BOOLCPP, &movePointlight, "" );
	TwAddVarRW(m_gui, "Lights", TW_TYPE_INT32, &m_value, "min=0 max=9" );

	TwAddVarRW(m_gui, "Ambient", TW_TYPE_COLOR3F, &m_lightAmbient, "step=0.1 group='Lighting'");
 	
	TwAddVarRW(m_gui, "Diffuse", TW_TYPE_COLOR3F, &m_lightDiffuse, "step=0.1 group='Lighting'"); 

	TwAddVarRW(m_gui, "Specular X", TW_TYPE_FLOAT, &m_lightSpecular.x, "step=0.1 group='Lighting'");  
	TwAddVarRW(m_gui, "Specular Y", TW_TYPE_FLOAT, &m_lightSpecular.y, "step=0.1 group='Lighting'");  
	TwAddVarRW(m_gui, "Specular Z", TW_TYPE_FLOAT, &m_lightSpecular.z, "step=0.1 group='Lighting'");  
	TwAddVarRW(m_gui, "Specular W", TW_TYPE_FLOAT, &m_lightSpecular.w, "step=0.1 group='Lighting'"); 

	TwAddVarRW(m_gui, "Intensity", TW_TYPE_FLOAT, &m_lightAttenuation.x, "step=0.1 group='Lighting'");
	TwAddVarRW(m_gui, "Decay", TW_TYPE_FLOAT, &m_lightAttenuation.y, "step=0.1 group='Lighting'");
	TwAddVarRW(m_gui, "Falloff", TW_TYPE_FLOAT, &m_lightAttenuation.z, "step=0.1 group='Lighting'");
	TwAddVarRW(m_gui, "Beam", TW_TYPE_FLOAT, &m_lightAttenuation.w, "min=0 max=5 step=0.1 group='Lighting'");

	TwAddButton(m_gui, "Mouse", NULL, NULL, " label='LMB - Rotate'  group='Help' ");
	TwAddButton(m_gui, "Camera", NULL, NULL, " label='W - Forward'  group='Help' ");
	TwAddButton(m_gui, "Camera2", NULL, NULL, " label='S - Backward' group='Help' ");
	TwAddButton(m_gui, "Camera3", NULL, NULL, " label='A - Left'    group='Help' ");
	TwAddButton(m_gui, "Camera4", NULL, NULL, " label='D - Right'   group='Help' ");
	TwAddButton(m_gui, "Camera5", NULL, NULL, " label='Z - Up'      group='Help' ");
	TwAddButton(m_gui, "Camera6", NULL, NULL, " label='X - Down'    group='Help' ");
																	
}

void Application::RenderGUI() 
{
	//-----------------------------------
	//	Render GUI to set render target
	//-----------------------------------

	int twerr = TwDraw(); 
	if(twerr == 0)
		std::cout << TwGetLastError() << std::endl;
}