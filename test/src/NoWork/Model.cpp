#include "NoWork/Model.h"
#include "NoWork/Log.h"
#include "NoWork/FileSystem.h"


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


Model::Model()
{
	m_TotalVertices = 0;
}


ModelPtr Model::Load(const std::string path, Mesh::DataUsage usage)
{
	LOG_DEBUG("Loading model '" << path << "'...");

	
	double startTime = glfwGetTime();
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path,
		
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType | 
		aiProcess_PreTransformVertices | 
		aiProcess_SplitLargeMeshes |
		aiProcess_FindInvalidData | aiProcess_GenUVCoords | aiProcess_FlipUVs);

	
	if (!scene)
	{
		LOG_ERROR("Unable to load Model '" << path << "': " << importer.GetErrorString());
		return nullptr;
	}

	const int iVertexTotalSize = sizeof(aiVector3D) * 2 + sizeof(aiVector2D);

	int iTotalVertices = 0;

	ModelPtr model = ModelPtr(new Model);
	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		VertexList vertices;
		FaceList faces;

		aiMesh* mesh = scene->mMeshes[i];

		//get indices/faces
		int iMeshFaces = mesh->mNumFaces;
		model->m_MaterialIndices.push_back(mesh->mMaterialIndex);
		for (int j = 0; j < iMeshFaces; j++)
		{
			const aiFace& face = mesh->mFaces[j];
			faces.push_back(Face(face.mIndices[0], face.mIndices[1], face.mIndices[2]));
		}

		//get vertices
		vertices.reserve(mesh->mNumVertices);
		bool hasNormals = mesh->HasNormals();
		bool hasTangent = mesh->HasTangentsAndBitangents();
		bool hasUvCoords = mesh->mNumUVComponents[0] > 0;
		for (int k = 0; k < mesh->mNumVertices; k++)
		{
			aiVector3D pos = mesh->mVertices[k];
			aiVector3D uv = hasUvCoords ? mesh->mTextureCoords[0][k] : aiVector3D(0,0,0);
			aiVector3D normal = hasNormals ? mesh->mNormals[k] : aiVector3D(1.0f, 1.0f, 1.0f);
			aiVector3D tangent = hasTangent ? mesh->mTangents[k] : aiVector3D(1.0f, 0.0f, 0.0f);
			aiColor4D color = mesh->HasVertexColors(0) ? mesh->mColors[0][k] : aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);

			Vertex v = Vertex(glm::vec3(pos.x, pos.y, pos.z), glm::vec3(normal.x, normal.y, normal.z), 
				glm::vec3(tangent.x, tangent.y, tangent.z), glm::vec2(uv.x, uv.y), glm::vec4(color.r, color.g, color.b, color.a));

			vertices.push_back(v);
		}
		MeshPtr m = Mesh::Create(vertices, faces, false, usage);
		if (!m)
		{
			LOG_ERROR("Unable to load submesh '" << mesh->mName.C_Str() << "' from model '" << path << "'");
			continue;
		}
		model->m_Meshes.push_back(m);
		model->m_TotalVertices += mesh->mNumVertices;
	}

	int iNumMaterials = scene->mNumMaterials;

	std::string basePath = FileSystem::GetPath(path);

	std::map<std::string, TexturePtr> loadedTextures;
	for (int i = 0; i < iNumMaterials; i++)
	{
		const aiMaterial* material = scene->mMaterials[i];
		
		int a = 5;
		int texIndex = 0;
		aiString texPath;  // filename
		std::string path;

		aiString matName;
		material->Get(AI_MATKEY_NAME, matName);
		MaterialPtr mat = std::make_shared<Material>(matName.C_Str(), nullptr);
		if (material->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath) == AI_SUCCESS)
		{
			path = basePath + "/" + texPath.data;
			Texture2DPtr tex(nullptr);
			if (!loadedTextures[path])
			{
				tex = Texture2D::Load(path);
				loadedTextures[path] = tex;
			}
			else
				tex = std::dynamic_pointer_cast<Texture2D>(loadedTextures[path]);
			mat->SetDiffuseTexture(tex);
		}
		if (material->GetTexture(aiTextureType_NORMALS, texIndex, &texPath) == AI_SUCCESS)
		{
			path = basePath + "/" + texPath.data;
			Texture2DPtr tex(nullptr);
			if (!loadedTextures[path])
			{
				tex = Texture2D::Load(path);
				loadedTextures[path] = tex;
			}
			else
				tex = std::dynamic_pointer_cast<Texture2D>(loadedTextures[path]);
			mat->SetNormalTexture(tex);
		}
		if (material->GetTexture(aiTextureType_SHININESS, texIndex, &texPath) == AI_SUCCESS)
		{
			path = basePath + "/" + texPath.data;
			Texture2DPtr tex(nullptr);
			if (!loadedTextures[path])
			{
				tex = Texture2D::Load(path);
				loadedTextures[path] = tex;
			}
			else
				tex = std::dynamic_pointer_cast<Texture2D>(loadedTextures[path]);
			mat->SetRoughnessTexture(tex);
		}
		if (material->GetTexture(aiTextureType_SPECULAR, texIndex, &texPath) == AI_SUCCESS)
		{
			path = basePath + "/" + texPath.data;
			Texture2DPtr tex(nullptr);
			if (!loadedTextures[path])
			{
				tex = Texture2D::Load(path);
				loadedTextures[path] = tex;
			}
			else
				tex = std::dynamic_pointer_cast<Texture2D>(loadedTextures[path]);
			mat->SetMetallicTexture(tex);
		}
		aiColor3D col;
		material->Get(AI_MATKEY_COLOR_DIFFUSE, col);
		float fval;
		material->Get(AI_MATKEY_OPACITY, fval);
		mat->SetDiffuseColor(glm::vec4(col.r, col.g, col.b, fval));

		material->Get(AI_MATKEY_SHININESS, fval);
		mat->SetRoughness(1.0f - fval);
		
		material->Get(AI_MATKEY_SHININESS_STRENGTH, fval);
		mat->SetMetallic(1.0f - fval);
		
		model->m_Materials.push_back(mat);
		
	}

	double endTime = glfwGetTime();

	LOG_DEBUG("Model loaded: " << model->m_TotalVertices << " vertices total.");
	LOG_DEBUG("Loading model took " << endTime - startTime << " seconds");

	return model;
}

Model::~Model()
{
	m_Meshes.clear();

	m_Materials.clear();
	m_MaterialIndices.clear();
}

MeshPtr Model::GetSubmesh(size_t index)
{
	if (index >= m_Meshes.size()) return nullptr;
	return m_Meshes[index];
}

void Model::Render(ShaderPtr shader)
{
	shader->Use();
	bool update = false;
	if (m_LastShader != shader)
	{
		m_LastShader = shader;
		update = true;
	}
	for (int i = 0; i < m_Meshes.size(); i++)
	{
		if (m_Materials[m_MaterialIndices[i]])
		{
			if (update) m_Materials[m_MaterialIndices[i]]->SetShader(shader);
			m_Materials[m_MaterialIndices[i]]->Bind();
		}

		m_Meshes[i]->GetTransform()->SetModelMatrix(m_Transform.GetModelMatrix());
		m_Meshes[i]->Render(shader);
	}
}

NOWORK_API void Model::ReplaceMaterial(int id, MaterialPtr tex)
{
	if (id < m_Materials.size())
	{
		m_Materials[id] = tex;
	}
}

NOWORK_API MaterialPtr Model::GetMaterial(int meshId)
{
	return m_Materials[m_MaterialIndices[meshId]];
}
