#pragma once

#include "nowork/Common.h"

#include "NoWork/RenderTexture.h"
#include "NoWork/AsyncGLWorker.h"
#include <unordered_map>

class Framebuffer;
typedef std::shared_ptr<Framebuffer> FramebufferPtr;

class Framebuffer : public AsyncGLWorker
{
	friend class Renderer;
public:
	enum AttachmentType
	{
		COLOR0 = 0x8CE0,
		COLOR1,
		COLOR2,
		COLOR3,
		COLOR4,
		COLOR5,
		COLOR6,
		COLOR7,
		COLOR8,
		COLOR9,
		COLOR10,
		COLOR11,
		COLOR12,
		COLOR13,
		COLOR14,
		COLOR15,
		DEPTH_ATTACHMENT = 0x8D00,
		STENCIL_ATTACHMENT = 0x8D20,
		DEPTH_STENCIL_ATTACHMENT = 0x821A,
		BACK = 0x0405,
	};


	NOWORK_API ~Framebuffer();

	NOWORK_API static FramebufferPtr Create(int width, int height);

	NOWORK_API bool BindTexture(RenderTexturePtr renderTexture, AttachmentType targetAttachmentType);
	NOWORK_API void Bind();
	NOWORK_API void BindWrite();
	NOWORK_API void BindRead();
	NOWORK_API void Unbind();
	NOWORK_API unsigned int GetID() { return m_FBO; }

	NOWORK_API RenderTexturePtr GetAttachedTexture(AttachmentType at);

	NOWORK_API RenderTexturePtr CreateAndAttachTexture(AttachmentType targetAttachmentType, RenderTexture::Type type, Texture::Format textureFormat, bool compressed = false);
protected:
	Framebuffer();

	virtual void DoAsyncWork(int mode, void *params) override;

private:
	struct AsyncBindParameters
	{
		RenderTexturePtr renderTexture;
		AttachmentType targetAttachmentType;
	};

	unsigned int m_FBO;
	glm::ivec2 m_Size;
	std::unordered_map<AttachmentType, RenderTexturePtr> m_BoundTextures;
	std::vector<AttachmentType> m_BoundAttachmentTypes;
};

