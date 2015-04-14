#pragma once

#include "ComputeShader.h"
#include "ComputeBuffer.h"
#include "ComputeTexture.h"

class Compute
{
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;

public:

	Compute(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~Compute();

	ComputeShader* CreateComputeShader(TCHAR* shaderFile, TCHAR* blobFileAppendix, char* functionName, D3D10_SHADER_MACRO* defines);

	ID3D11Buffer* CreateConstantBuffer(UINT uSize, VOID* initData, char* debugName = nullptr);
	ID3D11Buffer* CreateDynamicBuffer(UINT uSize, VOID* initData, char* debugName = nullptr);

	ComputeBuffer* CreateBuffer(COMPUTE_BUFFER_TYPE uType, UINT uElementSize,
		UINT uCount, bool isSRV, bool isUAV, VOID* pInitData, bool createStaging = false, char* debugName = nullptr);

	ComputeTexture* CreateTexture(DXGI_FORMAT dxFormat,	UINT uWidth,
		UINT uHeight, UINT uRowPitch, VOID* pInitData, bool createStaging = false, char* debugName = nullptr);

	ComputeTexture* CreateTexture(TCHAR* textureFilename, char* debugName = nullptr);

private:
	ID3D11Buffer* CreateStructuredBuffer(UINT uElementSize, UINT uCount, bool isSRV, bool isUAV, VOID* initData);
	ID3D11Buffer* CreateRawBuffer(UINT uSize, VOID* initData);
	ID3D11ShaderResourceView* CreateBufferSRV(ID3D11Buffer* buffer);
	ID3D11UnorderedAccessView* CreateBufferUAV(ID3D11Buffer* buffer);
	ID3D11Buffer* CreateStagingBuffer(UINT uSize);

	//texture functions
	ID3D11Texture2D* CreateTextureResource(DXGI_FORMAT dxFormat,UINT uWidth, UINT uHeight, UINT uRowPitch, VOID* initData);
	//ID3D11Buffer* CreateRawBuffer(UINT uSize, VOID* pInitData);
	ID3D11ShaderResourceView* CreateTextureSRV(ID3D11Texture2D* texture);
	ID3D11UnorderedAccessView* CreateTextureUAV(ID3D11Texture2D* texture);
	ID3D11Texture2D* CreateStagingTexture(ID3D11Texture2D* texture);

	void SetDebugName(ID3D11DeviceChild* object, char* debugName);
};


