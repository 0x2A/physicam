#include "nowork/Mesh.h"
#include "nowork/Log.h"
#include "nowork/Renderer.h"
#include "NoWork/Framework.h"

Renderer* Mesh::m_sRenderer = NULL;


Mesh::Mesh() : AsyncGLWorker()
{
	m_VertexArrayObject = 0;
}


Mesh::~Mesh()
{
	m_Vertices.clear();
	m_Faces.clear();

	unsigned int buffers[] = { m_IndexBuffer, m_VertexBuffer, m_NormalBuffer, m_TexCoordBuffer };
	glDeleteBuffers(4, buffers);
	glDeleteVertexArrays(1, &m_VertexArrayObject);
}

MeshPtr Mesh::Create(const VertexList &vertices, const FaceList &faces, bool calculateNormals, DataUsage usage)
{
	LOG_DEBUG("Creating mesh with " << vertices.size() << " vertices");

	MeshPtr mesh = MeshPtr(new Mesh);
	mesh->m_DataUsage = usage;
	mesh->m_Vertices = vertices;
	mesh->m_Renderer = m_sRenderer;
	mesh->m_Faces = faces;
	mesh->m_NumIndices = (unsigned int)faces.size() * 3;
	mesh->m_NumVertices = (unsigned int)vertices.size();

	//if (mesh->m_Faces.size() == 0 && calculateNormals || mesh->m_Faces.size() > 0)
	if (calculateNormals)
		mesh->CalculateNormals();

	mesh->CreateVBO();
	return mesh;
}

NOWORK_API MeshPtr Mesh::Create(const VertexList &vertices, DataUsage usage /*= DataUsage::STATIC_DRAW*/)
{
	LOG_DEBUG("Creating mesh with " << vertices.size() << " vertices");

	MeshPtr mesh = MeshPtr(new Mesh);
	mesh->m_DataUsage = usage;
	mesh->m_Vertices = vertices;
	mesh->m_Renderer = m_sRenderer;
	mesh->m_NumIndices = 0;
	mesh->m_NumVertices = (unsigned int)vertices.size();

	mesh->CreateVBO();
	return mesh;
}


void Mesh::CalculateNormals()
{
	if (m_Faces.size() == 0)
	{
		if (m_Vertices.size() < 3)
		{
			LOG_ERROR("Not enough vertices to calculate normals (min 3 needed)");
			return;
		}

		glm::vec3 normal;
		for (int i = 0; i < m_Vertices.size(); i++)
		{
			normal = glm::cross(m_Vertices[i + 1].position - m_Vertices[i].position, m_Vertices[i + 2].position - m_Vertices[i].position);
			normal = glm::normalize(normal);
			m_Vertices[i].normal = m_Vertices[i + 1].normal = m_Vertices[i + 2].normal = normal;
			i += 2;
		}
	}
}

bool Mesh::CreateVBO()
{
	if (m_Vertices.size() == 0 && !(m_DataUsage == DYNAMIC_COPY || m_DataUsage == DYNAMIC_DRAW ||m_DataUsage == DYNAMIC_READ))
	{
		LOG_ERROR("No vertices defined. Please add some vertices",__FUNCTION__);
		return false;
	}
	if (m_NumIndices == 0)
	{
		//LOG_WARNING("No indices defined. Rendering in array mode (slower)", __FUNCTION__);
	}
	
	if (!NoWork::IsMainThread())
	{
		AddToGLQueue(0);
		return true;
	}

	//generate vertex buffer object
	glGenVertexArrays(1, &m_VertexArrayObject);
	glBindVertexArray(m_VertexArrayObject);

	// buffer for indices
	if (m_NumIndices > 0 || (m_DataUsage == DYNAMIC_COPY || m_DataUsage == DYNAMIC_DRAW || m_DataUsage == DYNAMIC_READ))
	{
		glGenBuffers(1, &m_IndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
		if (m_Faces.size() != 0)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face) * m_Faces.size(), &m_Faces[0], m_DataUsage);
	}

	//buffer for vertices
	glGenBuffers(1, &m_VertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	if (m_Vertices.size() != 0)
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_Vertices.size(), &m_Vertices[0], m_DataUsage);

	
	//setup data locations

	// Positions (location = 0)
	glEnableVertexAttribArray(MODEL_VERTEX_LOCATION);
	glVertexAttribPointer(MODEL_VERTEX_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

	//Normals (location = 1)
	glEnableVertexAttribArray(MODEL_NORMAL_LOCATION);
	glVertexAttribPointer(MODEL_NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));

	//texcoords (location = 2)
	glEnableVertexAttribArray(MODEL_TEXCOORD_LOCATION);
	glVertexAttribPointer(MODEL_TEXCOORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) * 2));

	//vertexColor (location = 3)
	glEnableVertexAttribArray(MODEL_VERTEX_COLOR_LOCATION);
	glVertexAttribPointer(MODEL_VERTEX_COLOR_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) * 2 + sizeof(glm::vec2)));

	glEnableVertexAttribArray(MODEL_TANGENT_LOCATION);
	glVertexAttribPointer(MODEL_TANGENT_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) * 2 + sizeof(glm::vec2) + sizeof(glm::vec4)));

	// unbind buffers
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


	//clear buffer on static draw since we dont need it in system memory anymore
	if (m_DataUsage == STATIC_DRAW)
	{
		m_Vertices.clear();
		m_Faces.clear();
	}

	return true;
}

