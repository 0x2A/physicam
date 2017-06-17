#pragma once

#include "nowork/Framework.h"
#include "nowork/Renderer.h"

//Base game interface
class GameBase
{
	friend class NoWork;
public:
	virtual void Init() = 0; //called to load content needed while loading state (like bg textures for loading screen etc)
	virtual void OnLoadContent() = 0; //Actual loading function. This is running in a seperate thread!
	virtual void OnUpdate(double deltaTime) = 0; //called if game should update
	virtual void OnLoadRender() = 0; //called while loading process to render loading screen etc.
	virtual void OnRender() = 0; //called while rendering process
	virtual void OnShutdown() = 0; //called when framework is shutting down

protected:
	NoWork* m_Framework;
	Renderer* m_Renderer;
};