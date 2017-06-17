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
 *	@file camera.h
 */

#pragma once

#include <physicam/physicam_def.h>
#include <physicam/transform.h>
#include <physicam/PostProcessing.h>
#include <memory>


namespace PhysiCam
{

	typedef std::shared_ptr<Camera> CameraPtr;
	class PHYSICAM_DLL Camera
	{
		friend class PostProcessor;

	public:
		struct Sensor 
		{
			float SensorHeight;
			float CoC;
		};

		enum SensorPreset : unsigned int
		{
			SENSOR_4_3 = 0,			 // 4/3 sensor, used in compact cameras
			SENSOR_APS_C,		 //default for consumer DSLRs
			SENSOR_SMALL_FORMAT, //also known as 35 mm sensor, default for professional DSLRs
			SENSOR_MEDIUM_FORMAT,
			SENSOR_LARGE_FORMAT
		};

		Camera(int screenWidth, int screenHeight);
		~Camera();

		static bool Init();

		void Update(double deltaTime);
		void RenderPostProcessing(PhysiCamFBOInputDesc inputFBODesc, unsigned int outputFramebufferId);


		void UseAutoExposure(bool b){ m_AutoExposure = b; }
		bool UsingAutoExposure() { return m_AutoExposure; }

		Camera::Sensor SensorType() { return m_SensorType; }
		void SetSensorType(Camera::Sensor sensor) { m_SensorType = sensor; }
		void SetSensorFromPreset(SensorPreset preset);

		float MinAperture() const { return m_MinAperture; }
		void SetMinAperture(float val) { m_MinAperture = glm::max(val, m_MinAperture); }

		float MaxAperture() const { return m_MaxAperture; }
		void SetMaxAperture(float val) { m_MaxAperture = glm::min(val, m_MaxAperture); }

		float Aperture() const { return m_Aperture; }
		void SetAperture(float val) { m_Aperture = val; }

		float Iso() const { return m_Iso; }
		void SetIso(float val) { m_Iso = val; }

		float MaxIso() const { return m_MaxIso; }
		void SetMaxIso(float val) { m_MaxIso = glm::min(val, m_MaxIso); }

		float MinIso() const { return m_MinIso; }
		void SetMinIso(float val) { m_MinIso = glm::max(val, m_MinIso); }

		float MinShutterSpeed() const { return m_MinShutterSpeed; }
		void SetMinShutterSpeed(float val) { m_MinShutterSpeed = glm::max(val, m_MinShutterSpeed); }

		float MaxShutterSpeed() const { return m_MaxShutterSpeed; }
		void SetMaxShutterSpeed(float val) { m_MaxShutterSpeed = glm::min(val, m_MaxShutterSpeed); }

		float ShutterSpeed() const { return m_ShutterSpeed; }
		void SetShutterSpeed(float val) { m_ShutterSpeed = val; }

		float SensorHeight() const { return m_SensorType.SensorHeight; }
		void SetSensorHeight(float val) { m_SensorType.SensorHeight = val; }


		//OpenGL stuff
		float GetClipNear() const;
		void SetClipNear(float);

		float GetClipFar() const;
		void SetClipFar(float);

		glm::mat4 GetViewMatrix() const { return m_ViewMatrix; }
		void SetViewMatrix(glm::mat4 val) { m_ViewMatrix = val; }

		glm::mat4 GetProjectionMatrix() const { return m_ProjectionMatrix; }
		void SetProjectionMatrix(glm::mat4 val) { m_ProjectionMatrix = val; }

		glm::mat4 GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		float AspectRatio() const { return m_AspectRatio; }
		void SetAspectRatio(float val) { m_AspectRatio = val; }

		float FocalLength() const { return m_FocalLength; }
		void SetFocalLength(float val) { m_FocalLength = val; }

		Transform* GetTransform() { return &m_Transform; }

		void UpdateScreenSize(int width, int height);

		PostProcessor* GetPostProcessor(){ return m_PostProcessor; }

		float DeltaTime() const { return m_DeltaTime; }
	private:

		/*
		* Get an exposure using the Saturation-based Speed method.
		*/
		float GetSaturationBasedExposure();
		
		/*
		* Get an exposure using the Standard Output Sensitivity method.
		* Accepts an additional parameter of the target middle grey.
		*/
		float GetStandardOutputBasedExposure(float middleGrey = 0.18f);

		/*
		* compute ISO, shutterSpeed, and aperture automatically
		*/
		void ApplyProgramAuto(float targetEV);

		// Given an aperture, shutter speed, and exposure value compute the required ISO value
		float ComputeISO(float aperture, float shutterSpeed, float ev);

		// Given the camera settings compute the current exposure value
		float ComputeCurrentEV();

		// Using the light metering equation compute the target exposure value
		float ComputeTargetEV(float averageLuminance);

		// Compute vertical Field of view degrees from focal length
		float ComputeFOV(float fl);
		Transform m_Transform;


		//target EV, used for auto exposure
		float m_TargetEV;

		bool m_AutoExposure;

		//sensor parameters

		float m_MinIso;
		float m_MaxIso;
		float m_Iso;
		
		float m_MaxShutterSpeed;
		float m_MinShutterSpeed;
		float m_ShutterSpeed;
		
		//float m_SensorHeight;
		Sensor m_SensorType;
		Sensor m_SensorPresets[5];
		
		//lense parameters

		//measured in mm
		float m_FocalLength;
		
		float m_MinAperture;
		float m_MaxAperture;
		float m_Aperture;
		

		//openGL relevant values
		float m_ClipNear;
		float m_ClipFar;

		glm::ivec2 m_ScreenSize;
		float m_AspectRatio;


		float m_DeltaTime;

		float m_AverageSceneLuminance;

		PostProcessor *m_PostProcessor;
		
		glm::mat4 m_ViewMatrix, m_ProjectionMatrix, m_ViewProjectionMatrix;

	};
}