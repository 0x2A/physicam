#pragma once

#include "nowork/Common.h"
#include "nowork/Renderer.h"

namespace Window
{
	typedef enum
	{
		WINDOW_FULLSCREEN = 0x00000001,							/**< fullscreen window */
		WINDOW_OPENGL_FORWARD_COMPAT = 0x00000002,              /**< enable OpenGL forward compatibility */
		WINDOW_RESIZABLE = 0x00000004,							/**< window is resizable */
		WINDOW_VISIBLE = 0x00000008,							/**< window is visible*/
		WINDOW_DECORATED = 0x00000010,							/**< window has border and icon etc */
		WINDOW_STEREO_RENDERING = 0x00000020,					/**< enable stereoscopic rendering */
		WINDOW_OPENGL_DEBUG_CONTEXT = 0x00000040,				/**< enable OpenGL debug context */
		WINDOW_SHOWED = WINDOW_VISIBLE | WINDOW_DECORATED,	/**< windowed mode */
	} Flags;
}

struct GLFWwindow;
class GameBase;
class NoWork
{
	friend class Renderer;

public:
	NOWORK_API NoWork();
	NOWORK_API ~NoWork();

	NOWORK_API bool CreateNewWindow(std::string title, int width, int height, int posX = 40, int posY = 40, int flags = Window::Flags::WINDOW_SHOWED, int multisampling = 0);
	NOWORK_API void EnableVSync();
	NOWORK_API void DisableVSync();
	/** 
	/* Sets the OpenGL profile to core only, so that only core OpenGL versions of the selected version are compatible (disables backwards compatibility and extensions)
	/* Must be called before CreateNewWindow!
	*/
	NOWORK_API void SetOpenGLCoreProfileOnly() { m_CoreProfileOnly = true; }
	
	/**
	/* Sets the OpenGL version to use
	/* Must be called before CreateNewWindow!
	*/
	NOWORK_API void SetOpenGLVersion(unsigned char major, unsigned char minor) { m_OpenGLMajorVersion = major, m_OpenGLMinorVersion = minor; }

	NOWORK_API void RegisterGame(GameBase* handle) { m_GameHandle = handle; }
	NOWORK_API void Run();

	NOWORK_API void Exit();

	NOWORK_API bool ExtensionAvailable(std::string name);

	NOWORK_API static bool IsMainThread();

	NOWORK_API GLFWwindow* GetWindow() { return m_Window; }
	NOWORK_API glm::ivec2 ScreenSize();

private:

	GLFWwindow* DetectMaxSupportedGlVersionAndCreateWindow(std::string title, int width, int height, bool fullscreen);

	void Update();
	

	void ContentLoaderFunc();

	GLFWwindow* m_Window = NULL;
	//GLFWwindow *m_LoaderThreadWindow = NULL;

	GameBase* m_GameHandle = NULL;
	long m_WindowFlags;
	bool m_CoreProfileOnly = false;
	unsigned char m_OpenGLMajorVersion = 1, m_OpenGLMinorVersion = 0;
	Renderer* m_Renderer;

	bool m_Loading = false;
	std::thread m_LoadingThread;
	double currentFrame, deltaTime, lastFrame;

	NOWORK_API static std::thread::id m_MainThreadId;
};