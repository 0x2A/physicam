
#include "NoWork/RenderTexture.h"
#include "NoWork/Framework.h"


RenderTexturePtr RenderTexture::Create(int width, int height, Type type, Texture::Format textureFormat, bool compressed /*= false*/)
{
	unsigned int tType = type;
	if (textureFormat == GL_DEPTH_COMPONENT || textureFormat == GL_DEPTH_STENCIL)
	{
		tType = GL_TEXTURE_2D;
	}


	unsigned int bindingType;
	//get binding type
	switch (tType)
	{
	case GL_TEXTURE_1D:
		bindingType = GL_TEXTURE_BINDING_1D;
		break;
	case GL_TEXTURE_3D:
		bindingType = GL_TEXTURE_BINDING_3D;
		break;
	case GL_TEXTURE_2D:
	default:
		bindingType = GL_TEXTURE_BINDING_2D;
		break;
	}

	RenderTexturePtr tex = RenderTexturePtr(new RenderTexture(tType, bindingType));
	tex->m_Width = width;
	tex->m_Height = height;
	tex->m_Format = textureFormat;

	if (!NoWork::IsMainThread())
		tex->AddToGLQueue(0, nullptr);
	else
		tex->Generate();
	return tex;
}

RenderTexture::RenderTexture(unsigned int texType, unsigned int texBindingType) : Texture(texType, texBindingType)
{}

void RenderTexture::DoAsyncWork(int mode, void *params)
{
	switch (mode)
	{
	case 0:
		Generate();
		break;
	}
}

void RenderTexture::Generate()
{
	glGenTextures(1, &m_TextureId);
	glBindTexture(m_TextureType, m_TextureId);

	glTexParameteri(m_TextureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(m_TextureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameterf(m_TextureType, GL_TEXTURE_WRAP_S, 0x812F);
	glTexParameterf(m_TextureType, GL_TEXTURE_WRAP_T, 0x812F);

	int texIntFrmt = m_Format;
	if (m_Format == GL_DEPTH_COMPONENT || m_Format == GL_DEPTH_STENCIL)
	{
		// This is to allow usage of shadow2DProj function in the shader
		//glTexParameteri(m_TextureType, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		//glTexParameteri(m_TextureType, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		m_Type = GL_UNSIGNED_BYTE;
	}
	else
		GetInternalFormat(m_TextureType, m_Format, &texIntFrmt, &m_Type);
	glTexImage2D(m_TextureType, 0, m_Format, m_Width, m_Height, 0, texIntFrmt, m_Type, 0);
	m_Format = (Texture::Format)texIntFrmt;
	m_InternalFormat = texIntFrmt;
}


