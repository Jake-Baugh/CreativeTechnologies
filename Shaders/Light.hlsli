struct PointLight
{
	float4 position, ambient, diffuse, specular, attenuation;
};

struct Material
{
	float3 position;
    float3 normal;
    float3 diffuse;
    float3 specular;
};

cbuffer pointLightConst : register(b1)
{
	PointLight pLights[10];
}

float3 CalculatePointLight(Material M, PointLight L)
{
	float3 lightColour = float3(0.0f, 0.0f, 0.0f);

	//--------------------------------------------------
	// Calculate vector from the material to the light.
	//--------------------------------------------------

	float3 lightVec = L.position.xyz - M.position;

	//-------------------------------------------------
	// Calculate the distance from material to light.
	//-------------------------------------------------

	float d = length(lightVec);
	
	if( d > L.attenuation.w )
		return float3(0.0f, 0.0f, 0.0f);

	//--------------------------
	// Normalize light vector.
	//--------------------------

	lightVec = normalize(lightVec); 
	
	//---------------------------------------------
	// Increment light output by the ambient light
	//---------------------------------------------

	lightColour += M.diffuse * L.ambient.xyz;	
	
	//-----------------------------------------------------------
	// Add diffuse and specular, provided the material is in 
	// the line of sight of the light
	//-----------------------------------------------------------

	float diffuseFactor = dot(lightVec, M.normal);

	if( diffuseFactor > 0.0f )
	{	//max(spec.a, 1.0f);
		float specPower  = max(2.0f, 1.0f);
		float3 toEye     = normalize(camPos.xyz - M.position);
		float3 R         = reflect(-lightVec, M.normal);
		float specFactor = pow(max(dot(R, toEye), 0.0f), specPower);
	
		//-----------------------
		// Diffuse and specular
		//-----------------------

		lightColour +=  diffuseFactor * M.diffuse * L.diffuse.xyz;
		lightColour +=  specFactor * M.specular * L.specular.xyz;
	}
	
	//------------
	// Attenuate
	//------------

	return lightColour / dot(L.attenuation.xyz, float3(1.0f, d, d*d));
}
