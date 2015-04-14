#ifndef DIRECT3D_H
#define DIRECT3D_H

#include "stdafx.h"

using namespace DirectX;

class Direct3D
{
public:

	Direct3D();
	~Direct3D();
	
	HRESULT InitializeWindow(HINSTANCE p_hInstance, PWSTR p_pCmdLine, int p_nCmdShow);
	HRESULT InitializeDirectX(int p_width, int p_height);

	IDXGISwapChain* GetSwapChain() { return m_swapChain; }
	ID3D11Device* GetDevice() { return m_device; }
	ID3D11DeviceContext* GetDeviceContext() { return m_deviceContext; }
	ID3D11UnorderedAccessView* GetUnorderedAccessView() { return m_backBufferUAV; }
	ID3D11RenderTargetView* GetRenderTarget() { return m_renderTarget; }
	ID3D11DepthStencilView* GetDepthBuffer() { return m_depthBuffer; }
	HWND GetMainWindowHandle() { return m_hwnd; }
	int GetWindowWidth() { return m_width; }
	int GetWindowHeight() { return m_height; }
	XMFLOAT2 GetMousePos() { return m_mousePos; }
	bool GetLeftMousePressed() { return m_leftMousePressed; }

	LRESULT msgProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );	
private:
	HWND m_hwnd;

	bool m_leftMousePressed;

	int m_width;
	int m_height;

	XMFLOAT2 m_mousePos;
	XMFLOAT2 m_prevMousePos;

	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11UnorderedAccessView* m_backBufferUAV;  
	ID3D11RenderTargetView* m_renderTarget;
	ID3D11DepthStencilView* m_depthBuffer;
};
#endif
