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
 *	@file PostProcessing.cpp
 */

#include <physicam/PostProcessing.h>
#include <physicam/camera.h>
#include <physicam/Vertex.h>
#include <physicam/Face.h>
#include <physicam/ShaderCode.h>
#include <physicam/RenderTexture.h>

#include <GL/glew.h>

namespace PhysiCam
{
	RenderTexturePtr LensDistColorTexture;
	RenderTexturePtr LensDistDepthTexture;
	RenderTexturePtr bloomBrightnessTexture;
	RenderTexturePtr bloomLenseFlareBrightnessTexture;
	RenderTexturePtr sceneTextures[2];
	RenderTexturePtr downSampleTexture;
	RenderTexturePtr bloomTextureHor[5];
	RenderTexturePtr bloomTextureVert[5];
	RenderTexturePtr bloomOutputTex;
	RenderTexturePtr lenseFlareTexture;

	PostProcessor::PostProcessor(Camera *c) : m_Camera(c), m_BloomThreshold(1.0f), m_BloomEnabled(true), m_DirtTextureId(-1), 
		m_DoFEnabled(true), m_ToneMappingMethod(TonemappingMethod::Filmic), m_DoFAberation(0.6f), m_DoFFocalDistance(3.0f), 
		m_DoFAutofocus(true), m_DoFVignetting(true), m_DoFShowFocus(false), m_DoFMaxBlur(3.0f), m_LensDistortionAmount(0.1f),
		m_MaxNoise(0.45f), m_MinNoise(0.015f)
	{
		InitFBOs();
		InitQuadMesh();
		InitShaders();
		InitRenderTextures();

#if USE_INCREMENTAL_GAUSS_BLUR
		m_BloomSpreads[0] = 16.0f;
		m_BloomSpreads[1] = 16.0f;
		m_BloomSpreads[2] = 24.0f;
		m_BloomSpreads[3] = 24.0f;
		m_BloomSpreads[4] = 32.0f;
		m_BloomStrengths[0] = 0.75f;
		m_BloomStrengths[1] = 0.75f;
		m_BloomStrengths[2] = 1.0f;
		m_BloomStrengths[3] = 1.0f;
		m_BloomStrengths[4] = 1.0f;
#else
		m_BloomSpreads[0] = 1.0f;
		m_BloomSpreads[1] = 4.0f;
		m_BloomSpreads[2] = 8.0f;
		m_BloomSpreads[3] = 16.0f;
		m_BloomSpreads[4] = 32.0f;
		m_BloomStrengths[0] = 0.2f;
		m_BloomStrengths[1] = 0.3f;
		m_BloomStrengths[2] = 0.5f;
		m_BloomStrengths[3] = 0.6f;
		m_BloomStrengths[4] = 0.8f;
#endif
		m_BloomIntensity = 0.5f;
	}

	PostProcessor::~PostProcessor()
	{
		glDeleteBuffers(1, &m_QuadVBO);
		DeleteFBOs();
		DeleteShaders();
		DeleteRenderTextures();
	}


	void PostProcessor::RenderFullscreenQuad()
	{
		glBindVertexArray(m_QuadVBO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

	void PostProcessor::InitQuadMesh()
	{
		VertexList vertices;
		FaceList faces;

		Vertex v(glm::vec3(-1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 0)); vertices.push_back(v);
		v = Vertex(glm::vec3(1, -1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 0)); vertices.push_back(v);
		v = Vertex(glm::vec3(1, 1, 0), glm::vec3(0, 0, 1), glm::vec2(1, 1)); vertices.push_back(v);
		v = Vertex(glm::vec3(-1, 1, 0), glm::vec3(0, 0, 1), glm::vec2(0, 1)); vertices.push_back(v);
		faces.push_back(Face(0, 1, 3));
		faces.push_back(Face(1, 2, 3));

		//generate vertex buffer object
		glGenVertexArrays(1, &m_QuadVBO);
		glBindVertexArray(m_QuadVBO);
		
		// buffer for indices
		glGenBuffers(1, &m_IndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face) * faces.size(), &faces[0], GL_STATIC_DRAW);

		//buffer for vertices
		glGenBuffers(1, &m_VertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

		//setup data locations

		// Positions (location = 0)
		glEnableVertexAttribArray(PC_MODEL_VERTEX_LOCATION);
		glVertexAttribPointer(PC_MODEL_VERTEX_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);

		//Normals (location = 1)
		glEnableVertexAttribArray(PC_MODEL_NORMAL_LOCATION);
		glVertexAttribPointer(PC_MODEL_NORMAL_LOCATION, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));

		//texcoords (location = 2)
		glEnableVertexAttribArray(PC_MODEL_TEXCOORD_LOCATION);
		glVertexAttribPointer(PC_MODEL_TEXCOORD_LOCATION, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) * 2));