void Mesh::Render(ShaderPtr shader, RenderMode mode)
{
	if (!shader)
	{
		LOG_ERROR("Invalid shader! Cant render!");
		return;
	}

//	if (m_Vertices.size() == 0)
//		return;

	glBindVertexArray(m_VertexArrayObject);

	glm::mat4 modelMatrix = m_Transform.GetModelMatrix();

	shader->Use();
	shader->SetParameterMat4("ModelMatrix", modelMatrix);
	shader->SetParameterMat4("MVPMatrix", m_Renderer->GetCamera()->GetViewProjectionMatrix()*modelMatrix);
	shader->SetParameterMat4("ModelViewMatrix", m_Renderer->GetCamera()->GetViewMatrix()*modelMatrix);

	if (m_NumIndices > 0)
		glDrawElements(mode, m_NumIndices, GL_UNSIGNED_INT, 0);
	else
		glDrawArrays(mode, 0, m_NumVertices);

	//glBindVertexArray(0);
	Renderer::DrawCalls++;
}

NOWORK_API MeshPtr Mesh::CreatePlane(DataUsage usage /*= DataUsage::STATIC_DRAW*/)
{
	std::vector<Vertex> vertices;
	std::vector<Face> faces;

	Vertex v(glm::vec3(-1, -1, 0), glm::vec3(0, 0, 1), glm::vec3(1,0,0), glm::vec2(0, 0), glm::vec4(1,1,1,1)); vertices.push_back(v);
	v = Vertex(glm::vec3(1, -1, 0), glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec2(1, 0), glm::vec4(1, 1, 1, 1)); vertices.push_back(v);
	v = Vertex(glm::vec3(1, 1, 0), glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec2(1, 1), glm::vec4(1, 1, 1, 1)); vertices.push_back(v);
	v = Vertex(glm::vec3(-1, 1, 0), glm::vec3(0, 0, 1), glm::vec3(1, 0, 0), glm::vec2(0, 1), glm::vec4(1, 1, 1, 1)); vertices.push_back(v);

	faces.push_back(Face(0, 1, 3));
	faces.push_back(Face(1, 2, 3));
	return Create(vertices, faces, false, usage);
}

void Mesh::Init(Renderer* renderer)
{
	m_sRenderer = renderer;
}

void Mesh::UpdateBufferData(bool reallocate)
{
	if (!(m_DataUsage == DYNAMIC_DRAW || m_DataUsage == DYNAMIC_COPY))
		return;

	if (!NoWork::IsMainThread())
	{
		AddToGLQueue(1, reinterpret_cast<void*>(+reallocate));
		return;
	}

	if (m_Vertices.size() == 0) return;

	m_NumIndices = m_Faces.size()*3;
	m_NumVertices = m_Vertices.size();

	glBindVertexArray(m_VertexArrayObject);
	if (m_NumIndices > 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
		if (reallocate)
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face) * m_Faces.size(), &m_Faces[0], m_DataUsage);
		else
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(Face) * m_Faces.size(), &m_Faces[0]);
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);

	if (reallocate)
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_Vertices.size(), &m_Vertices[0], m_DataUsage);
	else
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * m_Vertices.size(), &m_Vertices[0]);

	
}

void Mesh::DoAsyncWork(int mode, void *params)
{
	switch (mode)
	{
	case 0:
		CreateVBO();
		break;
	case 1:
		UpdateBufferData((bool)(params));
		break;
	}
}

