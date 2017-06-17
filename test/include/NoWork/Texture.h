#pragma once

#include "nowork/Common.h"
#include "NoWork/AsyncGLWorker.h"

class NoWork;
class Renderer;

class Texture;
typedef std::shared_ptr<Texture> TexturePtr;

class Texture : public AsyncGLWorker
{
	friend class NoWork;
	friend class Renderer;
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

	enum AsyncMode_t : int
	{
		AM_GenerateTexture = 0,
		AM_SetLinearTextureFilter,
		AM_SetTextureComparison,
		AM_OFFSET
	};

	NOWORK_API ~Texture();

	NOWORK_API void SetParameteri(unsigned int pName, int param);
	NOWORK_API void SetParameteriv(unsigned int pName, const int *param);
	NOWORK_API void SetParameterf(unsigned int pName, float param);
	NOWORK_API void SetParameterfv(unsigned int pName, const float *param);

	NOWORK_API void Bind(uint32_t slot = 0);

	NOWORK_API static void GetInternalFormat(unsigned int textureType, unsigned int targetFormat, int* internalFormat, int *type);

	NOWORK_API void SetLinearTextureFilter(bool state);
	NOWORK_API void SetTextureComparison(unsigned int compareMode, unsigned int compareFunc);

	NOWORK_API unsigned int GetTextureId(){ return m_TextureId; }
	NOWORK_API unsigned int GetType() { return m_TextureType; }

	NOWORK_API glm::ivec2 GetSize() { return glm::ivec2(m_Width, m_Height); }

protected:

	Texture(unsigned int texType = GL_TEXTURE_2D, unsigned int texBindingType = GL_TEXTURE_BINDING_2D); //values for GL_TEXTURE_2D and GL_TEXTURE_BINDING_2D, maybe we should include opengl headers

	static void Init(NoWork* framework, Renderer* renderer);

	void GenerateTexture();

	virtual void DoAsyncWork(int mode, void *params) override;

	NOWORK_API static bool m_UseDSA, m_UseTexStorage, m_UseInternalFormat;
	NOWORK_API static Renderer* m_Renderer;

	unsigned int m_TextureType, m_TextureBindingType;
	unsigned int m_TextureId;
	unsigned int m_Width, m_Height;
	bool m_CreateMipMaps;
};