#pragma once

#include "nowork/Common.h"
#include "NoWork/Texture.h"
#include "NoWork/Texture2D.h"
#include "NoWork/Shader.h"

class Material;
typedef std::shared_ptr<Material> MaterialPtr;

class NOWORK_API  Material
{
public:

	Material(std::string name = "", ShaderPtr shader = nullptr);

	void SetCustomTexture(std::string name, TexturePtr texture);
	TexturePtr GetCustomTexture(std::string name);

	void SetCustomValuef(std::string name, float v);
	float GetCustomValuef(std::string name);

	void SetCustomValueVec2(std::string name, glm::vec2 v);
	glm::vec2 GetCustomValueVec2(std::string name);

	void SetCustomValueVec3(std::string name, glm::vec3 v);
	glm::vec3 GetCustomValueVec3(std::string name);

	void SetCustomValueVec4(std::string name, glm::vec4 v);
	glm::vec4 GetCustomValueVec4(std::string name);


	void SetDiffuseTexture(Texture2DPtr tex);
	Texture2DPtr GetDiffuseTexture();
	void SetDiffuseColor(glm::vec4 col);

	void SetNormalTexture(Texture2DPtr tex);
	Texture2DPtr GetNormalTexture();

	void SetMetallicTexture(Texture2DPtr tex);
	Texture2DPtr GetMetallicTexture();
	void SetMetallic(float m);

	void SetRoughnessTexture(Texture2DPtr tex);
	Texture2DPtr GetRoughnessTexture();
	void SetRoughness(float m);

	void SetShader(ShaderPtr shader);
	ShaderPtr GetShader();
	
	glm::vec2 GetTiling() const { return m_Tiling; }
	void Tiling(glm::vec2 val) { m_Tiling = val; }

	void Bind();

	std::string GetName() const { return m_Name; }
	void Name(std::string val) { m_Name = val; }
private:
	Texture2DPtr m_TexDiffuse;
	Texture2DPtr m_TexNormal;
	Texture2DPtr m_TexMetallic;
	Texture2DPtr m_TexRoughness;

	glm::vec4 m_DiffuseColor;
	float m_Metallic, m_Roughness;
	glm::vec2 m_Tiling;
	
	std::unordered_map<std::string, TexturePtr> m_CustomTextures;
	std::unordered_map<std::string, float> m_CustomfloatValues;
	std::unordered_map<std::string, glm::vec2> m_CustomVec2Values;
	std::unordered_map<std::string, glm::vec3> m_CustomVec3Values;
	std::unordered_map<std::string, glm::vec4> m_CustomVec4Values;
	
	ShaderPtr m_Shader;

	std::string m_Name;
	
};