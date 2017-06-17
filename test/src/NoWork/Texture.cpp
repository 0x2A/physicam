#include "nowork/Texture.h"
#include "nowork/NoWork.h"

Renderer* Texture::m_Renderer = NULL;
bool Texture::m_UseDSA;
bool Texture::m_UseTexStorage;
bool Texture::m_UseInternalFormat;

//gl3w is missing this, idk why...
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE

void Texture::Init(NoWork* framework, Renderer* renderer)
{
	m_Renderer = renderer;

	//check if extensions are available
	m_UseDSA = false;//framework->ExtensionAvailable("GL_EXT_direct_state_access"); //looks like some drivers still have problems with this since its very new
	m_UseTexStorage = framework->ExtensionAvailable("GL_ARB_texture_storage"); 
	m_UseInternalFormat = framework->ExtensionAvailable("GL_ARB_internalformat_query2");

}

Texture::Texture(GLuint textType, GLenum texBindingType)
{
	m_TextureType = textType;
	m_TextureBindingType = texBindingType;
	m_TextureId = 0;
}

void Texture::SetParameteri(GLenum pName, GLint param)
{
	if (m_UseDSA) //more performance, if available
	{
		glTextureParameteri(m_TextureId, pName, param);
	}
	else //No direct state access available, use slower method
	{
		GLuint boundTexture = 0;
		glGetIntegerv(m_TextureBindingType, (GLint*)&boundTexture); //get currently bound texture
		glBindTexture(m_TextureType, m_TextureId); //bind to this texture, so we can change state
		glTexParameteri(m_TextureType, pName, param); //change state for this texture
		if(boundTexture != m_TextureId) glBindTexture(m_TextureType, boundTexture); //rebind to old texture again
	}
}

void Texture::SetParameterf(GLenum pName, GLfloat param)
{
	if (m_UseDSA) //more performance, if available
	{
		glTextureParameterf(m_TextureId, pName, param);
	}
	else //No direct state access available, use slower method
	{
		GLuint boundTexture = 0;
		glGetIntegerv(m_TextureBindingType, (GLint*)&boundTexture); //get currently bound texture
		glBindTexture(m_TextureType, m_TextureId); //bind to this texture, so we can change state
		glTexParameterf(m_TextureType, pName, param); //change state for this texture
		if (boundTexture != m_TextureId) glBindTexture(m_TextureType, boundTexture); //rebind to old texture again
	}
}

void Texture::SetParameteriv(GLenum pName, const GLint *param)
{
	if (m_UseDSA) //more performance, if available
	{
		glTextureParameteriv(m_TextureId, pName, param);
	}
	else //No direct state access available, use slower method
	{
		GLuint boundTexture = 0;
		glGetIntegerv(m_TextureBindingType, (GLint*)&boundTexture); //get currently bound texture
		glBindTexture(m_TextureType, m_TextureId); //bind to this texture, so we can change state
		glTexParameteriv(m_TextureType, pName, param); //change state for this texture
		if (boundTexture != m_TextureId) glBindTexture(m_TextureType, boundTexture); //rebind to old texture again
	}
}

void Texture::SetParameterfv(GLenum pName, const GLfloat *param)
{
	if (m_UseDSA) //more performance, if available
	{
		glTextureParameterfv(m_TextureId, pName, param);
	}
	else //No direct state access available, use slower method
	{
		GLuint boundTexture = 0;
		glGetIntegerv(m_TextureBindingType, (GLint*)&boundTexture); //get currently bound texture
		glBindTexture(m_TextureType, m_TextureId); //bind to this texture, so we can change state
		glTexParameterfv(m_TextureType, pName, param); //change state for this texture
		if (boundTexture != m_TextureId) glBindTexture(m_TextureType, boundTexture); //rebind to old texture again
	}
}

void Texture::GenerateTexture()
{
	if (!NoWork::IsMainThread())
	{
		AddToGLQueue(AsyncMode_t::AM_GenerateTexture);
		return;
	}

	glGenTextures(1, &m_TextureId);
	glBindTexture(m_TextureType, m_TextureId);
}

void Texture::Bind(uint32_t slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(m_TextureType, m_TextureId);
}

