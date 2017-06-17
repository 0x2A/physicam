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
 *	@file camera.cpp
 */

#include <physicam/Camera.h>
#include <physicam/physicam_gl.h>

#include <GL/glew.h>

namespace PhysiCam
{

	//constructor, default camera parameters to some useful defaults
	Camera::Camera(int screenWidth, int screenHeight)
		: m_Iso(100), m_Aperture(7.5f), m_ShutterSpeed(0.0025f), m_AutoExposure(true),
		m_FocalLength(36), m_MaxAperture(22.0f), m_MinAperture(1.8f), m_MinIso(100.0f), m_MaxIso(6400.0f),
		m_MaxShutterSpeed(0.00025f), m_MinShutterSpeed(0.0333f), m_SensorType({24.f, 0.03f}), m_ClipNear(0.5f), m_ClipFar(1000.0f),
		m_AspectRatio(screenWidth / (float)screenHeight), m_TargetEV(0), m_AverageSceneLuminance(0.0f)
	{
		m_ScreenSize = glm::ivec2(screenWidth, screenHeight);
		m_PostProcessor = new PostProcessor(this);

		m_DeltaTime = 0.0f;

		//values used from wikipedia
		m_SensorPresets[SENSOR_4_3] = {17.3f, 0.015f};
		m_SensorPresets[SENSOR_APS_C] = { 22.5f, 0.015f };
		m_SensorPresets[SENSOR_SMALL_FORMAT] = { 24.f, 0.03f };
		m_SensorPresets[SENSOR_MEDIUM_FORMAT] = { 36.f, 0.05f };
		m_SensorPresets[SENSOR_LARGE_FORMAT] = { 90.f, 0.10f };
	}

	Camera::~Camera()
	{
		DelPtr(m_PostProcessor);
	}


	float Camera::GetSaturationBasedExposure()
{
		// Compute the maximum luminance possible with H_sbs sensitivity
		// maxLum = 78 / ( S * q ) * N^2 / t
		// = 78 / ( S * q ) * 2^ EV_100
		// = 78 / (100 * 0.65) * 2^ EV_100
		// = 1.2 * 2^ EV
		// Reference: http://en.wikipedia.org/wiki/Film_speed
		//float maxLuminance = 1.2f * pow(2.0f, EV100);
		//return 1.0f / maxLuminance;
		
		float l_max = (7800.0f / 65.f) * (m_Aperture*m_Aperture) / (m_Iso * m_ShutterSpeed);
		return 1.0f / l_max;
	}

	float Camera::GetStandardOutputBasedExposure(float middleGrey /*= 0.18f*/)
	{
		float avg = (1000.0f / 65.0f) * pow(m_Aperture,2) / (m_Iso * m_ShutterSpeed);
		return middleGrey / avg;
	}

	void Camera::ApplyProgramAuto(float targetEV)
{
		// Start with the assumption that we want an aperture of 4.0
		m_Aperture = 4.0f;

		// Start with the assumption that we want a shutter speed of 1/f
		m_ShutterSpeed = 1.0f / (m_FocalLength);

		// Compute the resulting ISO if we left both shutter and aperture here
		m_Iso = glm::clamp(ComputeISO(m_Aperture, m_ShutterSpeed, targetEV), m_MinIso, m_MaxIso);

		// Apply half the difference in EV to the aperture
		float evDiff = targetEV - ComputeCurrentEV();
		m_Aperture = glm::clamp(m_Aperture * powf(sqrt(2.0f), evDiff * 0.5f), m_MinAperture, m_MaxAperture);

		// Apply the remaining difference to the shutter speed
		evDiff = targetEV - ComputeCurrentEV();
		m_ShutterSpeed = glm::clamp(m_ShutterSpeed * powf(2.0f, -evDiff), m_MaxShutterSpeed, m_MinShutterSpeed);
	}


	float Camera::ComputeISO(float aperture, float shutterSpeed, float ev)
	{
		return (aperture*aperture * 100.0f) / (shutterSpeed * powf(2.0f, ev));
	}

