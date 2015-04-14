#include "stdafx.h"
#include "ObjLoader.h"

ObjLoader::ObjLoader(Compute* compute, ID3D11Device* device)
{
	m_device	= device;
	m_computeWrap = compute;
	
	m_vertices.clear();

	m_positions.clear();
	m_positions.shrink_to_fit();
	m_texCoords.clear();
	m_texCoords.shrink_to_fit();
	m_normals.clear();
	m_normals.shrink_to_fit();

	m_topCorner = XMFLOAT4((float)INT_MAX, (float)INT_MAX, (float)INT_MAX, 0);
	m_bottomCorner = XMFLOAT4(0,0,0,0);
}

ObjLoader::~ObjLoader()
{

}

Model* ObjLoader::AddStaticModel( string modelName, string OBJFileName )
{
	LoadModelFromOBJFile(modelName, OBJFileName);

	CreateBuffers();

	Model* model = new Model();

	model->bufferName = modelName;
	model->vertexBuffer = m_vertexBuffer;
	model->offset = 0;
	model->stride = sizeof(Vertex);
	model->size = m_vertices.size();
	model->material = m_material;
	model->bottomBoundingCorner = m_bottomCorner;
	model->topBoundingCorner = m_topCorner;

	return model;
}

HRESULT ObjLoader::CreateBuffers()
{
	//Create Buffer
	int vertexCount = static_cast<int>(m_vertices.size());
	m_vertexBuffer = m_computeWrap->CreateConstantBuffer(sizeof(Vertex) * vertexCount,  &m_vertices.front());
	return S_OK;
}

void ObjLoader::CalculateBoundingBox(XMFLOAT4 vector3)
{
	if(vector3.x < 3 && vector3.x > -3 && vector3.y < 2 && vector3.y > -2 && vector3.z < 3 && vector3.z > -3)
	{
		if(m_topCorner.x == (float)INT_MAX && m_topCorner.y == (float)INT_MAX && m_topCorner.z == (float)INT_MAX)
		{
			m_topCorner = vector3;
			m_bottomCorner = vector3;
		}
		if(vector3.x < m_bottomCorner.x) 
			m_bottomCorner.x = vector3.x;
		if(vector3.y < m_bottomCorner.y)
			m_bottomCorner.y = vector3.y;
		if(vector3.z < m_bottomCorner.z)
			m_bottomCorner.z = vector3.z;
		if(vector3.x > m_topCorner.x)
			m_topCorner.x = vector3.x;
		if(vector3.y > m_topCorner.y)
			m_topCorner.y = vector3.y;
		if(vector3.z > m_topCorner.z)
			m_topCorner.z = vector3.z;
	}
}


ID3D11ShaderResourceView* ObjLoader::CreateTexture( string textureFileName )
{
	std::wstring wsTmp(textureFileName.begin(), textureFileName.end());

	ID3D11ShaderResourceView* resourceView = nullptr;

	CreateDDSTextureFromFile( m_device, wsTmp.c_str(), nullptr, &resourceView );
	
	return resourceView;
}

int ObjLoader::CalculateIndex( Vertex* vertex )
{
	int indexReturn = m_vertices.size();
	m_vertices.push_back(*vertex);
	return indexReturn;
}

void ObjLoader::LoadMaterialFromMTL(string materialPath, string materialFileName )
{
	m_material = new Material();

	fstream objFile(materialPath + "\\" + materialFileName);

	if(objFile)
	{
		string line;
		string prefix;

		while(objFile.eof() == false)
		{
			prefix = "NULL"; //leave nothing from the previous iteration
			stringstream lineStream;

			getline(objFile, line);
			lineStream << line;
			lineStream >> prefix;

			if(prefix == "map_Kd")
			{
				string materialTextureName;
				lineStream >> materialTextureName;

				//------------------------------------------
				//	Create texture from resource dds in mtl
				//------------------------------------------

				m_material->m_textureResource = CreateTexture(materialPath + "\\" + materialTextureName);
			}
			else if(prefix == "Ns")
			{
				int nShininess;
				lineStream >> nShininess;
				m_material->m_shininess = nShininess;
			}
			else if(prefix == "d" || prefix == "Tr" )
			{
				lineStream >> m_material->m_alpha;
			}
			else if(prefix == "Ks")
			{
				float r, g, b;
				lineStream >> r >> g >> b;
				m_material->m_diffuse = XMFLOAT4( r, g, b, 0 );
			}
			else if(prefix == "Kd")
			{
				float r, g, b;
				lineStream >> r >> g >> b;
				m_material->m_specular = XMFLOAT4( r, g, b, 0 );
			}	
			else if(prefix == "Ka")
			{
				float r, g, b;
				lineStream >> r >> g >> b;
				m_material->m_ambient = XMFLOAT4( r, g, b, 0 );
			}
		}
	}
}

void ObjLoader::LoadModelFromOBJFile(string modelName, string OBJFileName )
{
	fstream objFile(OBJFileName + "\\" + modelName );

	if(objFile)
	{

		string strMaterialFilename;
		string line;
		string prefix;

		while(objFile.eof() == false)
		{
			prefix = "NULL"; 
			stringstream lineStream;

			getline(objFile, line);
			lineStream << line;
			lineStream >> prefix;

			if(prefix == "mtllib")
			{
				lineStream >> strMaterialFilename;
				LoadMaterialFromMTL(OBJFileName, strMaterialFilename);
			}
			else if(prefix == "v")
			{
				XMFLOAT4 pos;
				pos.w = 0;
				lineStream >> pos.x >> pos.y >> pos.z;
				m_positions.push_back(pos);
				CalculateBoundingBox(pos);
			}
			else if(prefix == "vt")
			{
				XMFLOAT4 uv;
				uv.y = 0;
				uv.w = 0;
				lineStream >> uv.x >> uv.y;
				uv.y = 1 - uv.y; 
				m_texCoords.push_back(uv);
			}
			else if (prefix == "vn")
			{
				XMFLOAT4 normal;
				normal.w = 0;
				lineStream >> normal.x >> normal.y >> normal.z;
				m_normals.push_back(normal);
			}
			else if(prefix == "f")
			{
				Vertex tempVertex;
				char tmp;

				int indexPos = 0;
				int texPos = 0;
				int normPos = 0;

				for(int i=0; i<3; i++)
				{

					lineStream >> indexPos >> tmp >> texPos >>  tmp >> normPos;

					ZeroMemory(&tempVertex, sizeof(Vertex));
					
					tempVertex.position = m_positions[ indexPos - 1];
					tempVertex.texCoord = m_texCoords[ texPos - 1 ]; 
					tempVertex.normal = m_normals[ normPos - 1 ]; 
					tempVertex.diffuse = m_material->m_diffuse;
					tempVertex.specular = m_material->m_specular;

					m_vertices.push_back(tempVertex);

				}
			}
		}
	}

}

