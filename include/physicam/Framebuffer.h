#pragma once


#include "RenderTexture.h"
#include "arch.h"
#include <unordered_map>
#include <memory>


namespace PhysiCam
{

	class Framebuffer;
	typedef std::shared_ptr<Framebuffer> FramebufferPtr;

	class PHYSICAM_DLL Framebuffer
	{
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


		~Framebuffer();

		static FramebufferPtr Create(int width, int height);

		bool BindTexture(RenderTexturePtr renderTexture, AttachmentType targetAttachmentType);
		void Bind();
		void BindWrite();
		void BindRead();
		void Unbind();
		unsigned int GetID() { return m_FBO; }

		RenderTexturePtr GetAttachedTexture(AttachmentType at);

		RenderTexturePtr CreateAndAttachTexture(AttachmentType targetAttachmentType, RenderTexture::Type type, RenderTexture::Format textureFormat, bool generateMipMaps = false, bool compressed = false);
	protected:
		Framebuffer();

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

}