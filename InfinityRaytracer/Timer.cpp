#include "stdafx.h"
#include "Timer.h"

Timer::Timer(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dDeviceContext)
	: mDevice(d3dDevice), mDeviceContext(d3dDeviceContext)
{
    D3D11_QUERY_DESC desc;
    desc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
    desc.MiscFlags = 0;
	mDevice->CreateQuery(&desc, &mDisjoint);

	desc.Query = D3D11_QUERY_TIMESTAMP;
	mDevice->CreateQuery(&desc, &mStart);
	mDevice->CreateQuery(&desc, &mStop);
}

Timer::~Timer()
{
	if(mStart)		mStart->Release();
	if(mStop)		mStop->Release();
	if(mDisjoint)	mDisjoint->Release();
}

void Timer::Start()
{
	mDeviceContext->Begin(mDisjoint);
 
	mDeviceContext->End(mStart);
}

void Timer::Stop()
{
    mDeviceContext->End(mStop);

    mDeviceContext->End(mDisjoint);
}

double Timer::GetTime()
{
	UINT64 startTime = 0;
	while(mDeviceContext->GetData(mStart, &startTime, sizeof(startTime), 0) != S_OK);

	UINT64 endTime = 0;
	while(mDeviceContext->GetData(mStop, &endTime, sizeof(endTime), 0) != S_OK);

	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
	while(mDeviceContext->GetData(mDisjoint, &disjointData, sizeof(disjointData), 0) != S_OK);

	double time = -1.0f;
	if(disjointData.Disjoint == FALSE)
	{
		UINT64 delta = endTime - startTime;
		double frequency = static_cast<double>(disjointData.Frequency);
		time = (delta / frequency) * 1000.0f;
	}

	return time;
}