#ifndef COMMON
#define COMMON

struct Vertex
{
	float4 position;
	float4 normal;
	float4 texCoord;
	float4 diffuse;
	float4 specular;
};

struct Ray
{
	//Ray direction
	float4 direction;	
	//Ray origin
	float4 origin;
};

/*struct Sphere
{
	//Sphere position
	float3 position;
	//Sphere radius
	float radius;
	//Colour
	float3 color;
};*/

struct Hit
{
	float4 position;
	//ray hit == true, ray miss == false
	bool isHit;

	//Hit distance
	float distance;
	
	//Colour
	//float3 color;
	
	//Hit UV
	float2 uv;

	//Hit triangle ID
	int triangleID;
};

struct Global
{
	int windowWidth;
	int windowHeight;
	int pointlights;
	int vertices;
};

cbuffer camConst : register(b0)
{
	row_major float4x4 viewMatrix;
	row_major float4x4 projMatrix;
	float4 camPos;
}

cbuffer lightVertexConst : register(b2)
{
	Vertex pVertex[800];
}

cbuffer modelBBConst : register(b3)
{
	float4 bounds[2];
}

cbuffer loopCounter : register(b4)
{
	int loopCount;
}

cbuffer globalBuffer : register(b5)
{
	Global global;
}

Texture2D pTexture;


/*Hit RaySphereIntersect(Ray ray, Sphere sphere, float maxRange)
{
	//-----------------------------
	// Real time rendering 3rd Ed
	//-----------------------------

	Hit hit;
	hit.distance = 100000.0f;
	hit.color = float3(0.0f, 0.0f, 0.0f);

	//-----------------------------------------------
	// Calculate the vector from the ray origin 
	// to the center of the sphere
	//-----------------------------------------------

	float3 l = sphere.position - ray.origin;

	//-------------------------------------------------------
	// s = l . d
	// The projection of the l vector onto the ray direction
	//-------------------------------------------------------

	float s = dot(l, ray.direction);

	//----------------------------------------------
	//If true, the sphere is behind the rays origin
	//----------------------------------------------

	if(s < 0)
	{
		return hit;
	}

	float l2 = dot(l, l);
	float r2 = sphere.radius * sphere.radius;
	float s2 = s * s;

	float m2 = l2 - s2;
	
	//------------------------------------
	// The ray misses the sphere and all 
	// other calculations can be ommitted
	//------------------------------------
	
	if(m2 > r2)
	{
		return hit;
	}

	//------------------------------
	// squared length of the vector
	//------------------------------

	float q = sqrt(r2 - m2);

	//----------------------------------------------------------
	//	this case is used for when the ray is outside the sphere	
	//----------------------------------------------------------

	float t0 = s - q;

	//----------------------------------------------------------
	//	this case is used for when the ray is inside the sphere 
	//----------------------------------------------------------

	float t1 = s + q;
	
	if(t0 > maxRange)
	{
		return hit;
	}

	if (l2 > r2)
	{
		hit.distance = t0;
	}
	else
	{
		hit.distance = t1;
	}

	hit.color = sphere.color;

	return hit;

}*/

Hit RayTriangleIntersect(Ray ray, int triangleID, float maxRange)
{
	
	Hit hit;
	hit.distance = 0.0f;
	hit.isHit = false;
	hit.triangleID = triangleID;

	Vertex vertex1 = pVertex[triangleID];
	Vertex vertex2 = pVertex[triangleID+1];
	Vertex vertex3 = pVertex[triangleID+2];

	float3 e1 = vertex2.position.xyz-vertex1.position.xyz;
	float3 e2 = vertex3.position.xyz-vertex1.position.xyz;
	float3 q = cross(ray.direction.xyz, e2);
	float a = dot(e1, q);
	
	if(a > -0.000001 && a < 0.000001)
	{
		 return hit;
	}
	
	float f = 1 / a;
	float3 s = ray.origin.xyz - vertex1.position.xyz;
	float u = f*(dot(s, q));

	if(u < 0 || u > 1.0f) 
	{
		return hit;
	}
	float3 r = cross(s, e1);
	float v = f*(dot(ray.direction.xyz, r));

	if(v < 0 || (u + v) > 1) 
	{
		return hit;
	}
	float t = f*(dot(e2, r));

	if(t < 0 || t > maxRange)
	{
		return hit;
	}

	hit.distance = t;
	hit.isHit = true;
	hit.uv = (1-u-v)*vertex1.texCoord.xy + u*vertex2.texCoord.xy + v*vertex3.texCoord.xy;
	return hit;

}

bool RayAABBIntersect(Ray ray, float maxRange)
{
    float3 invdir = 1.0f / ray.direction.xyz;
 
    float t1 = (bounds[0].x - ray.origin.x)*invdir.x;
    float t2 = (bounds[1].x - ray.origin.x)*invdir.x;
    float t3 = (bounds[0].y - ray.origin.y)*invdir.y;
    float t4 = (bounds[1].y - ray.origin.y)*invdir.y;
    float t5 = (bounds[0].z - ray.origin.z)*invdir.z;
    float t6 = (bounds[1].z - ray.origin.z)*invdir.z;

    float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
    float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

    if(tmax < 0)
    	return false;
    if(tmin > tmax)
    	return false;

    return true;
}

Hit RayObjectIntersect(Ray ray, float maxRange)
{
	Hit hit;
	hit.distance = 99999.0f;
	hit.isHit = false;

	Hit rayVsTriData;
	if(RayAABBIntersect(ray, maxRange))
	{
		for(int i = 0; i < global.vertices; i=i+3)
		{
			rayVsTriData = RayTriangleIntersect(ray, i, maxRange);
			if(rayVsTriData.isHit == true && rayVsTriData.distance < hit.distance)
			{
				hit = rayVsTriData;
			}
		}
	}

	return hit;
}

bool isRayObjectIntersect(Ray ray, float maxRange)
{
	Hit rayVsTriData;
	if(RayAABBIntersect(ray, maxRange))
	{
		for(int i = 0; i < global.vertices; i=i+3)
		{
			rayVsTriData = RayTriangleIntersect(ray, i, maxRange);
			if(rayVsTriData.isHit == true)
			{
				return true;
			}
		}
	}

	return false;
}
#endif