#pragma once

#include "nowork/Common.h"
#include "physicam/camera.h"
#include "NoWork/Mesh.h"
#include "NoWork/AsyncGLWorker.h"
#include "NoWork/RenderTexture.h"
#include "NoWork/Framebuffer.h"

#include <queue>
#include <mutex>

struct GLFWwindow;
class Texture;
class NoWork;
class Renderer
{
	friend class AsyncGLWorker;
public:

	enum ClearBufferBits
	{
		DEPTH_BUFFER = 0x00000,
		STENCIL_BUFFER = 0x00000400,
		COLOR_BUFFER = 0x00004000
	};

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
		DEPTH_STENCIL_ATTACHMENT = 0x821A
	};

	Renderer(NoWork* framework, GLFWwindow* window);
	~Renderer();

	NOWORK_API PhysiCam::CameraPtr GetCamera() const { return m_Camera; }
	NOWORK_API void GetFramebufferSize(int& width, int& height) { width = m_FramebufferWidth; height = m_FramebufferHeight; }
	NOWORK_API float GetAspectRatio() const { return m_AspectRatio; }
	NOWORK_API GLFWwindow * GetWindow() const { return m_Window; }

	void Update();
	void Render();

	NOWORK_API void SetBackfaceCulling(bool state);

	NOWORK_API void SetAlphaBlending(bool state);
	NOWORK_API void SetWireframeMode(bool state);
	
	NOWORK_API void SetViewPort(int x, int y, int width, int height);

	NOWORK_API void ClearScreen(int bitfield = ClearBufferBits::STENCIL_BUFFER | ClearBufferBits::COLOR_BUFFER);

	NOWORK_API void SetAnisotropicFiltering(float val);
	NOWORK_API float GetAnisotropicFilterValue();
	NOWORK_API bool AnisotropicFilteringAvailable() { return m_AnisotropicFilteringAvail; }
	
	NOWORK_API void DoAsyncGLQueue();

	NOWORK_API int* GetViewport() { return m_Viewport; }

	NOWORK_API void SetDepthTest(bool t);

	NOWORK_API void RenderFullscreenQuad(ShaderPtr shader, glm::vec2 resolution = glm::vec2(-1,-1));
	NOWORK_API void Blit(RenderTexturePtr source, FramebufferPtr framebuffer, ShaderPtr shader);
	NOWORK_API void Blit(RenderTexturePtr source, FramebufferPtr framebuffer);
	NOWORK_API void Blit(RenderTexturePtr source);
	

	NOWORK_API FramebufferPtr GetWindowFramebuffer() { return m_WindowFramebuffer; }

	//NOWORK_API void SetCamera(Camera* val) { m_Camera = val; }

	NOWORK_API static int DrawCalls;

protected:

	void RegisterAsyncGLWork(AsyncGLWork_t worker);
	
private:


	std::queue<AsyncGLWork_t> m_AsyncGlQueue;
	std::mutex m_GLQueueMutex;

	GLFWwindow *m_Window;
	
	int m_FramebufferWidth, m_FramebufferHeight;
	
	float m_AspectRatio;

	float m_MaxAnisotropicFiltering;
	float m_AnisotropicFilteringVal;
	bool m_AnisotropicFilteringAvail;
	
	PhysiCam::CameraPtr m_Camera = NULL;

	NoWork *m_Framework;

	int m_Viewport[4];

	MeshPtr m_FullscreenQuad;
	FramebufferPtr m_WindowFramebuffer;

};