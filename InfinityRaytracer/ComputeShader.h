#pragma once

#include "stdafx.h"
#include <tchar.h>

class ComputeShader
{
public:

	~ComputeShader();

private:
	
	explicit ComputeShader();

	bool Init(TCHAR* shaderFile, TCHAR* blobFileAppendix, char* functionName, D3D10_SHADER_MACRO* defines,
		ID3D11Device* device, ID3D11DeviceContext* deviceContext);

	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;

	ID3D11ComputeShader* mShader;

public:

	void Set();
	void Unset();
	
	friend class Compute;
};