GLint GetInternalFormatLegacy(GLenum targetFormat)
{
	GLint internalFormat;
	switch (targetFormat)
	{
	case GL_R8:
	case GL_R8_SNORM:
	case GL_R16:
	case GL_R16_SNORM:
	case GL_R16F:
	case GL_R32F:
	case GL_R8I:
	case GL_R8UI:
	case GL_R16I:
	case GL_R16UI:
	case GL_R32I:
	case GL_R32UI:
		internalFormat = GL_RED;
		break;
	case GL_RG8:
	case GL_RG8_SNORM:
	case GL_RG16:
	case GL_RG16F:
	case GL_RG32F:
	case GL_RG8I:
	case GL_RG8UI:
	case GL_RG16I:
	case GL_RG16UI:
	case GL_RG32I:
	case GL_RG32UI:
		internalFormat = GL_RG;
		break;
	case GL_R3_G3_B2:
	case GL_RGB4:
	case GL_RGB5:
	case GL_RGB8:
	case GL_RGB8_SNORM:
	case GL_RGB10:
	case GL_RGB12:
	case GL_RGB16_SNORM:
	case GL_RGBA2:
	case GL_RGBA4:
	case GL_SRGB8:
	case GL_RGB16F:
	case GL_RGB32F:
	case GL_R11F_G11F_B10F:
	case GL_RGB9_E5:
	case GL_RGB8I:
	case GL_RGB8UI:
	case GL_RGB16I:
	case GL_RGB16UI:
	case GL_RGB32I:
	case GL_RGB32UI:
		internalFormat = GL_RGB;
		break;
	case GL_RGB5_A1:
	case GL_RGBA8:
	case GL_RGBA8_SNORM:
	case GL_RGB10_A2:
	case GL_RGB10_A2UI:
	case GL_RGBA12:
	case GL_RGBA16:
	case GL_RGBA16F:
	case GL_RGBA32F:
	case GL_RGBA8I:
	case GL_RGBA8UI:
	case GL_RGBA16I:
	case GL_RGBA16UI:
	case GL_RGBA32I:
	case GL_RGBA32UI:
		internalFormat = GL_RGBA;
	default:
		internalFormat = GL_RGB;
		break;
	}
	return internalFormat;
}

void Texture::GetInternalFormat(GLenum textureType, GLenum targetFormat, GLint* internalFormat, GLint *type)
{
	if (m_UseInternalFormat)
	{
		
		glGetInternalformativ(textureType, targetFormat, GL_TEXTURE_IMAGE_FORMAT, 1, internalFormat);
		glGetInternalformativ(textureType, targetFormat, GL_TEXTURE_IMAGE_TYPE, 1, type);

		if (*internalFormat == 0)
			*internalFormat = GetInternalFormatLegacy(targetFormat);
	}
	else //extension GL_ARB_internalformat_query2 not available, we have to determine the values by ourself (see https://www.opengl.org/sdk/docs/man/html/glTexStorage2D.xhtml#idp50915184)
	{
				
		*internalFormat = GetInternalFormatLegacy(targetFormat);
		*type = GL_UNSIGNED_BYTE; //not shure if this is correct, cant find a table for this tho
	}
}

NOWORK_API void Texture::SetLinearTextureFilter(bool state)
{
	if (!NoWork::IsMainThread())
	{
		AddToGLQueue(AsyncMode_t::AM_SetLinearTextureFilter, reinterpret_cast<void *>(+state));
		return;
	}
	
	if (state)
	{
		float aniFilter = m_Renderer->GetAnisotropicFilterValue();
		if (aniFilter > 0.0f)
		{
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniFilter);
		}
		else
		{
			SetParameteri(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			SetParameteri(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
	}
	else
	{
		SetParameteri(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		SetParameteri(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
}

NOWORK_API void Texture::SetTextureComparison(unsigned int compareMode, unsigned int compareFunc)
{
	if (!NoWork::IsMainThread())
	{
		unsigned int* arr = new unsigned int[2];
		arr[0] = compareMode;
		arr[1] = compareFunc;
		AddToGLQueue(AsyncMode_t::AM_SetTextureComparison, arr);
		return;
	}

	SetParameteri(GL_TEXTURE_COMPARE_MODE, compareMode);
	SetParameteri(GL_TEXTURE_COMPARE_FUNC, compareFunc);
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_TextureId);
}

void Texture::DoAsyncWork(int mode, void *params)
{
	unsigned int* arr;
	switch ((AsyncMode_t)mode)
	{
	case AsyncMode_t::AM_GenerateTexture:
		GenerateTexture();
		break;
	case AsyncMode_t::AM_SetLinearTextureFilter:
		SetLinearTextureFilter((bool)params);
		break;
	case AsyncMode_t::AM_SetTextureComparison:
		arr = static_cast<unsigned int*>(params);
		SetTextureComparison(arr[0], arr[1]);
		break;
	default:
		break;
	}
}


