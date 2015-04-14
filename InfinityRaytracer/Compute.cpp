#include "stdafx.h"
#include "Compute.h"

Compute::Compute(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_device = device;
	m_deviceContext = deviceContext;
}

ComputeBuffer* Compute::CreateBuffer(COMPUTE_BUFFER_TYPE uType, UINT uElementSize, UINT uCount, bool isSRV, bool isUAV, VOID* initData, bool bCreateStaging, char* debugName)
{
	ComputeBuffer* buffer = new ComputeBuffer();
	buffer->m_deviceContext = m_deviceContext;

	if(uType == STRUCTURED_BUFFER)
		buffer->m_resource = CreateStructuredBuffer(uElementSize, uCount, isSRV, isUAV, initData);
	else if(uType == RAW_BUFFER)
		buffer->m_resource = CreateRawBuffer(uElementSize * uCount, initData);

	if(buffer->m_resource != nullptr)
	{
		if(isSRV)
			buffer->m_resourceView = CreateBufferSRV(buffer->m_resource);
		if(isUAV)
			buffer->m_unorderedAccessView = CreateBufferUAV(buffer->m_resource);
		
		if(bCreateStaging)
			buffer->m_staging = CreateStagingBuffer(uElementSize * uCount);
	}

	if(debugName)
	{
		if(buffer->m_resource)				SetDebugName(buffer->m_resource, debugName);
		if(buffer->m_staging)				SetDebugName(buffer->m_staging, debugName);
		if(buffer->m_resourceView)			SetDebugName(buffer->m_resourceView, debugName);
		if(buffer->m_unorderedAccessView)	SetDebugName(buffer->m_unorderedAccessView, debugName);
	}

	return buffer; //return shallow copy
}

ID3D11Buffer* Compute::CreateStructuredBuffer(UINT uElementSize, UINT uCount, bool isSRV, bool isUAV, VOID* initData)
{
    ID3D11Buffer* pBufOut = nullptr;

    D3D11_BUFFER_DESC desc;
    ZeroMemory( &desc, sizeof(desc) );
    desc.BindFlags = 0;
	
	if(isUAV)	desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	if(isSRV)	desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    
	UINT bufferSize = uElementSize * uCount;
	desc.ByteWidth = bufferSize < 16 ? 16 : bufferSize;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    desc.StructureByteStride = uElementSize;

    if ( initData )
    {
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = initData;
		m_device->CreateBuffer( &desc, &InitData, &pBufOut);
    }
	else
	{
		m_device->CreateBuffer(&desc, nullptr, &pBufOut);
	}

	return pBufOut;
}

ID3D11Buffer* Compute::CreateRawBuffer(UINT uSize, VOID* initData)
{
    ID3D11Buffer* pBufOut = nullptr;

    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_INDEX_BUFFER | D3D11_BIND_VERTEX_BUFFER;
    desc.ByteWidth = uSize;
    desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;

    if ( initData )
    {
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = initData;
        m_device->CreateBuffer(&desc, &InitData, &pBufOut);
    }
	else
	{
        m_device->CreateBuffer(&desc, nullptr, &pBufOut);
	}

	return pBufOut;
}

ID3D11ShaderResourceView* Compute::CreateBufferSRV(ID3D11Buffer* pBuffer)
{
	ID3D11ShaderResourceView* pSRVOut = nullptr;

    D3D11_BUFFER_DESC descBuf;
    ZeroMemory(&descBuf, sizeof(descBuf));
    pBuffer->GetDesc(&descBuf);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    desc.BufferEx.FirstElement = 0;

    if(descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        // This is a Raw Buffer
        desc.Format = DXGI_FORMAT_R32_TYPELESS;
        desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
        desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
    }
	else if(descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        // This is a Structured Buffer
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
    }
	else
	{
		return nullptr;
	}

    m_device->CreateShaderResourceView(pBuffer, &desc, &pSRVOut);

	return pSRVOut;
}