		//vertexColor (location = 3)
		glEnableVertexAttribArray(PC_MODEL_VERTEX_COLOR_LOCATION);
		glVertexAttribPointer(PC_MODEL_VERTEX_COLOR_LOCATION, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) * 2 + sizeof(glm::vec2)));

		// unbind buffers
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void PostProcessor::InitShaders()
	{
		m_ShaderBlitScreen = Shader::Create(ScreenAlignedVertSrc, BlitScreenSrc);
		m_ShaderDownsample = Shader::Create(ScreenAlignedVertSrc, DownsampleScreenSrc);
		m_ShaderLensDistortion = Shader::Create(ScreenAlignedVertSrc, LensDistortionSrc);
		m_ShaderBrightPass = Shader::Create(ScreenAlignedVertSrc, BrightPassSrc);
		m_ShaderHorizontalBlur = Shader::Create(ScreenAlignedVertSrc, BlurHorizontalSrc);
		m_ShaderVerticalBlur = Shader::Create(ScreenAlignedVertSrc, BlurVerticalSrc);
		m_ShaderIncrementalGaussBlur = Shader::Create(ScreenAlignedVertSrc, IncrGaussBlurSrc);
		m_ShaderBloomCompose = Shader::Create(ScreenAlignedVertSrc, BloomComposeSrc);
		m_ShaderLenseFlare = Shader::Create(ScreenAlignedVertSrc, LenseFlareSrc);
		m_ShaderLenseBloomCompose = Shader::Create(ScreenAlignedVertSrc, BloomLenseComposeSrc);
		m_ShaderToneMapping = Shader::Create(ScreenAlignedVertSrc, ToneMapperSrc);
		m_DoFShader = Shader::Create(ScreenAlignedVertSrc, DoFSrc);
	}
	void PostProcessor::DeleteShaders()
	{
// 		DelPtr(m_ShaderBlitScreen);
// 		DelPtr(m_ShaderDownsample);
// 		DelPtr(m_ShaderBrightpass);
// 		DelPtr(m_ShaderHorizontalBlur);
// 		DelPtr(m_ShaderVerticalBlur);
// 		DelPtr(m_ShaderBloomCompose);
// 		DelPtr(m_ShaderLenseFlare);
	}


	void PostProcessor::InitFBOs()
	{
		auto screenSize = m_Camera->m_ScreenSize;
		m_LenseDistortionFBO = Framebuffer::Create(screenSize.x, screenSize.y);
		m_SceneFBOs[0] = Framebuffer::Create(screenSize.x, screenSize.y);
		m_SceneFBOs[1] = Framebuffer::Create(screenSize.x, screenSize.y);

		m_BrightnessPassFBO = Framebuffer::Create(screenSize.x*0.5f, screenSize.y*0.5f);

		m_DownSampleFBO = Framebuffer::Create(screenSize.x*0.5f, screenSize.y*0.5f);

		float size = 0.5f;
		for (int i = 0; i < 5; i++)
		{
			m_BloomhorFBOs[i] = Framebuffer::Create(screenSize.x*size, screenSize.y*size);
			m_BloomvertFBOs[i] = Framebuffer::Create(screenSize.x*size, screenSize.y*size);
			size *= 0.5f;
		}
		m_BloomOutputFBO = Framebuffer::Create(screenSize.x*0.5f, screenSize.y*0.5f);
		m_LenseFlareFBO = Framebuffer::Create(screenSize.x*0.5f, screenSize.y*0.5f);


	}

	void PostProcessor::InitRenderTextures()
	{
		auto screenSize = m_Camera->m_ScreenSize;
		
		LensDistColorTexture = m_LenseDistortionFBO->CreateAndAttachTexture(Framebuffer::COLOR0, RenderTexture::TEXTURE_2D, RenderTexture::RGB32F);
		LensDistDepthTexture = m_LenseDistortionFBO->CreateAndAttachTexture(Framebuffer::COLOR1, RenderTexture::TEXTURE_2D, RenderTexture::R32F); //since we dont do depth testing here, DEPTH_ATTACHMENT wont work. we just use a r32F texture
		
		downSampleTexture = m_DownSampleFBO->CreateAndAttachTexture(Framebuffer::COLOR0, RenderTexture::TEXTURE_2D, RenderTexture::RGB32F, true);
		
		bloomBrightnessTexture = m_BrightnessPassFBO->CreateAndAttachTexture(Framebuffer::COLOR1, RenderTexture::TEXTURE_2D, RenderTexture::RGB32F, false);
		bloomLenseFlareBrightnessTexture = m_BrightnessPassFBO->CreateAndAttachTexture(Framebuffer::COLOR2, RenderTexture::TEXTURE_2D, RenderTexture::RGB16F, false);

		sceneTextures[0] = m_SceneFBOs[0]->CreateAndAttachTexture(Framebuffer::COLOR0, RenderTexture::TEXTURE_2D, RenderTexture::RGB32F);
		sceneTextures[1] = m_SceneFBOs[1]->CreateAndAttachTexture(Framebuffer::COLOR0, RenderTexture::TEXTURE_2D, RenderTexture::RGB32F);
				
		lenseFlareTexture = m_LenseFlareFBO->CreateAndAttachTexture(Framebuffer::COLOR0, RenderTexture::TEXTURE_2D, RenderTexture::RGB32F);
		
		for (int i = 0; i < 5; i++)
		{
			bloomTextureHor[i] = m_BloomhorFBOs[i]->CreateAndAttachTexture(Framebuffer::COLOR0, RenderTexture::TEXTURE_2D, RenderTexture::RGB32F); 
			bloomTextureVert[i] = m_BloomvertFBOs[i]->CreateAndAttachTexture(Framebuffer::COLOR0, RenderTexture::TEXTURE_2D, RenderTexture::RGB32F);	
		}

		bloomOutputTex = m_BloomOutputFBO->CreateAndAttachTexture(Framebuffer::COLOR0, RenderTexture::TEXTURE_2D, RenderTexture::RGB32F);
	}

	void PostProcessor::DeleteRenderTextures()
	{
		downSampleTexture.reset();
		sceneTextures[0].reset();
		sceneTextures[1].reset();

		for (int i = 0; i < 5; i++)
		{
			bloomTextureHor[i].reset();
			bloomTextureVert[i].reset();
		}
	}

	void PostProcessor::UpdateScreenSize()
	{
		DeleteRenderTextures();
		InitRenderTextures();
	}

	void PostProcessor::Render(float exposure, PhysiCamFBOInputDesc inputFBODesc, unsigned int outputFramebufferId)
	{
		//first apply lense distortion using the camera settings
		ApplyLenseDistortion(inputFBODesc.ColorTextureId, inputFBODesc.depthBufferId);

		ApplyLuminance(exposure, LensDistColorTexture->GetTextureId());
		int indx = 0;

		//apply bloom if enabled
		if (m_BloomEnabled)
		{
			int t = (indx + 1) % 2;
			ApplyBloom(sceneTextures[indx], m_SceneFBOs[t]->GetID());
			indx = t;
		}

		if (m_DoFEnabled)
		{
			int t = (indx + 1) % 2;
			ApplyDoF(sceneTextures[indx], LensDistDepthTexture->GetTextureId(), m_SceneFBOs[t]->GetID());
			indx = t;
		}
		
		if (m_ToneMappingEnabled)
		{
			ApplyToneMapping(sceneTextures[indx], outputFramebufferId);
		}


		if (!m_ToneMappingEnabled)
		{
			sceneTextures[indx]->Bind(0);

			//blit final image to output
			auto scrSize = m_Camera->m_ScreenSize;
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, outputFramebufferId);
			m_ShaderBlitScreen->Bind();
			m_ShaderBlitScreen->SetParameteri("tex", 0);
			m_ShaderBlitScreen->SetParameterf("exp", 1.0f);
			glViewport(0, 0, scrSize.x, scrSize.y);
			RenderFullscreenQuad();
		}
	}

	float PostProcessor::GetAverageLuminance(unsigned int inputTexture)
	{
		//bind output framebuffer and bind renderTexture to it
		m_DownSampleFBO->Bind();

		//bind input texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, inputTexture);

		m_ShaderDownsample->Bind();
		m_ShaderDownsample->SetParameteri("tex", 0);

		//auto scrSize = m_Camera->m_ScreenSize;
		//glViewport(0, 0, scrSize.x*0.5f, scrSize.y*0.5f);
		RenderFullscreenQuad();
		downSampleTexture->GenerateMipMaps();
		
		float *fPixel = new float[3];
		glGetTexImage(GL_TEXTURE_2D, 9, GL_RGB, GL_FLOAT, fPixel); //9 is highest maximum mipmap level and should always end in a 1x1 texture

		//relative luminance: https://en.wikipedia.org/wiki/Relative_luminance
		float ret = (0.2126f*fPixel[0] + 0.7152f*fPixel[1] + 0.0722f*fPixel[2]);
		return ret;
	}
	


	void PostProcessor::ApplyLuminance(float exposure, unsigned int inputTexture)
	{
		//render to scene framebuffer
		m_SceneFBOs[0]->Bind();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, inputTexture);

		m_ShaderBlitScreen->Bind();
		m_ShaderBlitScreen->SetParameteri("tex", 0);
		m_ShaderBlitScreen->SetParameterf("exp", exposure);

		RenderFullscreenQuad();
	}


	void PostProcessor::ApplyLenseDistortion(unsigned int colTex, unsigned int depthTex)
	{
		m_LenseDistortionFBO->Bind();
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTex);

		auto scrSize = m_Camera->m_ScreenSize;
		m_ShaderLensDistortion->Bind();
		m_ShaderLensDistortion->SetParameteri("tex", 0);
		m_ShaderLensDistortion->SetParameteri("depth", 1);
		m_ShaderLensDistortion->SetParameterf("k", m_LensDistortionAmount);
		m_ShaderLensDistortion->SetParameterVec2("screenSize", glm::vec2(scrSize));
		RenderFullscreenQuad();
	}

	void PostProcessor::ApplyBloom(RenderTexturePtr tex, unsigned int outputFBO)
	{
		//render bright pass to temp fbo
		
		m_BrightnessPassFBO->Bind();
		
		//bind input texture
		tex->Bind(0);

		m_ShaderBrightPass->Bind();
		m_ShaderBrightPass->SetParameterf("threshold", m_BloomThreshold);
		m_ShaderBrightPass->SetParameterf("LenseFlareThreshold", m_BloomThreshold*10.f);
		m_ShaderBrightPass->SetParameteri("tex", 0);
		RenderFullscreenQuad();


		auto scrSize = m_Camera->m_ScreenSize;
		RenderTexturePtr inp = bloomBrightnessTexture;
		for (int i = 0; i < 5; i++)
		{
			//horizontal blur
			m_BloomhorFBOs[i]->Bind();
			inp->Bind(0);

			auto tSize = bloomTextureHor[i]->GetSize();
#if USE_INCREMENTAL_GAUSS_BLUR
			static glm::vec2 horBlurDir = glm::vec2(1.0f, 0.0f);
			m_ShaderIncrementalGaussBlur->Bind();
			m_ShaderIncrementalGaussBlur->SetParameteri("tex", 0);
			m_ShaderIncrementalGaussBlur->SetParameterf("radius", m_BloomSpreads[i]);
			m_ShaderIncrementalGaussBlur->SetParameterVec2("resolution", (glm::vec2)tSize);
			m_ShaderIncrementalGaussBlur->SetParameterVec2("uBlurDirection", horBlurDir);
#else
			m_ShaderHorizontalBlur->Bind();
			m_ShaderHorizontalBlur->SetParameteri("tex", 0);
			m_ShaderHorizontalBlur->SetParameterf("resolution", scrSize.x);
			m_ShaderHorizontalBlur->SetParameterf("radius", m_BloomSpreads[i]);
#endif
			RenderFullscreenQuad();

			//vertical blur
			m_BloomvertFBOs[i]->Bind();
			bloomTextureHor[i]->Bind(0);

#if USE_INCREMENTAL_GAUSS_BLUR
			static glm::vec2 vertBlurDir = glm::vec2(0.0f, 1.0f);
			//m_ShaderIncrementalGaussBlur->Bind();
			//m_ShaderIncrementalGaussBlur->SetParameteri("tex", 0);
			//m_ShaderIncrementalGaussBlur->SetParameterf("radius", m_BloomSpreads[i]);
			//m_ShaderIncrementalGaussBlur->SetParameterVec2("resolution", (glm::vec2)scrSize);
			m_ShaderIncrementalGaussBlur->SetParameterVec2("uBlurDirection", vertBlurDir);
#else
			m_ShaderVerticalBlur->Bind();
			m_ShaderVerticalBlur->SetParameteri("tex", 0);
			m_ShaderVerticalBlur->SetParameterf("resolution", scrSize.y);
			m_ShaderVerticalBlur->SetParameterf("radius", m_BloomSpreads[i]);
#endif
			RenderFullscreenQuad();

			inp = bloomTextureVert[i];
		}


		//render composition pass to scene fbo
		m_BloomOutputFBO->Bind();

		//compose bloom passes
		for (int i = 0; i < 5; i++)
			bloomTextureVert[i]->Bind(i);

		m_ShaderBloomCompose->Bind();
		int texLocations[] = { 0, 1, 2, 3, 4 };
		m_ShaderBloomCompose->SetParameteriv("tex", 5, texLocations);
		m_ShaderBloomCompose->SetParameterfv("strengths", 5, m_BloomStrengths);
		m_ShaderBloomCompose->SetParameterf("intensity", m_BloomIntensity);

		RenderFullscreenQuad();


		// ** Apply lenseflare **
#if USE_LENSE_FLARE
		m_LenseFlareFBO->Bind();
		
		const float ChromaticDistortion = 1.5f;
		glm::vec3 ChromaticDistortionVector(-ChromaticDistortion / scrSize.x*0.5f, 0.0f, ChromaticDistortion /scrSize.y*0.5f);

		bloomLenseFlareBrightnessTexture->Bind(0);
		m_ShaderLenseFlare->Bind();
		m_ShaderLenseFlare->SetParameteri("tex", 0);
		m_ShaderLenseFlare->SetParameterf("HaloWidth", 0.4f);
		m_ShaderLenseFlare->SetParameterVec3("ChromaticDistortionVector", ChromaticDistortionVector);
		m_ShaderLenseFlare->SetParameterVec2("screenSize", glm::vec2(scrSize));

		RenderFullscreenQuad();
#endif

		// compose bloom and lenseflare
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, outputFBO);
		bloomOutputTex->Bind(0);
		lenseFlareTexture->Bind(1);
		tex->Bind(2);

		if (m_DirtTextureId > 0)
		{
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, m_DirtTextureId);
		}

		m_ShaderLenseBloomCompose->Bind();
		m_ShaderLenseBloomCompose->SetParameteri("bloomPass", 0);
		m_ShaderLenseBloomCompose->SetParameteri("lenseFlare", 1);
		m_ShaderLenseBloomCompose->SetParameteri("baseTex", 2);
		if (m_DirtTextureId > 0) m_ShaderLenseBloomCompose->SetParameteri("dirtTexture", 3);
		m_ShaderLenseBloomCompose->SetParameteri("hasDirtTexture", m_DirtTextureId < 0 ? 0 : 1);
		m_ShaderLenseBloomCompose->SetParameterf("strength", m_BloomIntensity);
		glViewport(0, 0, scrSize.x, scrSize.y);
		RenderFullscreenQuad();

		
	}


	void PostProcessor::ApplyToneMapping(RenderTexturePtr tex, unsigned int outputFBO)
	{
		static float tTime = 0.0;
		tTime += m_Camera->DeltaTime()*0.001f;

		float noiseAmount = m_MinNoise + ((m_MaxNoise - m_MinNoise) / (m_Camera->MaxIso() - 1.0f)) * (m_Camera->Iso() - 1.0f);

		tex->Bind(0);

		//blit final image to output
		auto scrSize = m_Camera->m_ScreenSize;
		glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
		m_ShaderToneMapping->Bind();
		m_ShaderToneMapping->SetParameteri("tex", 0);
		m_ShaderToneMapping->SetParameterf("grainamount", noiseAmount);
		m_ShaderToneMapping->SetParameterf("timer", tTime);
		m_ShaderToneMapping->SetParameterVec2("screenSize", glm::vec2(scrSize));
		m_ShaderToneMapping->SetParameteri("tonemappingMethod", static_cast<int>(m_ToneMappingMethod));
		glViewport(0, 0, scrSize.x, scrSize.y);
		RenderFullscreenQuad();
	}

	void PostProcessor::ApplyDoF(RenderTexturePtr tex, unsigned int depthTextureId, unsigned int outputFBO)
	{
		tex->Bind(0);
		glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
		
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthTextureId);

		auto scrSize = m_Camera->m_ScreenSize;

		m_DoFShader->Bind();
		m_DoFShader->SetParameteri("ColorTexture", 0);
		m_DoFShader->SetParameteri("DepthTexture", 1);
		m_DoFShader->SetParameteri("showFocus", DoFShowFocus());
		m_DoFShader->SetParameteri("vignetting", DoFVignetting());
		m_DoFShader->SetParameteri("autofocus", DoFAutofocus());
		m_DoFShader->SetParameterf("fringe", DoFAberation());// = 0.7
		m_DoFShader->SetParameterf("focalDepth", DoFFocalDistance());
		m_DoFShader->SetParameterf("focalLength", m_Camera->FocalLength());
		m_DoFShader->SetParameterf("fstop", m_Camera->Aperture());
		m_DoFShader->SetParameterf("maxblur", DoFMaxBlur());
		m_DoFShader->SetParameterf("CoC", m_Camera->SensorType().CoC);
		m_DoFShader->SetParameterVec2("ScreenSize", (glm::vec2)scrSize);
		m_DoFShader->SetParameterVec2("CameraClips", glm::vec2(m_Camera->GetClipNear(), m_Camera->GetClipFar()));
		glViewport(0, 0, scrSize.x, scrSize.y);
		RenderFullscreenQuad();
	}

	void PostProcessor::DeleteFBOs()
	{
		m_DownSampleFBO.reset();
		m_SceneFBOs[0].reset();
		m_SceneFBOs[1].reset();

		for (int i = 0; i < 5; i++)
		{
			m_BloomhorFBOs[i].reset();
			m_BloomvertFBOs[i].reset();
		}
		
		m_LenseFlareFBO.reset();
	}

	void PostProcessor::RenderFlares()
	{

	}



}