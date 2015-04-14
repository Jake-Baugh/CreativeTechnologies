#ifndef STDAFX_H
#define STDAFX_H

#include <windows.h>
#include <D3D11.h>
#include <d3dCompiler.h>
#include <d3dcommon.h>
#include <DirectXMath.h>
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>

using namespace DirectX;
using namespace std;

#define SAFE_RELEASE(x)			{ if (x) { (x)->Release();	(x) = nullptr; } }
#define SAFE_DELETE(x)			{ if (x) { delete (x);		(x) = nullptr; } }
#define SAFE_DELETE_ARRAY(x)	{ if (x) { delete[](x);		(x) = nullptr; } }

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#endif