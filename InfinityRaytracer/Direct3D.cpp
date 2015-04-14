#include "stdafx.h"
#include "Direct3D.h"
#include <AntTweakBar.h>

namespace
{
	Direct3D* g_win32 = nullptr;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	if(TwEventWin(hWnd, message, wParam,lParam))
		return 0;


	return g_win32->msgProc(hWnd, message, wParam, lParam);
}

Direct3D::Direct3D()
{
}


Direct3D::~Direct3D()
{
}

//Init the windows window
HRESULT Direct3D::InitializeWindow( HINSTANCE p_hInstance, PWSTR p_pCmdLine, int p_nCmdShow )
{
	m_leftMousePressed = false;
	g_win32 = this;

	WNDCLASSEX l_windowEx		= {0};
	l_windowEx.cbSize			= sizeof(WNDCLASSEX);
	l_windowEx.style			= CS_HREDRAW | CS_VREDRAW;
	l_windowEx.lpfnWndProc		= WndProc;
	l_windowEx.cbClsExtra		= 0;
	l_windowEx.cbWndExtra		= 0;
	l_windowEx.hInstance		= p_hInstance;
	l_windowEx.hIcon			= 0;
	l_windowEx.hCursor			= LoadCursor(NULL, IDC_ARROW);
	l_windowEx.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	l_windowEx.lpszMenuName		= 0;
	l_windowEx.lpszClassName	= L"Infinity Raytracer";
	l_windowEx.hIcon			= 0;
	
	//Register the window
	if(!RegisterClassEx(&l_windowEx))
	{
		MessageBox(NULL, L"Could not register window class", L"RegisterClassEx", MB_OK);
		return E_FAIL;
	}

	//Set window size
	RECT l_rect = {0, 0, 800, 800};
	AdjustWindowRect(&l_rect, WS_OVERLAPPEDWINDOW, FALSE);

	//Create the window
	if(!(m_hwnd = CreateWindow(
							L"Infinity Raytracer",
							L"Infinity Raytracer", //Name displayed in the window top bar
							WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							l_rect.right - l_rect.left,
							l_rect.bottom - l_rect.top,
							NULL,
							NULL,
							p_hInstance,
							NULL)))
	{
		MessageBox(NULL, L"Could not create window!", L"CreateWindow", MB_OK);
		return E_FAIL;
	}

	ShowWindow(m_hwnd, p_nCmdShow);

	return S_OK;
}

HRESULT Direct3D::InitializeDirectX(int p_width, int p_height)
{
	HRESULT hr = S_OK;

	m_mousePos = XMFLOAT2(0.0f, 0.0f);

	//RECT rc;
	//GetClientRect( m_hwnd, &rc );
	//m_width = rc.right - rc.left;
	//m_height = rc.bottom - rc.top;

	m_width = p_width;
	m_height = p_height;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory( &sd, sizeof(sd) );
	sd.BufferCount = 1;
	sd.BufferDesc.Width = m_width;
	sd.BufferDesc.Height = m_height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	sd.OutputWindow = m_hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevelsToTry[] = {
		D3D_FEATURE_LEVEL_11_0
	};

	hr = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		featureLevelsToTry,
		ARRAYSIZE(featureLevelsToTry),
		D3D11_SDK_VERSION,
		&sd,
		&m_swapChain,
		&m_device,
		NULL,
		&m_deviceContext);

	if( FAILED(hr) )
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer;
	ID3D11Texture2D* dsBuffer;
	hr = m_swapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer );
	if( FAILED(hr) )
		return hr;

	hr = m_device->CreateRenderTargetView(pBackBuffer, NULL, &m_renderTarget);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	//Depth buffer
	D3D11_TEXTURE2D_DESC    g_DepthStencilDesc;
	g_DepthStencilDesc.Width = m_width;
	g_DepthStencilDesc.Height = m_height;
	g_DepthStencilDesc.MipLevels = 1;
	g_DepthStencilDesc.ArraySize = 1;
	g_DepthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	g_DepthStencilDesc.SampleDesc = sd.SampleDesc;
	g_DepthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	g_DepthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	g_DepthStencilDesc.CPUAccessFlags = 0;
	g_DepthStencilDesc.MiscFlags = 0;
	hr = m_device->CreateTexture2D(&g_DepthStencilDesc, NULL, &dsBuffer);
	if (FAILED(hr))
	{
		dsBuffer->Release();
		return hr;
	}
	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));
	dsDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	hr = m_device->CreateDepthStencilView(dsBuffer, &dsDesc, &m_depthBuffer);
	dsBuffer->Release();
	if (FAILED(hr))
		return hr;

	m_deviceContext->OMSetRenderTargets(1, &m_renderTarget, m_depthBuffer);

	// create shader unordered access view on back buffer for compute shader to write into texture
	hr = m_device->CreateUnorderedAccessView( pBackBuffer, NULL, &m_backBufferUAV );

	D3D11_VIEWPORT vp;
	vp.Width = (float)m_width;
	vp.Height = (float)m_height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_deviceContext->RSSetViewports(1, &vp);

	return hr;
}

LRESULT Direct3D::msgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{
	case WM_LBUTTONDOWN:
		{
			m_leftMousePressed = true;
			m_prevMousePos.x = (float)LOWORD(lParam); 
			m_prevMousePos.y = (float)HIWORD(lParam);
		}
		return 0;

	case WM_LBUTTONUP:
		m_leftMousePressed = false;
		return 0;

	case WM_MOUSEMOVE:
		m_mousePos.x = (float)LOWORD(lParam); 
		m_mousePos.y = (float)HIWORD(lParam);
		return 0;

	case WM_DESTROY: //On exit event
		PostQuitMessage(0);
		return 0;

	case WM_KEYDOWN: //Exit on ESC press
		switch(wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		}
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


