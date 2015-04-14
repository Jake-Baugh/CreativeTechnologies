#include "RaytracerCommon.hlsli"
#include "Light.hlsli"

RWStructuredBuffer<Ray> inOutRay : register(u0);
RWStructuredBuffer<Hit> OutputHit : register(u1);

[numthreads(16,16, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{
	uint padding = 0;
	uint numSphere = 0;

	int index = threadID.x + threadID.y * global.windowWidth;

	Ray originRay = inOutRay[index];

	Hit hit;

	if(loopCount == 0)
	{
		hit.distance = 0.0;
		hit.isHit = false;
		hit = RayObjectIntersect(originRay, 99999.0);
	}
	else
	{
		hit = OutputHit[index];
		if(hit.isHit == true)
		{
			hit = RayObjectIntersect(originRay, 99999.0);
		}
	}

	if(hit.isHit == true)
	{
		
		Ray reflectionRay;

		reflectionRay.direction = float4(reflect(originRay.direction.xyz, pVertex[hit.triangleID].normal.xyz), 1.0); 
		reflectionRay.origin = originRay.direction * (hit.distance - 0.0001) + originRay.origin;

		hit.position = reflectionRay.origin;

		inOutRay[index] = reflectionRay;
	}

	OutputHit[index] = hit;
}

