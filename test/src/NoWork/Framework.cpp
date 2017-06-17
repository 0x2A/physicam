#include "nowork/Framework.h"

#include "nowork/Input.h"
#include "nowork/EventHandler.h"
#include "nowork/Game.h"
#include "nowork/Log.h"
#include "nowork/Shader.h"
#include "nowork/Mesh.h"
#include "nowork/Texture.h"
#include "NoWork/Font.h"

#include <AntTweakBar/AntTweakBar.h>
#include <time.h>



typedef struct
{
	unsigned char major;
	unsigned char minor;
} glversion_t;

const glversion_t glVersions[] = { { 1, 1 }, { 1, 2 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 2, 0 }, { 2, 1 }, { 3, 0 }, 
									{ 3, 1 }, { 3, 2 }, { 3, 3 }, 
									{ 4, 0 }, { 4, 1 }, { 4, 2 }, { 4, 3 }, { 4, 4 }, { 4, 5 } };


std::thread::id NoWork::m_MainThreadId;


NoWork::NoWork()
{
	static Log log; //trick to call Logs constructor

	LOG_MESSAGE("NoWork framework version: " << NOWORK_VERSION);

	m_Renderer = 0;

	if (!glfwInit())
	{
		LOG_ERROR("Failed to initialize glfw");
	}

	glfwSetErrorCallback(&EventHandler::ErrorCallback);

	m_GameHandle = NULL;
	lastFrame = glfwGetTime();

	m_MainThreadId = std::this_thread::get_id();


	srand((uint32_t)time(NULL));
	rand(); //invoke rand one time cause some implementations ignore srand at first call
}

NoWork::~NoWork()
{
	DelPtr(m_Renderer);
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

NOWORK_API bool NoWork::CreateNewWindow(std::string title, int width, int height, int posX /*= 40*/, int posY /*= 40*/, int flags /*= Window::Flags::WINDOW_SHOWED*/, int multisampling /*= 0*/)
{
	//delete old renderer if there was created one before
	if (m_Renderer)
		DelPtr(m_Renderer);

	m_WindowFlags = flags;

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, flags & Window::Flags::WINDOW_OPENGL_FORWARD_COMPAT);
	glfwWindowHint(GLFW_RESIZABLE, flags & Window::Flags::WINDOW_RESIZABLE);
	glfwWindowHint(GLFW_VISIBLE, flags & Window::Flags::WINDOW_VISIBLE);
	glfwWindowHint(GLFW_DECORATED, flags & Window::Flags::WINDOW_DECORATED);
	glfwWindowHint(GLFW_STEREO, flags & Window::Flags::WINDOW_STEREO_RENDERING);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, flags & Window::Flags::WINDOW_OPENGL_DEBUG_CONTEXT);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, m_OpenGLMajorVersion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, m_OpenGLMinorVersion);
	glfwWindowHint(GLFW_SAMPLES, multisampling);

	if (m_CoreProfileOnly)
	{
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	m_Window = glfwCreateWindow(width, height, title.c_str(), (flags & Window::Flags::WINDOW_FULLSCREEN) ? glfwGetPrimaryMonitor() : NULL, NULL);
	if (!m_Window)
	{
		LOG_ERROR("OpenGL version " << (int)m_OpenGLMajorVersion << "." << (int)m_OpenGLMinorVersion << " not supported!");
		m_Window = DetectMaxSupportedGlVersionAndCreateWindow(title, width, height, flags & Window::Flags::WINDOW_FULLSCREEN);
		if (!m_Window)
		{
			LOG_ERROR("Fatal error, no supported OpenGL version found!");
			return false;
		}
		LOG_WARNING("Using latest supported OpenGL version: " << (int)m_OpenGLMajorVersion << "." << (int)m_OpenGLMinorVersion << "!");
	}
	if (!(flags & Window::Flags::WINDOW_FULLSCREEN)) 
		glfwSetWindowPos(m_Window, posX, posY);


	glfwMakeContextCurrent(m_Window);

	//glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	//m_LoaderThreadWindow = glfwCreateWindow(1, 1, "Thread Window", 0, m_Window);


	if (gl3wInit()) {
		LOG_ERROR("failed to initialize gl3w");
		return false;
	}

	if (!PhysiCam::Camera::Init())
	{
		LOG_ERROR("failed to initialize PhysiCam");
		return false;
	}

	//Print window informations of actual settings
	int w, h;
	glfwGetWindowSize(m_Window, &w, &h);
	GLFWmonitor* monitor = glfwGetWindowMonitor(m_Window);
	
	LOG_MESSAGE("Created Window:\n" << "Dimension: " << w << "x" << h << "\n" <<
		"Mode: " << ((monitor == 0) ? "windowed" : "fulscreen") << "\n" <<
		"OpenGL version: " << glGetString(GL_VERSION) << ", GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n" <<
		"Hardware vendor: " << glGetString(GL_VENDOR) << "\n" <<
		"Hardware name: " << glGetString(GL_RENDERER));


	Input::Init(this);
	glfwSetKeyCallback(m_Window, EventHandler::KeyEventCallback);
	
	TwInit(TwGraphAPI::TW_OPENGL_CORE, NULL);
	// - Directly redirect GLFW char events to AntTweakBar
	glfwSetCharCallback(m_Window, (GLFWcharfun)TwEventCharGLFW);
	// - Directly redirect GLFW mouse button events to AntTweakBar
	glfwSetMouseButtonCallback(m_Window, (GLFWmousebuttonfun)TwEventMouseButtonGLFW);
	glfwSetCursorPosCallback(m_Window, (GLFWcursorposfun)TwEventMousePosGLFW);

	if (ExtensionAvailable("GL_ARB_debug_output"))
	{
		glDebugMessageCallbackARB(EventHandler::GLErrorCallback, NULL);
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, false);
	}

	glfwSwapInterval(0);

	m_Renderer = new Renderer(this, m_Window);

	//initialize static vars
	Mesh::Init(m_Renderer);
	Texture::Init(this, m_Renderer);
	
	//font depends on textures so init it last
	Font::Init(this);

	return true;
}