	float Camera::ComputeCurrentEV()
	{
		return log2(((m_Aperture*m_Aperture) * 100.0f) / (m_ShutterSpeed * m_Iso));
	}
	
	float Camera::ComputeTargetEV(float averageLuminance)
	{
		// K is a light meter calibration constant
		static const float K = 12.5;
		return log2(averageLuminance * 100.0f / K);
	}



	void Camera::Update(double deltaTime)
	{
		if (m_AutoExposure)
		{
			//TODO: compute averageLuminance
			//m_TargetEV = ComputeTargetEV(20.0f);
			//ApplyProgramAuto();
		}
		m_DeltaTime = deltaTime;
		
		//get view matrix
		m_ViewMatrix = m_Transform.GetModelMatrix();

		//build projection matrix
		m_ProjectionMatrix = glm::perspective(ComputeFOV(m_FocalLength), m_AspectRatio, m_ClipNear, m_ClipFar);

		//precompute view-projection matrix
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}

	float Camera::ComputeFOV(float fl)
	{
		float angsize =  2 * atan(m_SensorType.SensorHeight / (fl*2.0f));
		return 57.3f * angsize;
	}

	float Camera::GetClipNear() const
	{
		return m_ClipNear;
	}

	void Camera::SetClipNear(float f)
	{
		m_ClipNear = f;
	}

	float Camera::GetClipFar() const
	{
		return m_ClipFar;
	}

	void Camera::SetClipFar(float f)
	{
		m_ClipFar = f;
	}

	void Camera::RenderPostProcessing(PhysiCamFBOInputDesc inputFBODesc, unsigned int outputFramebufferId)
	{
		if (inputFBODesc.FramebufferId < 0 || inputFBODesc.ColorTextureId < 0 ||
			inputFBODesc.depthBufferId < 0 || outputFramebufferId < 0)
		{
			std::cerr << "Invalid framebuffer description or output framebuffer id" << std::endl;
			return;
		}

		if (m_AutoExposure)
		{
			//lerp the luminance value so the image doesnt flicker, also this simulates eye adaption
			float averageLuminance = m_PostProcessor->GetAverageLuminance(inputFBODesc.ColorTextureId);
			m_AverageSceneLuminance = glm::lerp(m_AverageSceneLuminance, averageLuminance, 2.0f * DeltaTime());

			float targetEV = ComputeTargetEV(m_AverageSceneLuminance);//multiply by 1000 so we dont need thousands of lumen in framebuffer
			targetEV += m_TargetEV;
			ApplyProgramAuto(targetEV);
		}
		else
		{
			//EV = log2((100.0f * pow(m_Aperture, 2)) / (m_Iso * m_ShutterSpeed));
			//EV = (1000.0f / 65.0f) * (pow(m_Aperture, 2) / (m_Iso*m_ShutterSpeed));
			//EV = 1.0f / EV;

			//EV = log2(pow(m_Aperture, 2) / m_ShutterSpeed * 100 / m_Iso);
			//EV = 1.2f * pow(2.0f, EV);
		}

		float exposure = GetStandardOutputBasedExposure(0.18f);
		//exposure *= 1000; //multiply by 1000 so we dont need thousands of lumen in framebuffer


		//float exposure = GetSaturationBasedExposure(EV100);
		//if (!m_AutoExposure) exposure = 500.0f * exposure; //TODO: WHY THE FUCK DO I NEED THIS?

		m_PostProcessor->Render(exposure, inputFBODesc, outputFramebufferId);

	}

	void Camera::SetSensorFromPreset(SensorPreset preset)
	{
		m_SensorType = m_SensorPresets[preset];
	}

	bool Camera::Init()
	{
		glewExperimental = true;
		if (glewInit()) {
			std::cerr << "failed to initialize gl3w";
			return false;
		}

		GL::ValidateExtensions();
		return true;
	}

	void Camera::UpdateScreenSize(int width, int height)
	{
		m_ScreenSize = glm::ivec2(width, height);
		m_AspectRatio = width / (float)height;
		m_PostProcessor->UpdateScreenSize();
	}


}