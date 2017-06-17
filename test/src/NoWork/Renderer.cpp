#include "nowork/Renderer.h"
#include "nowork/Texture.h"
#include "nowork/Texture2D.h"
#include "nowork/Log.h"
#include "NoWork/Framework.h"

//gl3w is missing this, idk why...
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF


int Renderer::DrawCalls = 0;

Renderer::Renderer(NoWork* framework, GLFWwindow* window) : m_Window(window)
{
	m_Framework = framework;
	glfwGetFramebufferSize(window, &m_FramebufferWidth, &m_FramebufferHeight);
	m_AspectRatio = m_FramebufferWidth / (float)m_FramebufferHeight;

	m_Viewport[0] = 0;
	m_Viewport[1] = 0;
	m_Viewport[2] = m_FramebufferWidth;
	m_Viewport[3] = m_FramebufferHeight;
	glViewport(0, 0, m_FramebufferWidth, m_FramebufferHeight);
	SetBackfaceCulling(true);

	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	m_Camera = std::make_shared<PhysiCam::Camera>(m_FramebufferWidth, m_FramebufferHeight);

	m_AnisotropicFilteringAvail = m_Framework->ExtensionAvailable("GL_EXT_texture_filter_anisotropic");
	m_AnisotropicFilteringVal = 0.0f;

	if (m_AnisotropicFilteringAvail)
	{
		glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_MaxAnisotropicFiltering);
	}

	AsyncGLWorker::renderer = this;

	m_FullscreenQuad = Mesh::CreatePlane(Mesh::STATIC_DRAW);
	m_FullscreenQuad->m_Renderer = this; //dirty workaround, but they need each other :O

	m_WindowFramebuffer = FramebufferPtr(new Framebuffer);
	m_WindowFramebuffer->m_FBO = 0;
	m_WindowFramebuffer->m_BoundAttachmentTypes.push_back(Framebuffer::BACK);
	m_WindowFramebuffer->m_Size = glm::ivec2(m_FramebufferWidth, m_FramebufferHeight);
}

Renderer::~Renderer()
{
}

void Renderer::Update()
{
	//if (m_Camera)
		//m_Camera->Update();
}
void Renderer::Render()
{
	//m_Camera->Render();
}

void Renderer::ClearScreen(int bitfield /*= ClearBufferBits::STENCIL_BUFFER | ClearBufferBits::COLOR_BUFFER*/)
{
	glClear(bitfield);
}

void Renderer::SetBackfaceCulling(bool state)
{
	state ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
}

void Renderer::SetAlphaBlending(bool state)
{
	if (state)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

void Renderer::SetWireframeMode(bool state)
{
	if (state)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		SetBackfaceCulling(false);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		SetBackfaceCulling(true);
	}
}

void Renderer::SetViewPort(int x, int y, int width, int height)
{
	m_Viewport[0] = x;
	m_Viewport[1] = y;
	m_Viewport[2] = width;
	m_Viewport[3] = height;
	glViewport(x, y, width, height);
}


void Renderer::SetAnisotropicFiltering(float val)
{
	if (val <= 0.0f || !m_AnisotropicFilteringAvail) 
		m_AnisotropicFilteringVal = 0.0f;
	else
	{
		m_AnisotropicFilteringVal = val;
		if (m_AnisotropicFilteringVal > m_MaxAnisotropicFiltering)
			m_AnisotropicFilteringVal = m_MaxAnisotropicFiltering;
	}
}

float Renderer::GetAnisotropicFilterValue()
{
	if (!m_AnisotropicFilteringAvail)
		return 0.0f;
	return m_AnisotropicFilteringVal;
}


void Renderer::DoAsyncGLQueue()
{
	while (!m_AsyncGlQueue.empty())
	{
		m_GLQueueMutex.lock();
		auto work = m_AsyncGlQueue.front();
		work.worker->DoAsyncWork(work.mode, work.params);
		m_AsyncGlQueue.pop();
		m_GLQueueMutex.unlock();
	}
}

void Renderer::RegisterAsyncGLWork(AsyncGLWork_t work)
{
	m_GLQueueMutex.lock();
	m_AsyncGlQueue.push(work);
	m_GLQueueMutex.unlock();
}

NOWORK_API void Renderer::SetDepthTest(bool t)
{
	if (t)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
}

NOWORK_API void Renderer::RenderFullscreenQuad(ShaderPtr shader, glm::vec2 resolution /*= glm::vec2(-1,-1)*/)
{
	if (resolution.x > 0 && resolution.y > 0)
	{
		glViewport(0, 0, resolution.x, resolution.y);
	}
	m_FullscreenQuad->Render(shader);
}

NOWORK_API void Renderer::Blit(RenderTexturePtr source, FramebufferPtr framebuffer /*= nullptr*/)
{
	Blit(source, framebuffer, Shader::BlitScreenShader);
}

NOWORK_API void Renderer::Blit(RenderTexturePtr source)
{
	Blit(source, m_WindowFramebuffer);
}

NOWORK_API void Renderer::Blit(RenderTexturePtr source, FramebufferPtr framebuffer, ShaderPtr shader)
{
	framebuffer->Bind();
	shader->Use();
	shader->SetParameterTexture("texture0", source, 0);
	//ClearScreen(GL_COLOR_BUFFER_BIT);
	RenderFullscreenQuad(shader);
}

