#include "RaytracerCommon.hlsli"
#include "Light.hlsli"

RWStructuredBuffer<Ray> outRay : register(u0);

Ray getRay(float2 pixelpos)
{
	Ray ray; 
	float4  p1, p2;
	float2 pixelClip;

	//------------------------------------------------------
	//	Transform pixel coordinates to clip space 
	//------------------------------------------------------

	pixelClip.x = projMatrix._11*((pixelpos.x / (global.windowWidth * 0.5f)) - 1);
	pixelClip.y = projMatrix._22*(1.0f - (pixelpos.y / (global.windowHeight * 0.5f)));

	//----------------------------------------------------
	//	p1 = (normalizedX * NEAR, normalizedY * NEAR, NEAR)
	//----------------------------------------------------
	p1 = float4(pixelClip.x, pixelClip.y, 1, 0);

	//----------------------------------------------------
	//	p2 = (normalizedX * FAR, normalizedY * FAR, FAR)
	//----------------------------------------------------

	p2 = float4(pixelClip.x*2, pixelClip.y*2, 2, 0);

	//------------------------------------------
	//	Multiply p1 with inverse view matrix
	//------------------------------------------

	p1 = mul(p1, viewMatrix);

	//------------------------------------------
	//	Multiply p2 with inverse view matrix
	//------------------------------------------

	p2 = mul(p2, viewMatrix);

	//---------------------------------------------------------------
	//	Get vector between p1 and p2 and normalize to get direction
	//---------------------------------------------------------------

	ray.direction = normalize(p2 - p1);
	ray.origin = camPos;

	return ray;
}

[numthreads(16,16, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{
	outRay[threadID.x + threadID.y * global.windowWidth] = getRay(threadID.xy);
}

