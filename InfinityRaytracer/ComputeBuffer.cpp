#include "stdafx.h"
#include "ComputeBuffer.h"

ComputeBuffer::ComputeBuffer()
{
	m_resource = nullptr;
	m_resourceView = nullptr;
	m_unorderedAccessView = nullptr;
	m_staging = nullptr;	
}

ComputeBuffer::~ComputeBuffer()
{ 
	Release(); 
}

void ComputeBuffer::Release()
{
	SAFE_RELEASE(m_resource);
	SAFE_RELEASE(m_resourceView);
	SAFE_RELEASE(m_unorderedAccessView);
	SAFE_RELEASE(m_staging);
}

void ComputeBuffer::Unmap()
{
	m_deviceContext->Unmap(m_staging, 0);
}

void ComputeBuffer::CopyToStaging()
{
	m_deviceContext->CopyResource( m_staging, m_resource);
}

