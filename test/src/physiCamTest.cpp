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
*	@file physiCamTest.cpp
*/

#include <NoWork/framework.h>
#include <physiCamTest.h>
#include <NoWork/Log.h>
#include <NoWork/Input.h>
#include <NoWork/Font.h>
#include <NoWork/Model.h>
#include <AntTweakBar/AntTweakBar.h>

#define _PI	3.141592653589793f

#define ASSETS_FOLDER std::string("../../../assets/")

//parameters used for UI control
float m_FocalLength;
float m_ISO;
float m_Aperture;
float m_ShutterSpeed;
PhysiCam::Camera::SensorPreset m_SensorType;

bool m_BloomEnabled;
float m_BloomThreshold;
float m_BloomStrength;

float m_LenseDistAmount;

bool m_AutoModeLastValue; //this is used to avoid calling TwSetParam every frame
bool m_AutoMode;

bool m_DoFEnabled;
bool m_DoFAutofocus;
bool m_DoFVignetting;
bool m_DoFShowFocus;
float m_FocalDistance;
float m_DoFMaxBlur;

bool m_ToneMappingEnabled;
PhysiCam::TonemappingMethod m_TonemappingMethod;

ModelPtr m_Model;
ModelPtr m_Skydome;
MeshPtr m_Floor;

bool wireframe;

ShaderPtr m_UnlitShader;
ShaderPtr m_MaterialShader;
ShaderPtr m_SkydomeShader;

glm::vec3 camRot, camPos;
glm::vec3 movement;

glm::vec4 m_LightPos;
float lightIntensity;

PhysiCam::PhysiCamFBOInputDesc fboInpDesc;

Texture2DPtr m_TexCheckerboard;
Texture2DPtr m_SkyboxTexture;
Texture2DPtr m_LenseDirtTexture;

struct OpticalGhost
{
	float _reflections[2];
};

void CreateFlare(PhysiCam::Camera* cam, glm::vec3 lightDir)
{
	float theta_e = acosf(glm::dot(cam->GetTransform()->Forward(), lightDir));
	glm::vec4 axis = glm::normalize(cam->GetProjectionMatrix() * glm::vec4((cam->GetTransform()->WorldPosition() + lightDir * 10000.0f), 1.0f));
	float H_a = 0.5f;

	float a = cam->Aperture();
	float n = 7;

	std::vector<OpticalGhost> ghosts;
	for (int i = 0; i < a; ++i)
	{
		for (int j = i + 1; j < a; ++j)
		{
			OpticalGhost ghost;
			ghost._reflections[0] = j;
			ghost._reflections[1] = i;
			ghosts.push_back(ghost);
		}
	}
	for (int i = a+1; i < n; ++i)
	{
		for (int j = i + 1; j < n; ++j)
		{
			OpticalGhost ghost;
			ghost._reflections[0] = j;
			ghost._reflections[1] = i;
			ghosts.push_back(ghost);
		}
	}


}

void TestGame::Init()
{
	LOG_DEBUG("Initializing game");

	//position camera
	m_Camera = m_Renderer->GetCamera();


	//scene properties
	//glClearColor(0.42f, 0.69f, 0.92f, 1.0f);
	m_Renderer->SetAlphaBlending(true);
	m_Renderer->SetAnisotropicFiltering(16);
	wireframe = false;
	m_LightPos = glm::vec4(1, 1, 1, 0.0f);
	lightIntensity = 98000; //this value is in kLm, so 98 meaens 98000 lumen
	m_BloomStrength = 0.5f;
	m_BloomThreshold = 1.0f;
	m_BloomEnabled = true;

	//load font
	m_Font = Font::Create(ASSETS_FOLDER + "fonts/Roboto-Regular.ttf", 30);
	
	//get default camera parameters for HUD
	m_AutoMode = true;
	m_AutoModeLastValue = false;
	m_FocalLength = m_Camera->FocalLength();
	m_ISO = m_Camera->Iso();
	m_ShutterSpeed = 1.0f / m_Camera->ShutterSpeed();
	m_Aperture = m_Camera->Aperture();
	m_SensorType = PhysiCam::Camera::SENSOR_SMALL_FORMAT;

	camPos = glm::vec3(2, 6, 15);

	m_DoFMaxBlur = 3.0f;
	m_DoFEnabled = true;
	m_DoFAutofocus = true;
	m_DoFVignetting = true;
	m_DoFShowFocus = false;

	m_LenseDistAmount = 0.1f;

	m_ToneMappingEnabled = true;
	m_TonemappingMethod = PhysiCam::TonemappingMethod::Filmic;

	//Setup UI
	SetupUI();

	//Load shaders for scene rendering
	m_UnlitShader = Shader::Load(ASSETS_FOLDER + "shader/unlit.vert", ASSETS_FOLDER + "shader/unlit.frag");
	m_MaterialShader = Shader::Load(ASSETS_FOLDER + "shader/material.vert", ASSETS_FOLDER + "shader/material.frag");
	m_SkydomeShader = Shader::Load(ASSETS_FOLDER + "shader/skydome.vert", ASSETS_FOLDER + "shader/skydome.frag");

	//load lense flare dirt texture and provide to PhysiCam
	m_LenseDirtTexture = Texture2D::Load(ASSETS_FOLDER + "textures/lensflare_dirt.png");
	m_Camera->GetPostProcessor()->SetDirtTextureId(m_LenseDirtTexture->GetTextureId());

	//binding escape key to exit function
	Input::BindKey(KEY_ESCAPE, this, &TestGame::Exit);

}

