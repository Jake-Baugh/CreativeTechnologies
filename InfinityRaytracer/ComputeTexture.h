#pragma once

#include "stdafx.h"
#include "ComputeBuffer.h"

class ComputeTexture
{
public:	
	
	explicit ComputeTexture();
	~ComputeTexture();

	ID3D11Texture2D*			GetResource()
	{ return m_resource; }
	ID3D11ShaderResourceView*	GetResourceView()
	{ return m_resourceView; }
	ID3D11UnorderedAccessView*	GetUnorderedAccessView()
	{ return m_unorderedAccessView; }
	ID3D11Texture2D*			GetStaging()
	{ return m_staging; }

	template<class T>
	T* Map()
	{
		D3D11_MAPPED_SUBRESOURCE MappedResource; 
		T* p = nullptr;
		if(SUCCEEDED(m_deviceContext->Map( m_staging, 0, D3D11_MAP_READ, 0, &MappedResource )))
			p = (T*)MappedResource.pData;

		return p;
	}

private:

	void Release();
	void Unmap();
	void CopyToStaging();

	ComputeTexture(const ComputeBuffer& cb) {}
	
	ID3D11Texture2D* m_resource;
	ID3D11ShaderResourceView* m_resourceView;
	ID3D11UnorderedAccessView* m_unorderedAccessView;
	ID3D11Texture2D* m_staging;

	ID3D11DeviceContext* m_deviceContext;

	friend class Compute;
};

