#include "nowork/Input.h"

#include "nowork/Framework.h"


//std::unordered_map<long, void*> Input::m_KeyHandlersArgs;
std::unordered_map<long, Input::CallbackFunc> Input::m_KeyHandlers;
std::unordered_map<long, bool> Input::m_KeyStates;
std::unordered_map<long, bool> Input::m_MouseKeyStates;
NoWork* Input::m_Framework;
glm::vec2 Input::m_MouseLastPos;
glm::vec2 Input::m_MouseDelta;
glm::vec2 Input::m_MousePos;
bool Input::m_CenterMouse = false;

void Input::Init(NoWork* framework)
{
	m_Framework = framework;
	m_KeyHandlers.clear();
}

bool Input::KeyDown(long key)
{
	
	bool keyDown = (glfwGetKey(m_Framework->GetWindow(), key) != GLFW_RELEASE);
	if (keyDown)
	{
		if (!m_KeyStates[key]) //Key never pressed before, set it to true
		{
			m_KeyStates[key] = true;
			return true;
		}
		//key already pressed
		return false;
	}

	m_KeyStates[key] = false; //key not pressed, reset to false
	return false;
}


bool Input::KeyHeld(long key)
{
	return (glfwGetKey(m_Framework->GetWindow(), key) == GLFW_PRESS);
}

void Input::Update()
{
	double xPos, yPos;
	glfwGetCursorPos(m_Framework->GetWindow(), &xPos, &yPos);
	glm::vec2 currPos(xPos, yPos);
	if (!m_CenterMouse)
	{
		m_MouseDelta = currPos - m_MouseLastPos;
		m_MouseLastPos = m_MousePos = currPos;
	}
	else
	{
		glm::ivec2 screenSize = m_Framework->ScreenSize();
		m_MouseDelta = currPos - glm::vec2(screenSize/2);
		m_MousePos += m_MouseDelta;
		m_MouseLastPos = m_MousePos;

		glfwSetCursorPos(m_Framework->GetWindow(), screenSize.x / 2, screenSize.y / 2);
	}
}

void Input::HandleKey(long key)
{
	
	auto gotKey = m_KeyHandlers.find(key);
	if (gotKey != m_KeyHandlers.end())
	{
		m_KeyHandlers[key]();
	}
}

glm::vec2 Input::MouseDelta()
{
	return m_MouseDelta;
}

glm::vec2 Input::MousePos()
{
	return m_MousePos;
}

bool Input::MouseButtonDown(long btn)
{
	bool keyDown = (glfwGetMouseButton(m_Framework->GetWindow(), btn) != GLFW_RELEASE);
	if (keyDown)
	{
		if (!m_MouseKeyStates[btn]) //Key never pressed before, set it to true
		{
			m_MouseKeyStates[btn] = true;
			return true;
		}
		//key already pressed
		return false;
	}

	m_MouseKeyStates[btn] = false; //key not pressed, reset to false
	return false;
}

bool Input::MouseButtonHeld(long btn)
{
	return (glfwGetMouseButton(m_Framework->GetWindow(), btn) == GLFW_PRESS);
}

NOWORK_API void Input::GrabMouse(bool s)
{
	if (s)
		glfwSetInputMode(m_Framework->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(m_Framework->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