void TestGame::SetupUI()
{
	bar = TwNewBar("Parameters");

	TwAddVarRW(bar, "Light direction", TW_TYPE_DIR3F, &m_LightPos,
		" label='Light direction' group=Light ");

	TwAddVarRW(bar, "Light intensity", TW_TYPE_FLOAT, &lightIntensity,
		" label='Light intensity (kLm)' min=0 step=0.1 group=Light");

	TwAddVarRW(bar, "focal length", TW_TYPE_FLOAT, &m_FocalLength,
		" label='Focal length (mm)' min=1 max=800 step=2 help='focal length in mm' group=Lens ");

	TwAddVarRW(bar, "lensDistortion", TW_TYPE_FLOAT, &m_LenseDistAmount,
		" label='Lens Distortion' min=-3 max=3 step=0.01 help='Amount of lens distortion. Positive = barrel, Negative = pincushion' group=Lens ");

	//Tonemapping
	TwEnumVal sensorTypeEV[] = { { (int)PhysiCam::Camera::SENSOR_4_3, "4/3 Sensor" },{ (int)PhysiCam::Camera::SENSOR_APS_C , "APS-C Sensor" },
	{ (int)PhysiCam::Camera::SENSOR_SMALL_FORMAT, "Small Format (35mm)" },{ (int)PhysiCam::Camera::SENSOR_MEDIUM_FORMAT, "Medium Format" },
	{ (int)PhysiCam::Camera::SENSOR_LARGE_FORMAT, "Large Format" } };
	TwType SensorTypeT;
	// Defining season enum type
	SensorTypeT = TwDefineEnum("Sensor type", sensorTypeEV, 5);
	// Adding methods to bar

	TwAddVarRW(bar, "SensorType", SensorTypeT, &m_SensorType,
		" label='Sensor type' group=Settings");


	TwAddVarRW(bar, "Automode", TW_TYPE_BOOLCPP, &m_AutoMode,
		" label='Auto mode' group=Settings");

	TwAddVarRW(bar, "iso", TW_TYPE_FLOAT, &m_ISO,
		" label='ISO' min=100 max=6400 step=10 help='light sensitivity in ISO' group=Settings");

	TwAddVarRW(bar, "shutter Speed", TW_TYPE_FLOAT, &m_ShutterSpeed,
		" label='Shutter Speed' min=5 max=4000 step=10 help='Shutter speed in 1/x' group=Settings");

	TwAddVarRW(bar, "Aperture", TW_TYPE_FLOAT, &m_Aperture,
		" label='Aperture' min=1 max=22 step=0.1 help='Aperture f stops' group=Settings");

	TwDefine("Parameters/Lens group=Camera");
	TwDefine("Parameters/Settings group=Camera");
	
	//bloom
	TwAddVarRW(bar, "bloomEnabled", TW_TYPE_BOOLCPP, &m_BloomEnabled,
		" label='Enabled' group=Bloom");

	TwAddVarRW(bar, "bloomThreshold", TW_TYPE_FLOAT, &m_BloomThreshold,
		" label='Threshold' min=0 step=0.1 group=Bloom");
	
	TwAddVarRW(bar, "bloomStrength", TW_TYPE_FLOAT, &m_BloomStrength,
		" label='Strength' min=0 step=0.1 group=Bloom");
	
	TwDefine(" Parameters/Bloom group=Postprocessing");  // group Color is moved into group Properties


	//DoF
	TwAddVarRW(bar, "DoFEnabled", TW_TYPE_BOOLCPP, &m_DoFEnabled,
		" label='Enabled' group=Bokeh");
	TwAddVarRW(bar, "DoFAutofocus", TW_TYPE_BOOLCPP, &m_DoFAutofocus,
		" label='Autofocus' group=Bokeh");
	TwAddVarRW(bar, "DoFFocalDistance", TW_TYPE_FLOAT, &m_FocalDistance,
		" label='Focal Distance' min=0 step=0.5 group=Bokeh");
	TwAddVarRW(bar, "DoFVignetting", TW_TYPE_BOOLCPP, &m_DoFVignetting,
		" label='Vignetting' group=Bokeh");
	TwAddVarRW(bar, "DoFMaxBlur", TW_TYPE_FLOAT, &m_DoFMaxBlur,
		" label='Max Blur Amount' min=0 step=0.5 group=Bokeh");

	TwAddVarRW(bar, "DoFShowFocus", TW_TYPE_BOOLCPP, &m_DoFShowFocus,
		" label='Show focus' group=Bokeh");

	TwDefine(" Parameters/Bokeh group=Postprocessing");

	//Tonemapping
	TwEnumVal tonemappingEV[] = { { (int)PhysiCam::TonemappingMethod::Reinhard, "Reinhard" },{ (int)PhysiCam::TonemappingMethod::Filmic , "Filmic" },
							{ (int)PhysiCam::TonemappingMethod::Uncharted2, "Uncharted2" }};
	TwType toneMappingType;
	// Defining season enum type
	toneMappingType = TwDefineEnum("Tonemapping Method", tonemappingEV, 3);
	// Adding methodss to bar

	TwAddVarRW(bar, "TonemappingEnabled", TW_TYPE_BOOLCPP, &m_ToneMappingEnabled,
		" label='Enabled' group=Tonemapping");

	TwAddVarRW(bar, "TonemappingMethod", toneMappingType, &m_TonemappingMethod, 
		" label='Method' group=Tonemapping");

	TwDefine(" Parameters/Tonemapping group=Postprocessing");  // group Tonemapping is moved into group Postprocessing

	auto size = m_Framework->ScreenSize();
	TwWindowSize(size.x, size.y);
}