ID3D11UnorderedAccessView* Compute::CreateBufferUAV(ID3D11Buffer* pBuffer)
{
	ID3D11UnorderedAccessView* pUAVOut = nullptr;

	D3D11_BUFFER_DESC descBuf;
    ZeroMemory(&descBuf, sizeof(descBuf));
    pBuffer->GetDesc(&descBuf);
        
    D3D11_UNORDERED_ACCESS_VIEW_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = 0;

    if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
    {
        // This is a Raw Buffer
        desc.Format = DXGI_FORMAT_R32_TYPELESS; // Format must be DXGI_FORMAT_R32_TYPELESS, when creating Raw Unordered Access View
        desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
        desc.Buffer.NumElements = descBuf.ByteWidth / 4; 
    }
	else if(descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
    {
        // This is a Structured Buffer
        desc.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
        desc.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride; 
    }
	else
	{
		return nullptr;
	}
    
	m_device->CreateUnorderedAccessView(pBuffer, &desc, &pUAVOut);

	return pUAVOut;
}

ID3D11Buffer* Compute::CreateStagingBuffer(UINT uSize)
{
    ID3D11Buffer* debugbuf = nullptr;

    D3D11_BUFFER_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
	desc.ByteWidth = uSize;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
    
	m_device->CreateBuffer(&desc, nullptr, &debugbuf);

    return debugbuf;
}

// TEXTURE FUNCTIONS
ComputeTexture* Compute::CreateTexture(DXGI_FORMAT dxFormat, UINT uWidth,
	UINT uHeight, UINT uRowPitch, VOID* initData, bool createStaging, char* debugName)
{
	ComputeTexture* texture = new ComputeTexture();
	texture->m_deviceContext = m_deviceContext;

	texture->m_resource = CreateTextureResource(dxFormat, uWidth, uHeight, uRowPitch, initData);

	if(texture->m_resource != nullptr)
	{
		texture->m_resourceView = CreateTextureSRV(texture->m_resource);
		texture->m_unorderedAccessView = CreateTextureUAV(texture->m_resource);
		
		if(createStaging)
			texture->m_staging = CreateStagingTexture(texture->m_resource);
	}

	if(debugName)
	{
		if(texture->m_resource)				SetDebugName(texture->m_resource, debugName);
		if(texture->m_staging)				SetDebugName(texture->m_staging, debugName);
		if(texture->m_resourceView)			SetDebugName(texture->m_resourceView, debugName);
		if(texture->m_unorderedAccessView)	SetDebugName(texture->m_unorderedAccessView, debugName);
	}

	return texture;
}

ComputeTexture* Compute::CreateTexture(TCHAR* textureFilename, char* debugName)
{
	MessageBoxA(0, "ComputeWrap::CreateTexture is not implemented anymore!", "Error", 0);
	return nullptr;

	/*
	ComputeTexture* texture = new ComputeTexture();
	texture->_D3DContext = mD3DDeviceContext;

	if(SUCCEEDED(D3DX11CreateTextureFromFile(mD3DDevice, textureFilename, nullptr, nullptr, (ID3D11Resource**)&texture->_Resource, nullptr)))
	{
		texture->_ResourceView = CreateTextureSRV(texture->_Resource);
		
		if(debugName)
		{
			if(texture->_Resource)				SetDebugName(texture->_Resource, debugName);
			if(texture->_Staging)				SetDebugName(texture->_Staging, debugName);
			if(texture->_ResourceView)			SetDebugName(texture->_ResourceView, debugName);
			if(texture->_UnorderedAccessView)	SetDebugName(texture->_UnorderedAccessView, debugName);
		}
	}
	return texture;
	*/
}

ID3D11Texture2D* Compute::CreateTextureResource(DXGI_FORMAT dxFormat,
	UINT uWidth, UINT uHeight, UINT uRowPitch, VOID* initData)
{
	ID3D11Texture2D* pTexture = nullptr;

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = uWidth;
	desc.Height = uHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = dxFormat;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = initData;
	data.SysMemPitch = uRowPitch; //uWidth * 4;

	if(FAILED(m_device->CreateTexture2D( &desc, initData ? &data : nullptr, &pTexture )))
	{

	}

	return pTexture;
}

ID3D11ShaderResourceView* Compute::CreateTextureSRV(ID3D11Texture2D* texture)
{
	ID3D11ShaderResourceView* SRV = nullptr;

	D3D11_TEXTURE2D_DESC td;
	texture->GetDesc(&td);

	//init view description
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc; 
	ZeroMemory( &viewDesc, sizeof(viewDesc) ); 
	
	viewDesc.Format					= td.Format;
	viewDesc.ViewDimension			= D3D11_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels	= td.MipLevels;

	if(FAILED(m_device->CreateShaderResourceView(texture, &viewDesc, &SRV)))
	{
		//MessageBox(0, "Unable to create shader resource view", "Error!", 0);
	}

	return SRV;
}

ID3D11UnorderedAccessView* Compute::CreateTextureUAV(ID3D11Texture2D* texture)
{
	ID3D11UnorderedAccessView* UAV = nullptr;

	m_device->CreateUnorderedAccessView( texture, nullptr, &UAV );
	texture->Release();

	return UAV;
}

ID3D11Texture2D* Compute::CreateStagingTexture(ID3D11Texture2D* texture)
{
    ID3D11Texture2D* stagingTexture = nullptr;

    D3D11_TEXTURE2D_DESC desc;
	texture->GetDesc(&desc);
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.MiscFlags = 0;
    
	m_device->CreateTexture2D(&desc, nullptr, &stagingTexture);

    return stagingTexture;
}

ID3D11Buffer* Compute::CreateConstantBuffer(UINT uSize, VOID* initData, char* debugName)
{
	HRESULT hr;

	ID3D11Buffer* buffer = nullptr;

	// setup creation information
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	bool addMod = uSize % 16 != 0 ? true : false;
	cbDesc.ByteWidth = uSize + (addMod ? (16 - uSize % 16) : 0);
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	cbDesc.Usage = D3D11_USAGE_DEFAULT;

    if(initData)
    {
        D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(D3D11_SUBRESOURCE_DATA));
        InitData.pSysMem = initData;

        hr = m_device->CreateBuffer(&cbDesc, &InitData, &buffer);
		if(FAILED(hr))
			std::cout<< "Failed to build constant buffer!" << std::endl;
    }
	else
	{
        hr = m_device->CreateBuffer(&cbDesc, nullptr, &buffer);
		if(FAILED(hr))
			std::cout<< "Failed to build constant buffer!" << std::endl;
	}

	if(debugName && buffer)
	{
		SetDebugName(buffer, debugName);
	}

	return buffer;
}

