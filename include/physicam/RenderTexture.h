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
 *	@file RenderTexture.h
 */

#pragma once

#include <physicam/physicam_math.h>
#include <memory>

namespace PhysiCam
{
	class RenderTexture;
	typedef std::shared_ptr<RenderTexture> RenderTexturePtr;

	class RenderTexture
	{
	public:
		enum Format : unsigned int
		{
			R3_G3_B2 = 0x2A10,
			RGB4 = 0x804F,
			RGB5 = 0x8050,
			RGB8 = 0x8051,
			RGB10 = 0x8052,
			RGB12 = 0x8053,
			RGB16 = 0x8054,
			RGBA2 = 0x8055,
			RGBA4 = 0x8056,
			RGB5_A1 = 0x8057,
			RGBA8 = 0x8058,
			RGB10_A2 = 0x8059,
			RGBA12 = 0x805A,
			RGBA16 = 0x805B,
			RG = 0x8227,
			RG_INTEGER = 0x8228,
			R8 = 0x8229,
			R16 = 0x822A,
			RG8 = 0x822B,
			RG16 = 0x822C,
			R16F = 0x822D,
			R32F = 0x822E,
			RG16F = 0x822F,
			RG32F = 0x8230,
			R8I = 0x8231,
			R8UI = 0x8232,
			R16I = 0x8233,
			R16UI = 0x8234,
			R32I = 0x8235,
			R32UI = 0x8236,
			RG8I = 0x8237,
			RG8UI = 0x8238,
			RG16I = 0x8239,
			RG16UI = 0x823A,
			RG32I = 0x823B,
			RG32UI = 0x823C,
			RGBA32UI = 0x8D70,
			RGB32UI = 0x8D71,
			RGBA16UI = 0x8D76,
			RGB16UI = 0x8D77,
			RGBA8UI = 0x8D7C,
			RGB8UI = 0x8D7D,
			RGBA32I = 0x8D82,
			RGB32I = 0x8D83,
			RGBA16I = 0x8D88,
			RGB16I = 0x8D89,
			RGBA8I = 0x8D8E,
			RGB8I = 0x8D8F,
			RGB16F = 0x881B,
			RGB32F = 0x8815,
			RGBA16F = 0x881A,
			RGBA32F = 0x8814,
			R8_SNORM = 0x8F94,
			RG8_SNORM = 0x8F95,
			RGB8_SNORM = 0x8F96,
			RGBA8_SNORM = 0x8F97,
			R16_SNORM = 0x8F98,
			RG16_SNORM = 0x8F99,
			RGB16_SNORM = 0x8F9A,
			RGBA16_SNORM = 0x8F9B,
			RGB10_A2UI = 0x906F,
			DEPTH = 0x1902,
			DEPTH_STENCIL = 0x84F9,
			COMPRESSED_RED = 0x8225,
			COMPRESSED_RG = 0x8226,
			COMPRESSED_RGB = 0x84ED,
			COMPRESSED_RGBA = 0x84EE
		};

		enum Type : unsigned int
		{
			TEXTURE_1D = 0x0DE0,
			TEXTURE_2D = 0x0DE1,
			TEXTURE_3D = 0x0DE2,
		};

		static RenderTexturePtr Create(int width, int height, RenderTexture::Type type, RenderTexture::Format textureFormat, bool compressed = false, bool genMipMaps = false);
		~RenderTexture();

		void SetParameteri(unsigned int pName, int param);
		void SetParameteriv(unsigned int pName, const int *param);
		void SetParameterf(unsigned int pName, float param);
		void SetParameterfv(unsigned int pName, const float *param);

		void Bind(uint32_t slot = 0);

		static void GetInternalFormat(unsigned int textureType, unsigned int targetFormat, int* internalFormat, int *type);

		void SetLinearTextureFilter(bool state, float anisotropy = 0.0f);

		unsigned int GetTextureId() { return m_TextureId; }
		RenderTexture::Type GetType() { return (RenderTexture::Type)m_Type; }

		glm::ivec2 GetSize() { return m_Size; }

		bool AttachToFramebuffer(unsigned int FramebufferId, unsigned int attachementPoint);

		void GenerateMipMaps();

	protected:

		RenderTexture();

		void GenerateTexture(RenderTexture::Format textureFormat, bool genMipMaps);
		static int GetInternalFormatLegacy(unsigned int targetFormat);
		
		unsigned int m_TextureId;
		glm::ivec2 m_Size;
		RenderTexture::Format m_Format;
		unsigned int m_InternalFormat;
		int m_Type;
		
	};
}