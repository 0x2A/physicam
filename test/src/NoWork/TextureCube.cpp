
#include "NoWork/TextureCube.h"
#include "NoWork/Log.h"
#include "soil/SOIL.h"
#include "NoWork/Framework.h"

TextureCube::TextureCube() : Texture(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BINDING_CUBE_MAP)
{

}

TextureCubePtr TextureCube::Create(unsigned int width, unsigned int height, Texture::Format format, CubeMapPixelData pixels, bool constant /*= true*/)
{
	TextureCubePtr tex = TextureCubePtr(new TextureCube);
	tex->GenerateTexture();  //generate and bind new texture

	tex->m_Format = format;
	tex->m_Width = width;
	tex->m_Height = height;
	tex->m_Constant = constant;
	tex->m_Pixels = pixels;
	tex->CopyPixelData();

	return tex;
}

TextureCubePtr TextureCube::Load(const std::string& pathPosX, const std::string& pathNegX, 
	const std::string& pathPosY, const std::string& pathNegY, 
	const std::string& pathPosZ, const std::string& pathNegZ, bool constant /*= true*/)
{
	int width, height, channels;
	CubeMapPixelData pixdata;

	pixdata.posX = SOIL_load_image(pathPosX.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);
	if (!pixdata.posX)
	{
		LOG_ERROR("Unable to load cubemap texture '" << pathPosX << "': " << SOIL_last_result());
		return nullptr;
	}
	pixdata.negX = SOIL_load_image(pathNegX.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);
	if (!pixdata.negX)
	{
		LOG_ERROR("Unable to load cubemap texture '" << pathNegX << "': " << SOIL_last_result());
		DelPtr(pixdata.posX);
		return nullptr;
	}
	pixdata.posY = SOIL_load_image(pathPosY.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);
	if (!pixdata.posY)
	{
		LOG_ERROR("Unable to load cubemap texture '" << pathPosY << "': " << SOIL_last_result());
		DelPtr(pixdata.posX);
		DelPtr(pixdata.negX);
		return nullptr;
	}
	pixdata.negY = SOIL_load_image(pathNegY.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);
	if (!pixdata.negY)
	{
		LOG_ERROR("Unable to load cubemap texture '" << pathNegY << "': " << SOIL_last_result());
		DelPtr(pixdata.posX);
		DelPtr(pixdata.negX);
		DelPtr(pixdata.posY);
		return nullptr;
	}
	pixdata.posZ = SOIL_load_image(pathPosZ.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);
	if (!pixdata.posZ)
	{
		LOG_ERROR("Unable to load cubemap texture '" << pathPosZ << "': " << SOIL_last_result());
		DelPtr(pixdata.posX);
		DelPtr(pixdata.negX);
		DelPtr(pixdata.posY);
		DelPtr(pixdata.negY);
		return nullptr;
	}
	pixdata.negZ = SOIL_load_image(pathNegZ.c_str(), &width, &height, &channels, SOIL_LOAD_AUTO);
	if (!pixdata.negZ)
	{
		LOG_ERROR("Unable to load cubemap texture '" << pathNegZ << "': " << SOIL_last_result());
		DelPtr(pixdata.posX);
		DelPtr(pixdata.negX);
		DelPtr(pixdata.posY);
		DelPtr(pixdata.negY);
		DelPtr(pixdata.posZ);
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
	return Create(width, height, format, pixdata, constant);
}

void TextureCube::Update(const CubeMapPixelData& pixels)
{
	Update(pixels, m_Width, m_Height);
}

void TextureCube::Update(const CubeMapPixelData& pixels, int width, int height)
{
	if (m_UseTexStorage && m_Constant)
	{
		LOG_WARNING("Unable to update texture: Texture is set to constant and cant be changed");
		return;
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureId);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, m_Format, width, height, 0, m_InternalFormat, m_Type, pixels.posX);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, m_Format, width, height, 0, m_InternalFormat, m_Type, pixels.negX);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, m_Format, width, height, 0, m_InternalFormat, m_Type, pixels.posY);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, m_Format, width, height, 0, m_InternalFormat, m_Type, pixels.negY);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, m_Format, width, height, 0, m_InternalFormat, m_Type, pixels.posZ);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, m_Format, width, height, 0, m_InternalFormat, m_Type, pixels.negZ);
}

void TextureCube::CopyPixelData()
{
	if (!NoWork::IsMainThread())
	{
		AddToGLQueue(AsyncMode_t::AM_CopyPixData);
		return;
	}

	GetInternalFormat(GL_TEXTURE_CUBE_MAP, m_Format, &m_InternalFormat, &m_Type); //get internal base format and data type for target texture format

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_TextureId);


	if (m_UseTexStorage && m_Constant) //static and extension available? Use faster method
	{
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, 12, m_Format, m_Width, m_Height);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 0, 0, m_Width, m_Height, m_InternalFormat, m_Type, m_Pixels.posX);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 0, 0, m_Width, m_Height, m_InternalFormat, m_Type, m_Pixels.negX);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 0, 0, m_Width, m_Height, m_InternalFormat, m_Type, m_Pixels.posY);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 0, 0, m_Width, m_Height, m_InternalFormat, m_Type, m_Pixels.negY);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, 0, 0, m_Width, m_Height, m_InternalFormat, m_Type, m_Pixels.posZ);
		glTexSubImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, 0, 0, m_Width, m_Height, m_InternalFormat, m_Type, m_Pixels.negZ);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 10);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, m_Format, m_Width, m_Height, 0, m_InternalFormat, m_Type, m_Pixels.posX);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, m_Format, m_Width, m_Height, 0, m_InternalFormat, m_Type, m_Pixels.negX);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, m_Format, m_Width, m_Height, 0, m_InternalFormat, m_Type, m_Pixels.posY);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, m_Format, m_Width, m_Height, 0, m_InternalFormat, m_Type, m_Pixels.negY);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, m_Format, m_Width, m_Height, 0, m_InternalFormat, m_Type, m_Pixels.posZ);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, m_Format, m_Width, m_Height, 0, m_InternalFormat, m_Type, m_Pixels.negZ);
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	SetLinearTextureFilter(true); //Set texture filtering to linear

	SOIL_free_image_data(m_Pixels.posX);
	SOIL_free_image_data(m_Pixels.negX);
	SOIL_free_image_data(m_Pixels.posY);
	SOIL_free_image_data(m_Pixels.negY);
	SOIL_free_image_data(m_Pixels.posZ);
	SOIL_free_image_data(m_Pixels.negZ);
}

void TextureCube::DoAsyncWork(int mode, void *params)
{
	Texture::DoAsyncWork(mode, params);

	switch ((AsyncMode_t)mode)
	{
	case TextureCube::AM_CopyPixData:
		CopyPixelData();
		break;
	}
}
