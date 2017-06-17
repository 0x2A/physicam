/*
	PhysiCam - Physically based camera
	Copyright (C) 2015 Frank Köhnke

	This file is part of PhysiCam.

	This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License 
	as published by the Free Software Foundation; either 
	version 3 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 *	@file physiCamTest.h
 */

#pragma once

#include <NoWork/Game.h>
#include <NoWork/Framebuffer.h>
#include <NoWork/RenderTexture.h>
#include <NoWork/Font.h>
#include <physicam.h>

#include <AntTweakBar/AntTweakBar.h>

class TestGame : public GameBase
{
public:

	virtual void Init() override;

	virtual void OnLoadContent() override;

	virtual void OnUpdate(double deltaTime) override;

	virtual void OnRender() override;

	virtual void OnShutdown() override;

	//Bound to escape key, used to exit the game when escape pressed
	void Exit();

	virtual void OnLoadRender() override;

	private:

		void SetupUI();

		void HandleInput(double deltaTime);
		void UpdateFreeLookCamera(double deltaTime);


		PhysiCam::CameraPtr m_Camera;
		FramebufferPtr m_FrameBuffer;
		RenderTexturePtr m_ColorTexture;
		RenderTexturePtr m_DepthTexture;

		FontPtr m_Font;
		TwBar *bar;
};