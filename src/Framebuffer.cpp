#include <Physicam/Framebuffer.h>

#include <gl/glew.h>

namespace PhysiCam
{
	Framebuffer::Framebuffer()
	{
		m_FBO = 0;
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_FBO);
	}

	FramebufferPtr Framebuffer::Create(int width, int height)
	{
		FramebufferPtr fb = FramebufferPtr(new Framebuffer);
		fb->m_Size.x = width;
		fb->m_Size.y = height;

		glGenFramebuffers(1, &fb->m_FBO);
		return fb;
	}

	bool Framebuffer::BindTexture(RenderTexturePtr renderTexture, AttachmentType targetAttachmentType)
	{
		if (!renderTexture)
			return false;

		if (renderTexture->GetSize() != m_Size)
			std::cerr << "Warning: Bound render texture does not match framebuffer size! TextureId:" << renderTexture->GetTextureId() << " FramebufferId:" << m_FBO;

		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		switch (renderTexture->GetType())
		{
		case GL_TEXTURE_1D:
			glFramebufferTexture1D(GL_FRAMEBUFFER, targetAttachmentType, renderTexture->GetType(), renderTexture->GetTextureId(), 0);
			break;
		case GL_TEXTURE_2D:
			glFramebufferTexture2D(GL_FRAMEBUFFER, targetAttachmentType, renderTexture->GetType(), renderTexture->GetTextureId(), 0);
			break;
		case GL_TEXTURE_3D:
			glFramebufferTexture3D(GL_FRAMEBUFFER, targetAttachmentType, renderTexture->GetType(), renderTexture->GetTextureId(), 0, 0);
			break;
		default:
			glFramebufferTexture(GL_FRAMEBUFFER, targetAttachmentType, renderTexture->GetTextureId(), 0);
		}

		// check FBO status
		GLenum FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (FBOstatus != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "GLError: GL_FRAMEBUFFER_COMPLETE failed, CANNOT use FBO\n";
			return false;
		}

		m_BoundTextures[targetAttachmentType] = renderTexture;
		m_BoundAttachmentTypes.clear();
		for (auto &rt : m_BoundTextures)
		{
			if (rt.second && !(rt.first == DEPTH_ATTACHMENT || rt.first == STENCIL_ATTACHMENT || rt.first == DEPTH_STENCIL_ATTACHMENT))
				m_BoundAttachmentTypes.push_back(rt.first);
		}

		// switch back to window-system-provided framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		return true;
	}

	void Framebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
		if (m_BoundAttachmentTypes.size() > 0)
			glDrawBuffers(m_BoundAttachmentTypes.size(), (GLenum*)&m_BoundAttachmentTypes[0]);
		glViewport(0, 0, m_Size.x, m_Size.y);
	}

	void Framebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	RenderTexturePtr Framebuffer::CreateAndAttachTexture(AttachmentType targetAttachmentType,
		RenderTexture::Type type, RenderTexture::Format textureFormat, bool generateMipMaps, bool compressed /*= false*/)
	{
		RenderTexturePtr tex = RenderTexture::Create(m_Size.x, m_Size.y, type, textureFormat, compressed, generateMipMaps);
		if (!BindTexture(tex, targetAttachmentType))
		{
			std::cerr << "Failed to attach texture to Framebuffer: " << targetAttachmentType;
		}
		return tex;
	}

	RenderTexturePtr Framebuffer::GetAttachedTexture(AttachmentType at)
	{
		return m_BoundTextures[at];
	}


	 void Framebuffer::BindWrite()
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FBO);
		if (m_BoundAttachmentTypes.size() > 0)
			glDrawBuffers(m_BoundAttachmentTypes.size(), (GLenum*)&m_BoundAttachmentTypes[0]);
		glViewport(0, 0, m_Size.x, m_Size.y);
	}

	 void Framebuffer::BindRead()
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
	}
}