#include "RaytracerCommon.hlsli"
#include "Light.hlsli"

//---------------------------------------
// Write texture for final output color
//---------------------------------------

RWTexture2D<float4> output : register(u0);

//----------------------------------------
// Accumulated color from reflected rays
//----------------------------------------

RWStructuredBuffer<float4> accOutput : register(u1);

//--------------------
//Intersection buffer
//--------------------

RWStructuredBuffer<Hit> inputHit : register(u2);
//RWStructuredBuffer<Sphere> spheres : register(u3);

//-----------------------------------------------
// Sample the DDS texture attached to the object
//-----------------------------------------------

SamplerState TextureSampler
{
    Filter = MIN_MAG_MILINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

[numthreads(16,16, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{
	uint padding = 0;
	uint numSphere = 0;

	//spheres.GetDimensions(numSphere, padding);

	float index = threadID.x + threadID.y * global.windowWidth;

	//-----------------------------------------------------------
	// Clear all colour if its the first bounce of the frame
	//-----------------------------------------------------------

	if(loopCount == 0)
	{
		accOutput[index] = float4(0.0, 0.0, 0.0, 0.0);
		output[threadID.xy] = float4(0.9, 0.9, 0.9, 0.0);
	}

	Hit hit = inputHit[index];

	//-------------------------------------------
	//If no intersection occurs skip this loop
	//-------------------------------------------

	if(hit.isHit)
	{
		
		float3 lightColor = float3(0,0,0);
		Ray shadowRay;
		Material material;

		shadowRay.origin = hit.position;

		//------------------------------------------------
		//	Loop for every light if intersection occurs 
		//------------------------------------------------

		for(int j = 0; j < global.pointlights; j++)
		{
			shadowRay.direction = normalize(pLights[j].position - shadowRay.origin);

			float maxDistance = length(pLights[j].position.xyz - shadowRay.origin.xyz);

			if(isRayObjectIntersect(shadowRay, maxDistance))
				continue;
		
			//---------------------------------------------------------------------
			// Set the position of the material to the newly created 'shadow' ray
			///--------------------------------------------------------------------

			material.position 		= shadowRay.origin.xyz;
			material.normal 		= pVertex[hit.triangleID].normal.xyz;
			material.diffuse		= pVertex[hit.triangleID].diffuse.xyz;
			material.specular    	= pVertex[hit.triangleID].specular.xyz;

			//----------------------------------------------------------
			// Add the Point Light colour to the final colour output
			//----------------------------------------------------------

			lightColor += CalculatePointLight(material, pLights[j]);
		}
		
		//-------------------------
		//	Sample texture color
		//-------------------------

		float4 texColor = pTexture.SampleLevel( TextureSampler, hit.uv, 0 );		

		//------------------------------------------------------------
		//	Add color from previous bounces. Divide by bounce factor.
		//------------------------------------------------------------
		
		accOutput[index] += float4(lightColor * texColor.xyz, 1.0) / (loopCount+1);
		
		//-----------------
		//	Final output
		//-----------------

		output[threadID.xy] = accOutput[index];
	}
}