bool NoWork::ExtensionAvailable(std::string name)
{
	if (!glfwExtensionSupported(name.c_str()))
	{
		LOG_WARNING(name << " not supported!");
		return false;
	}
	return true;
}

void NoWork::Run()
{

	if (!m_GameHandle)
	{
		std::cout << "ERROR: No game registered! Please call RegisterGame and pass a valid game object pointer." << std::endl;
		Exit();
	}	

	//Initialize the game
	m_GameHandle->m_Framework = this;
	m_GameHandle->m_Renderer = m_Renderer;

	Shader::InitializeDefaultShaders();
	m_GameHandle->Init();

	m_Loading = true;
	m_LoadingThread = std::thread(&NoWork::ContentLoaderFunc, this);

	while (!glfwWindowShouldClose(m_Window))
	{
		Renderer::DrawCalls = 0;
		Input::Update();

		m_Renderer->DoAsyncGLQueue();
		if (m_Loading)
		{
			m_Renderer->Render();
			m_GameHandle->OnLoadRender();
		}
		else
		{
			Update();

			m_Renderer->Render();
			m_GameHandle->OnRender();
		}

		glfwSwapBuffers(m_Window);
		glfwPollEvents();

	}

	m_GameHandle->OnShutdown();
	m_LoadingThread.join();

	Font::Shutdown();
	TwTerminate();
}


void NoWork::Update()
{
	currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	m_GameHandle->OnUpdate(deltaTime);

	m_Renderer->Update();
}

void NoWork::Exit()

{
	glfwSetWindowShouldClose(m_Window, true);
}

void NoWork::ContentLoaderFunc()
{
	//We need to create a new OpenGL context (done in glfw by creating a new window, duh)
	//This is needed because in OpenGL only one thread can have access to an context
	//So OpenGL functions in this thread would fail if we wouldnt create a new context
	
	//glfwMakeContextCurrent(m_LoaderThreadWindow);

	
	m_GameHandle->OnLoadContent();
	
	m_Loading = false;
}

GLFWwindow* NoWork::DetectMaxSupportedGlVersionAndCreateWindow(std::string title, int width, int height, bool fullscreen)
{
	int index = 0;
	int elCount = sizeof(glVersions) / sizeof(glversion_t);
	for (int i = 0; i < elCount; i++)
	{
		if (m_OpenGLMajorVersion == glVersions[i].major && m_OpenGLMinorVersion == glVersions[i].minor)
		{
			index = i;
			break;
		}
	}

	//disable error callback while we try to get the latest glVersion working
	//so we dont spam the error log
	glfwSetErrorCallback(NULL);

	while (index >= 0)
	{
		--index;
		m_OpenGLMajorVersion = glVersions[index].major;
		m_OpenGLMinorVersion = glVersions[index].minor;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, m_OpenGLMajorVersion);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, m_OpenGLMinorVersion);

		GLFWwindow *window = glfwCreateWindow(width, height, title.c_str(), fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
		if (window)
		{
			glfwSetErrorCallback(&EventHandler::ErrorCallback);
			return window;
		}
	}

	//reenable error callback
	glfwSetErrorCallback(&EventHandler::ErrorCallback);

	return NULL;
}

void NoWork::EnableVSync()
{
	glfwSwapInterval(1);
}

void NoWork::DisableVSync()
{
	glfwSwapInterval(0);
}

bool NoWork::IsMainThread()
{
	return std::this_thread::get_id() == m_MainThreadId;
}

NOWORK_API glm::ivec2 NoWork::ScreenSize()
{
	int w, h;
	m_Renderer->GetFramebufferSize(w, h);
	return glm::ivec2(w, h);
}