void Compute::SetDebugName(ID3D11DeviceChild* object, char* debugName)
{
#if defined( DEBUG ) || defined( _DEBUG )
	// Only works if device is created with the D3D10 or D3D11 debug layer, or when attached to PIX for Windows
	object->SetPrivateData( WKPDID_D3DDebugObjectName, (unsigned int)strlen(debugName), debugName );
#endif
}

ComputeShader* Compute::CreateComputeShader(TCHAR* shaderFile, TCHAR* blobFileAppendix, char* pFunctionName, D3D10_SHADER_MACRO* pDefines)
{
	ComputeShader* cs = new ComputeShader();

	if(cs && !cs->Init(
		shaderFile,
		blobFileAppendix,
		pFunctionName,
		pDefines,
		m_device,
		m_deviceContext))
	{
		SAFE_DELETE(cs);
	}

	return cs;
}

ID3D11Buffer* Compute::CreateDynamicBuffer( UINT uSize, VOID* initData, char* debugName /*= nullptr*/ )
{
	ID3D11Buffer* pBuffer = nullptr;

	// setup creation information
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	bool addMod = uSize % 16 != 0 ? true : false;
	cbDesc.ByteWidth = uSize + (addMod ? (16 - uSize % 16) : 0);
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;

    if(initData)
    {
        D3D11_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = initData;
        m_device->CreateBuffer(&cbDesc, &InitData, &pBuffer);
    }
	else
	{
        m_device->CreateBuffer(&cbDesc, nullptr, &pBuffer);
	}

	if(debugName && pBuffer)
	{
		SetDebugName(pBuffer, debugName);
	}

	return pBuffer;
}
