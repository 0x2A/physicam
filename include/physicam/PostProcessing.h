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
 *	@file PostProcessing.h
 */

#pragma once

#include <physicam/shader.h>
#include <physicam/RenderTexture.h>
#include <physicam/Framebuffer.h>

#define PC_MODEL_VERTEX_LOCATION 0
#define PC_MODEL_NORMAL_LOCATION 1
#define PC_MODEL_TEXCOORD_LOCATION 2
#define PC_MODEL_VERTEX_COLOR_LOCATION 3


#define USE_INCREMENTAL_GAUSS_BLUR	1
#define USE_LENSE_FLARE				1

namespace PhysiCam
{
	typedef struct
	{
		unsigned int FramebufferId;
		unsigned int ColorTextureId;
		unsigned int depthBufferId;
	} PhysiCamFBOInputDesc;

	enum class TonemappingMethod : int
	{
		Reinhard = 0,
		Filmic,
		Uncharted2
	};

	class Camera;
	class PHYSICAM_DLL PostProcessor
	{
		friend class Camera;
	public:
		PostProcessor(Camera *c);
		~PostProcessor();

		
		void Render(float exposure, PhysiCamFBOInputDesc inputFBODesc, unsigned int outputFramebufferId);

		void UpdateScreenSize();

		float GetAverageLuminance(unsigned int inputTexture);
		
		/*** postprocessing effects functions ***/

		/* Bloom */
		bool BloomEnabled() const { return m_BloomEnabled; }
		void SetBloomEnabled(bool val) { m_BloomEnabled = val; }

		float BloomThreshold() const { return m_BloomThreshold; }
		void SetBloomThreshold(float val) { m_BloomThreshold = val; }

		void SetBloomSpead(int id, float val) { m_BloomSpreads[id] = val; }
		void SetBloomIntensity(float val) { m_BloomIntensity = val; }
		void SetBloomIntensity(int id, float val) { m_BloomStrengths[id] = val; }
		
		int DirtTextureId() const { return m_DirtTextureId; }
		void SetDirtTextureId(int val) { m_DirtTextureId = val; }

		/* Tonemapping */
		bool TonemappingEnabled() const { return m_ToneMappingEnabled; }
		void SetTonemappingEnabled(bool val) { m_ToneMappingEnabled = val; }
		void SetTonemappingMethod(TonemappingMethod method) { m_ToneMappingMethod = method; }

		/* DoF */
		bool DoFEnabled() const { return m_DoFEnabled; }
		void SetDoFEnabled(bool val) { m_DoFEnabled = val; }

		float DoFAberation() const { return m_DoFAberation; }
		void SetDoFAberation(float val) { m_DoFAberation = val; }

		float DoFFocalDistance() const { return m_DoFFocalDistance; }
		void SetDoFFocalDistance(float val) { m_DoFFocalDistance = val; }
		
		bool DoFAutofocus() const { return m_DoFAutofocus; }
		//ignored when using autofocus
		void SetDoFAutofocus(bool val) { m_DoFAutofocus = val; }
		
		bool DoFShowFocus() const { return m_DoFShowFocus; }
		void SetDoFShowFocus(bool val) { m_DoFShowFocus = val; }
		
		bool DoFVignetting() const { return m_DoFVignetting; }
		void SetDoFVignetting(bool val) { m_DoFVignetting = val; }

		float DoFMaxBlur() const { return m_DoFMaxBlur; }
		void SetDoFMaxBlur(float val) { m_DoFMaxBlur = val; }

		float LensDistortionAmount() const { return m_LensDistortionAmount; }
		void SetLensDistortionAmount(float val) { m_LensDistortionAmount = val; }

		float MaxNoise() const { return m_MaxNoise; }
		void SetMaxNoise(float val) { m_MaxNoise = val; }
		float MinNoise() const { return m_MinNoise; }
		void SetMinNoise(float val) { m_MinNoise = val; }

	private:
		void RenderFullscreenQuad();

		void InitFBOs();
		void DeleteFBOs();

		void InitQuadMesh();

		void InitShaders();
		void DeleteShaders();

		void InitRenderTextures();
		void DeleteRenderTextures();

		void ApplyLuminance(float exposure, unsigned int inputTexture);
		void ApplyLenseDistortion(unsigned int colTex, unsigned int depthTex);
		void ApplyBloom(RenderTexturePtr tex, unsigned int outputFBO);
		void ApplyToneMapping(RenderTexturePtr tex, unsigned int outputFBO);
		void ApplyDoF(RenderTexturePtr tex, unsigned int depthTextureId, unsigned int outputFBO);

		void RenderFlares();


		Camera *m_Camera;

		//Shader objects
		ShaderPtr m_ShaderBlitScreen;
		ShaderPtr m_ShaderDownsample;
		ShaderPtr m_ShaderLensDistortion;
		ShaderPtr m_ShaderBrightPass;
		ShaderPtr m_ShaderIncrementalGaussBlur;
		ShaderPtr m_ShaderHorizontalBlur;
		ShaderPtr m_ShaderVerticalBlur;
		ShaderPtr m_ShaderBloomCompose;
		ShaderPtr m_ShaderLenseBloomCompose;
		ShaderPtr m_ShaderLenseFlare;
		ShaderPtr m_DoFShader;
		ShaderPtr m_ShaderToneMapping;

		//buffers for fullscreen quad mesh
		unsigned int m_QuadVBO;
		unsigned int m_IndexBuffer, m_VertexBuffer, m_NormalBuffer, m_TexCoordBuffer;

		//FBOs
		FramebufferPtr m_DownSampleFBO;
		FramebufferPtr m_LenseDistortionFBO;
		FramebufferPtr m_LenseFlareFBO;
		FramebufferPtr m_BloomhorFBOs[5];
		FramebufferPtr m_BloomvertFBOs[5];
		FramebufferPtr m_BloomOutputFBO;
		FramebufferPtr m_BrightnessPassFBO;
		FramebufferPtr m_SceneFBOs[2];

		/*** postprocessing effects parameters ***/
		
		//lense distortion
		float m_LensDistortionAmount;

		//bloom
		bool m_BloomEnabled;
		float m_BloomThreshold;
		float m_BloomSpreads[5];
		float m_BloomStrengths[5];
		float m_BloomIntensity;
		int m_DirtTextureId;

		//Depth of field
		bool m_DoFEnabled;
		float m_DoFAberation;
		float m_DoFMaxBlur;
		//focal distance value in meters
		float m_DoFFocalDistance;
		bool m_DoFShowFocus;
		bool m_DoFVignetting;
		bool m_DoFAutofocus;

		float m_MaxNoise;
		float m_MinNoise;

		//Tonemapping
		bool m_ToneMappingEnabled;
		TonemappingMethod m_ToneMappingMethod;

	};

}