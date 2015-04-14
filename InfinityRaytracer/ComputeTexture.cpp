#include "stdafx.h"
#include "ComputeTexture.h"

ComputeTexture::ComputeTexture()
{
	m_resource = NULL;
	m_resourceView = NULL;
	m_unorderedAccessView = NULL;
	m_staging = NULL;
}

ComputeTexture::~ComputeTexture()
{
	Release();
}

void ComputeTexture::Release()
{
	SAFE_RELEASE(m_resource);
	SAFE_RELEASE(m_resourceView);
	SAFE_RELEASE(m_unorderedAccessView);
	SAFE_RELEASE(m_staging);
}

void ComputeTexture::Unmap()
{
	m_deviceContext->Unmap(m_staging, 0);
}

void ComputeTexture::CopyToStaging()
{
	m_deviceContext->CopyResource( m_staging, m_resource);
}