
#include "NoWork/Material.h"

Material::Material(std::string name, ShaderPtr shader)
{
	m_Name = name;
	m_Shader = shader;
	m_Tiling = glm::vec2(1, 1);
	m_Roughness = 1;
}

void Material::SetCustomTexture(std::string name, TexturePtr texture)
{
	m_CustomTextures[name] = texture;
}

void Material::SetCustomValuef(std::string name, float v)
{
	m_CustomfloatValues[name] = v;
}

float Material::GetCustomValuef(std::string name)
{
	return m_CustomfloatValues[name];
}

void Material::SetCustomValueVec2(std::string name, glm::vec2 v)
{
	m_CustomVec2Values[name] = v;
}

glm::vec2 Material::GetCustomValueVec2(std::string name)
{
	return m_CustomVec2Values[name];
}

void Material::SetCustomValueVec3(std::string name, glm::vec3 v)
{
	m_CustomVec3Values[name] = v;
}

glm::vec3 Material::GetCustomValueVec3(std::string name)
{
	return m_CustomVec3Values[name];
}

void Material::SetCustomValueVec4(std::string name, glm::vec4 v)
{
	m_CustomVec4Values[name] = v;
}

glm::vec4 Material::GetCustomValueVec4(std::string name)
{
	return m_CustomVec4Values[name];
}

TexturePtr Material::GetCustomTexture(std::string name)
{
	return m_CustomTextures[name];
}


void Material::SetDiffuseColor(glm::vec4 col)
{
	m_DiffuseColor = col;
}

void Material::Bind()
{
	if (!m_Shader) return;

	//do we have textures?
	m_Shader->Use();

	m_Shader->SetParameterVec4("hasTexture", glm::vec4((bool)m_TexDiffuse, (bool)m_TexNormal,
		(bool)m_TexMetallic, (bool)m_TexRoughness));

	//bind all parameters
	if (m_TexDiffuse)m_Shader->SetParameterTexture("DiffuseTexture", m_TexDiffuse, 0);
	if (m_TexNormal)m_Shader->SetParameterTexture("NormalTexture", m_TexNormal, 1);
	if (m_TexMetallic)m_Shader->SetParameterTexture("MetallicTexture", m_TexMetallic, 2);
	if (m_TexRoughness)m_Shader->SetParameterTexture("RoughnessTexture", m_TexRoughness, 3);
	
	m_Shader->SetParameterVec4("DiffuseColor", m_DiffuseColor);
	m_Shader->SetParameterf("Metallic", m_Metallic);
	m_Shader->SetParameterf("Roughness", m_Roughness);
	m_Shader->SetParameterVec2("Tiling", m_Tiling);

	int i = 4;
	for (auto &tex : m_CustomTextures)
	{
		m_Shader->SetParameterTexture(tex.first, tex.second, i);
		i++;
	}
	for (auto &v : m_CustomfloatValues)
	{
		m_Shader->SetParameterf(v.first, v.second);
	}
	for (auto &v : m_CustomVec2Values)
	{
		m_Shader->SetParameterVec2(v.first, v.second);
	}
	for (auto &v : m_CustomVec3Values)
	{
		m_Shader->SetParameterVec3(v.first, v.second);
	}
	for (auto &v : m_CustomVec4Values)
	{
		m_Shader->SetParameterVec4(v.first, v.second);
	}
}

void Material::SetDiffuseTexture(Texture2DPtr tex)
{
	m_TexDiffuse = tex;
}

Texture2DPtr Material::GetDiffuseTexture()
{
	return m_TexDiffuse;
}

void Material::SetNormalTexture(Texture2DPtr tex)
{
	m_TexNormal = tex;
}

Texture2DPtr Material::GetNormalTexture()
{
	return m_TexNormal;
}

void Material::SetMetallicTexture(Texture2DPtr tex)
{
	m_TexMetallic = tex;
}

Texture2DPtr Material::GetMetallicTexture()
{
	return m_TexMetallic;
}

void Material::SetMetallic(float m)
{
	m_Metallic = m;
}

void Material::SetRoughnessTexture(Texture2DPtr tex)
{
	m_TexRoughness = tex;
}

Texture2DPtr Material::GetRoughnessTexture()
{
	return m_TexRoughness;
}

void Material::SetRoughness(float m)
{
	m_Roughness = m;
}

void Material::SetShader(ShaderPtr shader)
{
	m_Shader = shader;
}

ShaderPtr Material::GetShader()
{
	return m_Shader;
}

