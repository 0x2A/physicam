#pragma once

#include "NoWork/Texture.h"


class TextureCube;
typedef std::shared_ptr<TextureCube> TextureCubePtr;


class TextureCube : public Texture
{
public:
	enum AsyncMode_t : int
	{
		AM_CopyPixData = Texture::AM_OFFSET
	};

	typedef struct _CubeMapPixelData
	{
		unsigned char* posX;
		unsigned char* negX;
		unsigned char* posY;
		unsigned char* negY;
		unsigned char* posZ;
		unsigned char* negZ;
		_CubeMapPixelData() : posX(nullptr), negX(nullptr),
			posY(nullptr), negY(nullptr), 
			posZ(nullptr), negZ(nullptr)
		{}
	} CubeMapPixelData;

	NOWORK_API static TextureCubePtr Create(unsigned int width, unsigned int height, Texture::Format format,
		CubeMapPixelData pixels, bool constant = true);

	NOWORK_API static TextureCubePtr Load(const std::string& pathPosX, const std::string& pathNegX,
		const std::string& pathPosY, const std::string& pathNegY, 
		const std::string& pathPosZ, const std::string& pathNegZ,
		bool constant = true);

	NOWORK_API void Update(const CubeMapPixelData& pixels);
	NOWORK_API void Update(const CubeMapPixelData& pixels, int width, int height);

protected:
	TextureCube();
	virtual void DoAsyncWork(int mode, void *params) override;

private:
	void CopyPixelData();


	CubeMapPixelData m_Pixels;
	int m_InternalFormat;
	int m_Type;
	Texture::Format m_Format;
	bool m_Constant;
};