#define SHRINK 1.0f

void TestGame::OnLoadContent()
{
	int scrW, scrH;
	m_Renderer->GetFramebufferSize(scrW, scrH);

	//create framebuffer needed for physicam (RGB32 + depth texture)
	m_FrameBuffer = Framebuffer::Create(scrW*SHRINK, scrH*SHRINK);

	m_ColorTexture = RenderTexture::Create(scrW*SHRINK, scrH*SHRINK, RenderTexture::TEXTURE_2D, Texture::RGB32F, false);
	m_DepthTexture = RenderTexture::Create(scrW*SHRINK, scrH*SHRINK, RenderTexture::TEXTURE_2D, Texture::DEPTH, false);

	m_FrameBuffer->BindTexture(m_ColorTexture, Framebuffer::COLOR0);
	m_FrameBuffer->BindTexture(m_DepthTexture, Framebuffer::DEPTH_ATTACHMENT);
	
	m_Model = Model::Load(ASSETS_FOLDER + "Models/dragon/dragon.fbx");
	m_Model->GetTransform()->Rotate(0, 180, 0);
	
	//create floor
	m_Floor = Mesh::CreatePlane();
	m_Floor->GetTransform()->Scale(500, 500, 500);
	m_Floor->GetTransform()->Rotate(-90, 0, 0);
	m_TexCheckerboard = Texture2D::Load(ASSETS_FOLDER + "textures/checkerboard.jpg");

	//create skydome
	m_Skydome = Model::Load(ASSETS_FOLDER + "Models/skydome/skydome.fbx");
	m_Skydome->GetTransform()->Scale(6, 6, 6);
	m_Skydome->GetTransform()->Rotate(0, 180, 0);
	m_SkyboxTexture = Texture2D::Load(ASSETS_FOLDER + "textures/Sky019_polar.hdr", true);


}

