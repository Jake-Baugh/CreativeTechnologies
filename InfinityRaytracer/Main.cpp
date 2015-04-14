#include "stdafx.h"
#include "direct3D.h"
#include "Application.h"
#include "Timer.h"
#include <iostream>
#include <fstream>
#include <vector>

#pragma warning(disable: 4099) 

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{

	Direct3D* direct3D = new Direct3D();

	direct3D->InitializeWindow(hInstance, pCmdLine, nCmdShow);
	if(direct3D->InitializeDirectX(800, 800));

	Timer* timer = new Timer(direct3D->GetDevice(), direct3D->GetDeviceContext());

	Application* application = new Application();
	application->Initialize(direct3D, timer);

	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	__int64 prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);

	int caseID = 0;

	MSG msg = {0};

	//-----------------
	//	Run main loop 
	//-----------------

	while(WM_QUIT != msg.message)
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			__int64 currTimeStamp = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
			float dt = (currTimeStamp - prevTimeStamp) * secsPerCnt;

			application->Update(dt);
			application->Render();

			prevTimeStamp = currTimeStamp;
		}
	}

	system("pause");
	return (int) msg.wParam;
}