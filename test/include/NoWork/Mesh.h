#pragma once

#include "nowork/Common.h"
#include "nowork/Transform.h"
#include "nowork/Shader.h"
#include "NoWork/AsyncGLWorker.h"

#define MODEL_VERTEX_LOCATION 0
#define MODEL_NORMAL_LOCATION 1
#define MODEL_TEXCOORD_LOCATION 2
#define MODEL_VERTEX_COLOR_LOCATION 3
#define MODEL_TANGENT_LOCATION 4
#define MODEL_BITANGENT_LOCATION 5
#define MODEL_MATERIAL_LOCATION 6

class Renderer;
class Vertex
{
public:
	Vertex(glm::vec3 _position, glm::vec3 _normal, glm::vec2 _texCoord) :
		position(_position), normal(_normal), texCoord(_texCoord)
	{
		if (glm::length(normal) != 0)
			glm::normalize(normal);

		color = glm::vec4(1, 1, 1, 1);
	}

	Vertex(glm::vec3 _position, glm::vec2 _texCoord) : Vertex(_position, glm::vec3(), _texCoord)
	{}

	Vertex(glm::vec3 _position) : Vertex(_position, glm::vec2())
	{}

	Vertex(glm::vec3 _position, glm::vec3 _normal, glm::vec3 _tangent, glm::vec2 _texCoord, glm::vec4 _color) :Vertex(_position, _normal, _texCoord)
	{
		tangent = _tangent;
		color = _color;
	}

	Vertex() : position(0), normal(0), tangent(0), texCoord(0)
	{
		color = glm::vec4(1, 1, 1, 1);
	}
	
	glm::vec3 position; //Position of the vertex
	glm::vec3 normal; //Normal of the vertex
	glm::vec2 texCoord; //Texture coordinate of the vertex
	glm::vec4 color; //Color of the vertex
	glm::vec3 tangent; //tangent of the vertex
private:
};

class Face
{
public:
	Face(unsigned int a, unsigned int b, unsigned int c)
	{
		Indices[0] = a;
		Indices[1] = b;
		Indices[2] = c;
	}
	unsigned int Indices[3];
};

//typedefs for easier usage

//list of vertices for mesh creation
typedef std::vector<Vertex> VertexList;

//list of faces (indices) for mesh creation
typedef std::vector<Face> FaceList;

class Mesh;
typedef std::shared_ptr<Mesh> MeshPtr;

class Mesh : public AsyncGLWorker
{
	friend class NoWork;
	friend class Renderer;
public:
	enum DataUsage
	{
		STREAM_DRAW = 0x88E0,
		STREAM_READ = 0x88E1,
		STREAM_COPY = 0x88E2,
		STATIC_DRAW = 0x88E4,
		STATIC_READ = 0x88E5,
		STATIC_COPY = 0x88E6,
		DYNAMIC_DRAW = 0x88E8,
		DYNAMIC_READ = 0x88E9,
		DYNAMIC_COPY = 0x88EA
	};

	enum RenderMode
	{
		POINTS   	   = 0x0000,
		LINES          = 0x0001,
		LINE_LOOP      = 0x0002,
		LINE_STRIP     = 0x0003,
		TRIANGLES      = 0x0004,
		TRIANGLE_STRIP = 0x0005,
		TRIANGLE_FAN   = 0x0006,
	};

	NOWORK_API ~Mesh();


	//Creates a mesh without indices. The mesh must be triangles
	NOWORK_API static MeshPtr Create(const VertexList &vertices, DataUsage usage = DataUsage::STATIC_DRAW);

	//Creates a mesh with indices. The mesh must be triangles
	NOWORK_API static MeshPtr Create(const VertexList &vertices, const FaceList &faces, bool calculateNormals = false, DataUsage usage = DataUsage::STATIC_DRAW);
	NOWORK_API static MeshPtr CreatePlane(DataUsage usage = DataUsage::STATIC_DRAW);

	NOWORK_API void Render(ShaderPtr shader, RenderMode mode = TRIANGLES);
	NOWORK_API Transform* GetTransform() {return &m_Transform; }

	NOWORK_API VertexList* GetVertexList() { return &m_Vertices; }
	NOWORK_API FaceList* GetFacesList() { return &m_Faces; }

	NOWORK_API void UpdateBufferData(bool reallocate = false);

protected:
	static void Init(Renderer* renderer);
	Mesh();

	bool CreateVBO();

private:
	void CalculateNormals();

	virtual void DoAsyncWork(int mode, void *params) override;



	Renderer* m_Renderer;
	static Renderer* m_sRenderer;

	unsigned int m_NumIndices, m_NumVertices;

	DataUsage m_DataUsage;
	std::vector<Vertex> m_Vertices;
	std::vector<Face> m_Faces;

	unsigned int m_VertexArrayObject;
	unsigned int m_IndexBuffer, m_VertexBuffer, m_NormalBuffer, m_TexCoordBuffer;

	Transform m_Transform;

};