void TestGame::OnUpdate(double deltaTime)
{
	HandleInput(deltaTime);

	//get pointer to Physicam postprocessor
	auto pp = m_Camera->GetPostProcessor();

	//this is used to avoid calling TwSetParam every frame
	if (m_AutoModeLastValue != m_AutoMode)
	{
		m_AutoModeLastValue = m_AutoMode;
		TwSetParam(bar, "iso", "readonly", TW_PARAM_CSTRING, 1, m_AutoMode ? "true" : "false");
		TwSetParam(bar, "shutter Speed", "readonly", TW_PARAM_CSTRING, 1, m_AutoMode ? "true" : "false");
		TwSetParam(bar, "Aperture", "readonly", TW_PARAM_CSTRING, 1, m_AutoMode ? "true" : "false");
		m_Camera->UseAutoExposure(m_AutoMode);
	}
	if (m_AutoMode) //Autoexposure?
	{
		//get values for HUD
		m_Aperture = m_Camera->Aperture();
		m_ISO = m_Camera->Iso();
		m_ShutterSpeed = 1.0f / m_Camera->ShutterSpeed();
		TwRefreshBar(bar);
	}
	else
	{
		//set values from HUD to camera
		m_Camera->SetIso(m_ISO);
		m_Camera->SetShutterSpeed(1.0f / m_ShutterSpeed);
		m_Camera->SetAperture(m_Aperture);
	}
	m_Camera->SetFocalLength(m_FocalLength);
	m_Camera->SetSensorFromPreset(m_SensorType);

	//Set PhysiCam Postprocessor parameters to HUD values
	pp->SetLensDistortionAmount(m_LenseDistAmount);
	pp->SetBloomThreshold(m_BloomThreshold);
	pp->SetBloomIntensity(m_BloomStrength);
	pp->SetBloomEnabled(m_BloomEnabled);

	pp->SetDoFEnabled(m_DoFEnabled);
	pp->SetDoFAutofocus(m_DoFAutofocus);
	pp->SetDoFVignetting(m_DoFVignetting);
	pp->SetDoFShowFocus(m_DoFShowFocus);
	pp->SetDoFFocalDistance(m_FocalDistance);
	pp->SetDoFMaxBlur(m_DoFMaxBlur);
		

	pp->SetTonemappingEnabled(m_ToneMappingEnabled);
	pp->SetTonemappingMethod(m_TonemappingMethod);

	//update PhysiCam matrices
	m_Camera->Update(deltaTime);

	//update PhysiCam framebuffer information
	fboInpDesc.ColorTextureId = m_ColorTexture->GetTextureId();
	fboInpDesc.depthBufferId = m_DepthTexture->GetTextureId();
	fboInpDesc.FramebufferId = m_FrameBuffer->GetID();

}


void TestGame::HandleInput(double deltaTime)
{
	UpdateFreeLookCamera(deltaTime);

	if (Input::KeyDown(KEY_O))
	{
		wireframe = !wireframe;
	}
}


void TestGame::UpdateFreeLookCamera(double deltaTime)
{
	glm::vec3 camDelta;
	// calculate movement direction

	if (Input::MouseButtonHeld(MOUSE_BUTTON_2))
	{
		Input::GrabMouse(true);
		glm::vec2 dRot = Input::MouseDelta();
		camRot.y += ((float)(dRot.x * 0.1f));
		camRot.x += ((float)(dRot.y * 0.1f));
		if (camRot.x < -90.0f) {
			camRot.x = -90.0f;
		}
		if (camRot.x > 90.0f) {
			camRot.x = 90.0f;
		}
	}
	else
		Input::GrabMouse(false);
	glm::mat4 mat = glm::translate(0.0f, 0.0f, -0.3f);
	mat *= glm::rotate(camRot.x, 1.0f, 0.0f, 0.0f);
	mat *= glm::rotate(camRot.y, 0.0f, 1.0f, 0.0f);


	float xa = 0.0F;
	float ya = 0.0F;
	float za = 0.0F;
	
	if ((Input::KeyHeld(KEY_W)) || (Input::KeyHeld(KEY_UP))) {
		za -= 1.0F;
	}
	if ((Input::KeyHeld(KEY_S)) || (Input::KeyHeld(KEY_DOWN))) {
		za += 1.0F;
	}
	if ((Input::KeyHeld(KEY_A)) || (Input::KeyHeld(KEY_LEFT))) {
		xa -= 1.0F;
	}
	if ((Input::KeyHeld(KEY_D)) || (Input::KeyHeld(KEY_RIGHT))) {
		xa += 1.0F;
	}
	if (Input::KeyHeld(KEY_E)) {
		ya += 1.0F;
	}
	if (Input::KeyHeld(KEY_Q)) {
		ya -= 1.0F;
	}

	float dist = xa * xa + za * za + ya * ya;
	if (dist > 0.01F) {

		dist = 10.0f / (float)sqrt(dist);
		xa *= dist;
		za *= dist;
		ya *= dist;

		float sin = (float)sinf(camRot.y * _PI / 180.0f);
		float cos = (float)cosf(camRot.y * _PI / 180.0f);

		camDelta.x += xa * cos - za * sin;
		camDelta.y += ya;
		camDelta.z += za * cos + xa * sin;
	}
	camPos += camDelta * (float)deltaTime;
	mat *= glm::translate(-camPos);
	m_Camera->GetTransform()->SetModelMatrix(mat);
}


