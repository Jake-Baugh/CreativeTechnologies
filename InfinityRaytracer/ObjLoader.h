#ifndef MODELLOADER_
#define MODELLOADER_

#include "Compute.h"
#include <DirectXTex.h>
#include <fstream>
#include <sstream>

#include <string>
#include <vector>

using namespace DirectX;
using namespace std;

struct Vertex
{
	XMFLOAT4 position;
	XMFLOAT4 normal;
	XMFLOAT4 texCoord;
	XMFLOAT4 diffuse;
	XMFLOAT4 specular;
};

struct Material
{
	XMFLOAT4 m_ambient;
	XMFLOAT4 m_diffuse;
	XMFLOAT4 m_specular;
	int	m_shininess;
	float m_alpha;
	ID3D11ShaderResourceView* m_textureResource;
	~Material()
	{
		m_textureResource->Release();
	}
};

struct Model
{
	XMFLOAT4 topBoundingCorner;
	XMFLOAT4 bottomBoundingCorner;
	ID3D11Buffer* vertexBuffer;
	string bufferName;
	UINT offset;
	UINT stride;
	int	size;
	Material* material;

	~Model()
	{
		delete vertexBuffer;
		delete material;
	}
};

struct AABB 
{
	AABB(XMFLOAT4 minBounds, XMFLOAT4 maxBounds)
	{
		bounds[0] = minBounds;
		bounds[1] = maxBounds;
	}

	XMFLOAT4 bounds[2];
};

class ObjLoader
{

public:
	
	ObjLoader(Compute* compute, ID3D11Device* device);
	Model* AddStaticModel(string modelName, string OBJFileName);
	~ObjLoader();

private:

	HRESULT	CreateBuffers();
	void CalculateBoundingBox(XMFLOAT4 vector3);
	void LoadMaterialFromMTL(string materialPath, string materialFileName);
	void LoadModelFromOBJFile(string modelName, string OBJFileName);
	int	CalculateIndex(Vertex* vertex);
	ID3D11ShaderResourceView* CreateTexture(string textureFileName);

private:

	ID3D11Device* m_device;
	Compute* m_computeWrap;

	vector<Vertex> m_vertices;

	ID3D11Buffer* m_vertexBuffer;

	string m_OBJFileName;

	vector<XMFLOAT4> m_positions;
	vector<XMFLOAT4> m_texCoords;
	vector<XMFLOAT4> m_normals;

	Material* m_material;

	XMFLOAT4 m_topCorner;
	XMFLOAT4 m_bottomCorner;
};
#endif