#pragma once

#include "nowork/Texture.h"

class Texture2D;
typedef std::shared_ptr<Texture2D> Texture2DPtr;

class Texture2D : public Texture
{
	friend class Renderer;
public:

	enum AsyncMode_t : int
	{
		AM_CopyPixData = Texture::AM_OFFSET
	};

	NOWORK_API static Texture2DPtr Create(unsigned int width, unsigned int height, Texture::Format format, unsigned char *pixels, bool createMipMaps = true, bool constant = true);
	NOWORK_API static Texture2DPtr Load(const std::string& path, bool createMipMaps = true, bool constant = true);

	NOWORK_API void Update(const unsigned char* pixels);
	NOWORK_API void Update(const unsigned char* pixels, int width, int height);
protected:

	Texture2D();

	virtual void DoAsyncWork(int mode, void *params) override;

private:

	void CopyPixelData();

	GLubyte *m_Pixels;

	int m_InternalFormat;
	int m_Type;
	Texture::Format m_Format;
	bool m_Constant;
};