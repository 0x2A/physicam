/*
	PhysiCam - Physically based camera
	Copyright (C) 2015 Frank Köhnke

	This file is part of PhysiCam.

	This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation; either 
	version 3 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 *	@file RenderTexture.cpp
 */


#include <physicam/RenderTexture.h>
#include <physicam/physicam_gl.h>

#include <gl/glew.h>

namespace PhysiCam
{


	RenderTexturePtr RenderTexture::Create(int width, int height, RenderTexture::Type type, RenderTexture::Format textureFormat, bool compressed /*= false*/, bool genMipMaps /*= false*/)
	{
		RenderTexturePtr tex = RenderTexturePtr(new RenderTexture());
		tex->m_Size = glm::ivec2(width,height);
		tex->m_Type = type;
		tex->m_Format = textureFormat;
		tex->GenerateTexture(textureFormat, genMipMaps);
		return tex;
	}

	RenderTexture::RenderTexture()
	{
		m_TextureId = 0;
	}

	RenderTexture::~RenderTexture()
	{
		glDeleteTextures(1, &m_TextureId);
	}

	void RenderTexture::SetParameteri(unsigned int pName, int param)
	{
		if (GL::HasDirectStateAccess) //more performance, if available
		{
			glTextureParameteri(m_TextureId, pName, param);
		}
		else //No direct state access available, use slower method
		{
			GLuint boundTexture = 0;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture); //get currently bound texture
			glBindTexture(GL_TEXTURE_2D, m_TextureId); //bind to this texture, so we can change state
			glTexParameteri(GL_TEXTURE_2D, pName, param); //change state for this texture
			if (boundTexture != m_TextureId) glBindTexture(GL_TEXTURE_2D, boundTexture); //rebind to old texture again
		}
	}

	void RenderTexture::SetParameteriv(unsigned int pName, const int *param)
	{
		if (GL::HasDirectStateAccess) //more performance, if available
		{
			glTextureParameteriv(m_TextureId, pName, param);
		}
		else //No direct state access available, use slower method
		{
			GLuint boundTexture = 0;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture); //get currently bound texture
			glBindTexture(GL_TEXTURE_2D, m_TextureId); //bind to this texture, so we can change state
			glTexParameteriv(GL_TEXTURE_2D, pName, param); //change state for this texture
			if (boundTexture != m_TextureId) glBindTexture(GL_TEXTURE_2D, boundTexture); //rebind to old texture again
		}
	}

	void RenderTexture::SetParameterf(unsigned int pName, float param)
	{
		if (GL::HasDirectStateAccess) //more performance, if available
		{
			glTextureParameterf(m_TextureId, pName, param);
		}
		else //No direct state access available, use slower method
		{
			GLuint boundTexture = 0;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture); //get currently bound texture
			glBindTexture(GL_TEXTURE_2D, m_TextureId); //bind to this texture, so we can change state
			glTexParameterf(GL_TEXTURE_2D, pName, param); //change state for this texture
			if (boundTexture != m_TextureId) glBindTexture(GL_TEXTURE_2D, boundTexture); //rebind to old texture again
		}
	}

	void RenderTexture::SetParameterfv(unsigned int pName, const float *param)
	{
		if (GL::HasDirectStateAccess) //more performance, if available
		{
			glTextureParameterfv(m_TextureId, pName, param);
		}
		else //No direct state access available, use slower method
		{
			GLuint boundTexture = 0;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, (GLint*)&boundTexture); //get currently bound texture
			glBindTexture(GL_TEXTURE_2D, m_TextureId); //bind to this texture, so we can change state
			glTexParameterfv(GL_TEXTURE_2D, pName, param); //change state for this texture
			if (boundTexture != m_TextureId) glBindTexture(GL_TEXTURE_2D, boundTexture); //rebind to old texture again
		}
	}

	void RenderTexture::Bind(uint32_t slot /*= 0*/)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_TextureId);
	}

	void RenderTexture::GetInternalFormat(unsigned int textureType, unsigned int targetFormat, int* internalFormat, int *type)
	{
		if (GL::HasInternalFormatQuery)
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

	int RenderTexture::GetInternalFormatLegacy(unsigned int targetFormat)
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

	void RenderTexture::SetLinearTextureFilter(bool state, float anisotropy)
	{
		if (state)
		{
			if (GL::HasAnisotropicFiltering)
			{
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
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


	void RenderTexture::GenerateTexture(RenderTexture::Format textureFormat, bool genMipMaps)
	{

		glGenTextures(1, &m_TextureId);
		glBindTexture(GL_TEXTURE_2D, m_TextureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, genMipMaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 0x812F);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, 0x812F);

		RenderTexture::Format texIntFrmt = textureFormat;
		if (textureFormat == RenderTexture::DEPTH || textureFormat ==  RenderTexture::DEPTH_STENCIL)
		{
			// This is to allow usage of shadow2DProj function in the shader
			//glTexParameteri(m_TextureType, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			//glTexParameteri(m_TextureType, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
			m_Type = GL_UNSIGNED_BYTE;
		}
		else
			GetInternalFormat(GL_TEXTURE_2D, textureFormat, (int*)&texIntFrmt, &m_Type);
		glTexImage2D(GL_TEXTURE_2D, 0, m_Format, m_Size.x, m_Size.y, 0, texIntFrmt, m_Type, 0);
		if (genMipMaps)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		m_Format = (RenderTexture::Format)texIntFrmt;
		m_InternalFormat = texIntFrmt;
	}

	bool RenderTexture::AttachToFramebuffer(unsigned int FramebufferId, unsigned int attachementPoint)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachementPoint, GL_TEXTURE_2D, m_TextureId, 0);

		// check FBO status
		GLenum FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (FBOstatus != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "GLError: GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO" << std::endl;
			return false;
		}
		return true;
	}

	void RenderTexture::GenerateMipMaps()
	{
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_TextureId);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

}