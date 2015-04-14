#include "stdafx.h"
#include "ComputeShader.h"
#include <string>
#include <cstdio>

ComputeShader::ComputeShader() : m_device(NULL), m_deviceContext(NULL){}
ComputeShader::~ComputeShader()
{
	SAFE_RELEASE(m_device);
}

bool ComputeShader::Init(TCHAR* shaderFile, TCHAR* blobFileAppendix, char* functionName, D3D10_SHADER_MACRO* defines,
						 ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	HRESULT hr = S_OK;
	m_device = device;
	m_deviceContext = deviceContext;

	ID3DBlob* compiledShader = nullptr;
	ID3DBlob* errorBlob = nullptr;

	FILE* shaderBlob = nullptr;

	TCHAR blobFilename[300];
	if(blobFileAppendix != nullptr)
	{
		size_t l1 = _tcslen(shaderFile);
		size_t l2 = _tcslen(_tcsrchr(shaderFile, _T('.')));
		_tcsncpy_s(blobFilename, shaderFile, l1 - l2);

		_tcscat_s(blobFilename, _T("_"));
		_tcscat_s(blobFilename, blobFileAppendix);
		_tcscat_s(blobFilename, _T(".blob"));

		//MessageBox(0, blobFilename, L"", 0);

		_tfopen_s(&shaderBlob, blobFilename, _T("rb"));
	}

	DWORD dwShaderFlags =	D3DCOMPILE_ENABLE_STRICTNESS |
		D3DCOMPILE_IEEE_STRICTNESS |
		//D3DCOMPILE_WARNINGS_ARE_ERRORS |
		D3DCOMPILE_PREFER_FLOW_CONTROL;

#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	dwShaderFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	if(shaderBlob == nullptr)
	{
		hr = D3DCompileFromFile(shaderFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE , functionName, "cs_5_0", 
			dwShaderFlags, NULL, &compiledShader, &errorBlob);
		if(errorBlob)
		{
			//	MessageBoxA(0, (char*)pErrorBlob->GetBufferPointer(), "Shader Error" , 0);
		}
		if(hr == S_OK)
		{
			if(blobFileAppendix != nullptr)
			{
				_tfopen_s(&shaderBlob, blobFilename, _T("wb"));

				if(shaderBlob != nullptr)
				{
					size_t size = compiledShader->GetBufferSize();
					fwrite(&size, sizeof(size_t), 1, shaderBlob);
					fwrite(compiledShader->GetBufferPointer(), size, 1, shaderBlob);
					fclose(shaderBlob);
				}
			}
		}
	}
	else
	{
		int size = 0;
		fread_s(&size, sizeof(int), sizeof(int), 1, shaderBlob);

		if(D3DCreateBlob(size, &compiledShader) == S_OK)
		{
			fread_s(compiledShader->GetBufferPointer(), size, size, 1, shaderBlob);
		}

		fclose(shaderBlob);
	}
	if (errorBlob)
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}

	if(hr == S_OK)
	{
		hr = m_device->CreateComputeShader(compiledShader->GetBufferPointer(),
			compiledShader->GetBufferSize(), nullptr, &mShader);
	}

	SAFE_RELEASE(errorBlob);
	SAFE_RELEASE(compiledShader);

	return (hr == S_OK);
}

void ComputeShader::Set()
{
	m_deviceContext->CSSetShader( mShader, NULL, 0 );
}

void ComputeShader::Unset()
{
	m_deviceContext->CSSetShader( NULL, NULL, 0 );
}