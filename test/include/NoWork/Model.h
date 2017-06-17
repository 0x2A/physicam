#pragma once

#include "NoWork/Common.h"
#include "NoWork/SceneObject.h"
#include "NoWork/Material.h"
#include "NoWork/Mesh.h"
#include <memory>

class Model;
typedef std::shared_ptr<Model> ModelPtr;

class Model : public SceneObject
{

public:

	NOWORK_API ~Model();
	NOWORK_API static ModelPtr Load(const std::string path, Mesh::DataUsage usage = Mesh::DataUsage::STATIC_DRAW);

	NOWORK_API MeshPtr GetSubmesh(size_t index);

	NOWORK_API int NumSubmeshes() { return m_Meshes.size(); }

	NOWORK_API int SubmeshMaterialIndex(int meshId){ return m_MaterialIndices[meshId]; }

	NOWORK_API MaterialPtr GetMaterial(int meshId);
	
	NOWORK_API void Render(ShaderPtr shader);

	NOWORK_API void ReplaceMaterial(int id, MaterialPtr tex);

protected:

	Model();

private:

	std::vector<MeshPtr> m_Meshes;
	std::vector<MaterialPtr> m_Materials;
	std::vector<int> m_MaterialIndices;
	int m_TotalVertices;

	ShaderPtr m_LastShader;
};