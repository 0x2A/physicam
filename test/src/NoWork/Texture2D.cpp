#include "nowork/Texture2D.h"
#include "nowork/Log.h"
#include "soil/SOIL.h"
#include "NoWork/NoWork.h"

Texture2D::Texture2D() : Texture(GL_TEXTURE_2D, GL_TEXTURE_BINDING_2D)
{

}

NOWORK_API  Texture2DPtr Texture2D::Create(unsigned int width, unsigned int height, Texture::Format format, unsigned char *pixels, bool createMipMaps, bool constant /*= true*/)
{
	Texture2DPtr tex = Texture2DPtr(new Texture2D);
	tex->GenerateTexture();  //generate and bind new texture

	tex->m_Format = format;
	tex->m_Width = width;
	tex->m_Height = height;
	tex->m_Constant = constant;
	tex->m_Pixels = pixels;
	tex->m_CreateMipMaps = createMipMaps;
	tex->CopyPixelData();

	return tex;
}

Texture2DPtr Texture2D::Load(const std::string& path, bool createMipMaps, bool constant /*= true*/)
{
	int width, height, channels;
	unsigned char *ht_map = SOIL_load_image(path.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);

	if (!ht_map)
	{
		LOG_ERROR("Unable to load texture '" << path << "': " << SOIL_last_result());
		return nullptr;
	}

	Texture::Format format;
	switch (channels)
	{
	case 1:
		format = Texture::R8;
		break;
	case 2:
		format = Texture::RG8;
		break;
	case 3:
		format = Texture::RGB8;
		break;
	case 4:
		format = Texture::RGBA8;
		break;
	}
	return Create(width, height, format, ht_map, createMipMaps, constant);
}

void Texture2D::Update(const unsigned char* pixels)
{
	Update(pixels, m_Width, m_Height);
}

void Texture2D::Update(const unsigned char* pixels, int width, int height)
{
	if (m_UseTexStorage && m_Constant)
	{
		LOG_WARNING("Unable to update texture: Texture is set to constant and cant be changed");
		return;
	}

	glBindTexture(GL_TEXTURE_2D, m_TextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, m_Format, width, height, 0, m_InternalFormat, m_Type, pixels);
}

void Texture2D::DoAsyncWork(int mode, void *params)
{
	Texture::DoAsyncWork(mode, params);

	switch ((AsyncMode_t)mode)
	{
	case Texture2D::AM_CopyPixData:
		CopyPixelData();
		break;
	}
}

void Texture2D::CopyPixelData()
{
	if (!NoWork::IsMainThread())
	{
		AddToGLQueue(AsyncMode_t::AM_CopyPixData);
		return;
	}

	GetInternalFormat(GL_TEXTURE_2D, m_Format, &m_InternalFormat, &m_Type); //get internal base format and data type for target texture format

	glBindTexture(GL_TEXTURE_2D, m_TextureId);

	if (m_CreateMipMaps)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	if (m_UseTexStorage && m_Constant) //static and extension available? Use faster method
	{
		glTexStorage2D(GL_TEXTURE_2D, 1, m_Format, m_Width, m_Height);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Width, m_Height, m_InternalFormat, m_Type, m_Pixels);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, m_Format, m_Width, m_Height, 0, m_InternalFormat, m_Type, m_Pixels);
	}

	if (m_CreateMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	SetLinearTextureFilter(true); //Set texture filtering to linear

	SOIL_free_image_data(m_Pixels);
}