void TestGame::OnLoadRender()
{
	//Render loading message
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto scrSize = m_Framework->ScreenSize();
	m_Font->PrintText("Loading...", glm::vec2(scrSize.x - 155.0f, 30.f));
	m_Font->Render();
}

void TestGame::OnRender()
{
	//(Re)enable depth test
	glEnable(GL_DEPTH_TEST);
	
	//bind framebuffer before clear call
	m_FrameBuffer->Bind();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_Renderer->SetWireframeMode(wireframe);

	//Render skybox
	m_SkyboxTexture->Bind(0);
	m_SkydomeShader->Use();
	m_SkydomeShader->SetParameterf("brightness", 100.0f); //average sky brightness at bright sun is 100 lumens
	m_Skydome->Render(m_SkydomeShader);

	//Render dragon
	m_MaterialShader->Use();
	m_MaterialShader->SetParameterVec2("UVTile", glm::vec2(1,1));
	m_MaterialShader->SetParameterVec4("lightPos", m_Camera->GetViewMatrix()*m_LightPos);
	m_MaterialShader->SetParameterVec3("camPos", m_Camera->GetTransform()->WorldPosition());
	m_MaterialShader->SetParameterf("lightIntensityInLux", lightIntensity);
	m_MaterialShader->SetParameterf("roughness", 0.2f);
	m_MaterialShader->SetParameterf("reflectance", 0.8f);
	m_Model->Render(m_MaterialShader);

	//Render floor
	m_TexCheckerboard->Bind(0);
	m_MaterialShader->SetParameterVec2("UVTile", glm::vec2(50.0f, 50.0f));
	m_MaterialShader->SetParameterf("roughness", 0.8f);
	m_MaterialShader->SetParameterf("reflectance", 0.25f);
	m_MaterialShader->SetParameteri("Texture", 0);
	m_Floor->Render(m_MaterialShader);

	m_Renderer->SetWireframeMode(false);

	//disable depth test since post processing doesnt need it
	glDisable(GL_DEPTH_TEST);


	//output framebuffer id 0 so it will be rendered to the window back buffer
	m_Camera->RenderPostProcessing(fboInpDesc, 0);

	// Draw tweak bars
	TwDraw();
}

void TestGame::OnShutdown()
{
// 	DelPtr(m_ColorTexture);
// 	DelPtr(m_DepthTexture);
// 	DelPtr(m_FrameBuffer);
}

void TestGame::Exit()
{
	m_Framework->Exit();
}


int main(int argc, char** argv)
{
	{
		//we need the framework itself
		NoWork framework;

		//and a game class object
		TestGame game;

		//Lets try to get opengl version 4.2
		framework.SetOpenGLVersion(4, 2);

		//now create a window (with debug context)
		if (!framework.CreateNewWindow("PhysiCam Test", 1280, 720, 40, 40, Window::Flags::WINDOW_SHOWED | Window::Flags::WINDOW_OPENGL_DEBUG_CONTEXT))
			return -1;

		//now give the framework our game class object
		framework.RegisterGame(&game);

		//run the game
		framework.Run();

		//since the destructor is deleting everything for us we are finished here
		return 0;
	}
}


