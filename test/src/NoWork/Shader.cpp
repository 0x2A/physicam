/*
*  ===========================================================================
*	This file is part of CrystalEngine source code.
*   Copyright (C) 2013  Frank Köhnke
*
*   This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*  ===========================================================================
*/


#include "nowork/Shader.h"

#include "nowork/Mesh.h"
#include "nowork/Framework.h"
#include "NoWork/FileSystem.h"

#include <regex>

NOWORK_API ShaderPtr Shader::DefaultUnlit;
NOWORK_API ShaderPtr Shader::DefaultUnlitTextured;
NOWORK_API ShaderPtr Shader::DefaultBlinPhong;
NOWORK_API ShaderPtr Shader::ScreenAlignedTextured;
NOWORK_API ShaderPtr Shader::ScreenAligned;
NOWORK_API ShaderPtr Shader::BlitScreenShader;


static bool ValidateShader(GLuint shader, const char* file = 0)
{
	const unsigned int BUFFER_SIZE = 512;
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	GLsizei length = 0;
	GLint success;

	//Note 1: Changed Validation: Intel returns 'No Errors' as InfoLog when in debug context
	//so we cant simply check for InfoLog to verify the successfull compilation of the shader.
	//So we first have to check the compile status and then get the detailed message.
	//that way we cant missinterpret compile information as error message (thx intel...)
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success != GL_TRUE)
	{
		glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);
		if (length > 0) {
			LOG_ERROR("Shader " << shader << " (" << (file ? file : "") << ") compile error: " << buffer << std::endl);
			return false;
		}
	}
	return true;
}

bool ValidateProgram(unsigned int program)
{
	const unsigned int BUFFER_SIZE = 512;
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	GLsizei length = 0;

	GLint loc = glGetUniformLocation(program, "ShadowTexture");
	if (loc != -1)
	{
		glUseProgram(program);
		glUniform1i(loc, 7);
		glUseProgram(0);
	}

	glValidateProgram(program);
	GLint status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	if (status == GL_FALSE)
	{
		LOG_ERROR("Error validating shader " << program << std::endl);

		glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);
		if (length > 0)
		{
			LOG_ERROR("Program " << program << " link error: " << buffer << std::endl);
		}
		//return false;
	}


	return true;
}

NOWORK_API  ShaderPtr Shader::Create(const std::string& vs, const std::string& fs)
{
	ShaderPtr shader = ShaderPtr(new Shader);

	if (!NoWork::IsMainThread())
	{
		std::string* arr = new std::string[2];
		arr[0] = vs;
		arr[1] = fs;
		shader->AddToGLQueue(0,arr);
		return shader;
	}

	if (!shader->CompileShaders(vs, fs))
	{
		return nullptr;
	}

	return shader;
}



NOWORK_API  ShaderPtr Shader::Load(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	LOG_DEBUG("Creating shader from file '" << vertexShaderPath << "' and '" << fragmentShaderPath << "'");

	std::string vertexSrc = FileSystem::LoadTextFile(vertexShaderPath);
	vertexSrc = PreprocessIncludes(vertexSrc, FileSystem::GetFilename(vertexShaderPath), FileSystem::GetPath(vertexShaderPath));
	
	std::string fragmentSrc = FileSystem::LoadTextFile(fragmentShaderPath);
	fragmentSrc = PreprocessIncludes(fragmentSrc, FileSystem::GetFilename(fragmentShaderPath), FileSystem::GetPath(fragmentShaderPath));

	return Create(vertexSrc, fragmentSrc);
}



void Shader::Use()
{
	glUseProgram(m_ShaderObject);
}

Shader::~Shader()
{
	glDetachShader(m_ShaderObject, m_VSObject);

	glDeleteShader(m_FSObject);
	glDetachShader(m_ShaderObject, m_FSObject);
	glDeleteShader(m_VSObject);
	glDeleteProgram(m_ShaderObject);
}

void Shader::InitializeDefaultShaders()
{
	const std::string defaultUnlitVertSrc =
		"#version 130\n"
		"in vec3 vertexPosition;\n"
		"in vec3 vertexNormal;\n"
		"in vec2 vertexUV;\n"
		"in vec4 vertexColor;\n"
		"in vec3 vertexTangent;\n"
		"\n"
		"uniform mat4 MVPMatrix;\n"
		"uniform mat4 ModelViewMatrix;\n"
		"uniform mat4 ModelMatrix;\n"
		"\n"
		"out vec2 texCoord;\n"
		"out vec4 vertColor;\n"
		"\n"
		"void main( void )\n"
		"{\n"
		"    texCoord = vertexUV;\n"
		"    vertColor = vertexColor;\n"
		"    gl_Position = MVPMatrix * vec4(vertexPosition,1);\n"
		"}";

	const std::string defaultUnlitFragSrc =
		"#version 130\n"
		"uniform vec4 DiffuseColor;\n"
		"\n"
		"in vec2 texCoord;\n"
		"in vec4 vertColor;\n"
		"\n"
		"out vec4 colorOut;\n"
		"\n"
		"void main( void )\n"
		"{\n"
		"    colorOut = DiffuseColor * vertColor;\n"
		"}";

	const std::string defaultUnlitFragTexturedSrc =
		"#version 130\n"
		"uniform vec4 DiffuseColor;\n"
		"uniform sampler2D Texture;\n"
		"\n"
		"in vec2 texCoord;\n"
		"in vec4 vertColor;\n"
		"\n"
		"out vec4 colorOut;\n"
		"\n"
		"void main( void )\n"
		"{\n"
		"	 vec4 col = texture(Texture, texCoord);\n"
		"    colorOut = DiffuseColor * vertColor * col;\n"
		"}";

	const std::string screenAlignedVertSrc =
		"#version 130\n"
		"in vec3 vertexPosition;\n"
		"in vec3 vertexNormal;\n"
		"in vec2 vertexUV;\n"
		"in vec4 vertexColor;\n"
		"in vec3 vertexTangent;\n"
		"\n"
		"out vec2 texCoord;\n"
		"out vec4 vertColor;\n"
		"\n"
		"void main( void )\n"
		"{\n"
		"    texCoord = vertexUV;\n"
		"    vertColor = vertexColor;\n"
		"    gl_Position = vec4(vertexPosition.xy, 0, 1);\n"
		"}";

	const std::string BlitScreenShaderVertSrc = R"(

		#version 130
		in vec4 vertexPosition;
		in vec2 vertexUV;

		out vec2 texCoord;

		void main(void)
		{
			texCoord = vertexUV;
			gl_Position = vertexPosition;
		};
		)";
	const std::string BlitScreenShaderFragSrc = R"(

		#version 130
		uniform sampler2D texture0;

		in vec2 texCoord;
		out vec4 colorOut;

		void main(void)
		{
			colorOut = vec4(texture(texture0,texCoord).xyz, 1);
		};
		)";

	DefaultUnlit = Shader::Create(defaultUnlitVertSrc, defaultUnlitFragSrc);
	DefaultUnlitTextured = Shader::Create(defaultUnlitVertSrc, defaultUnlitFragTexturedSrc);
	DefaultBlinPhong = NULL; //TODO
	ScreenAligned = Shader::Create(screenAlignedVertSrc, defaultUnlitFragSrc);
	ScreenAlignedTextured = Shader::Create(screenAlignedVertSrc, defaultUnlitFragTexturedSrc);
	BlitScreenShader = Shader::Create(BlitScreenShaderVertSrc, BlitScreenShaderFragSrc);

}



/*******************************************************************************
Shader inline implementation:
*******************************************************************************/


 Shader::Shader()
{}

inline void Shader::SetParameterf(std::string name, float val)
{
	glUniform1f(GetAttributeLocation(name), val);
}

NOWORK_API void Shader::SetParameterfv(std::string name, int num, float *val)
{
	glUniform1fv(GetAttributeLocation(name), num, val);
}

inline void Shader::SetParameteri(std::string name, int val)
{
	glUniform1i(GetAttributeLocation(name), val);
}
inline void Shader::SetParameterVec2(std::string name, glm::vec2 val)
{
	glUniform2f(GetAttributeLocation(name), val.x, val.y);
}
inline void Shader::SetParameterVec3(std::string name, glm::vec3 val)
{
	glUniform3f(GetAttributeLocation(name), val.x, val.y, val.z);
}

inline void Shader::SetParameterVec4(std::string name, glm::vec4 val)
{
	glUniform4f(GetAttributeLocation(name), val.x, val.y, val.z, val.w);
}

NOWORK_API void Shader::SetParameterVec4v(std::string name, int num, glm::vec4 *val)
{
	glUniform4fv(GetAttributeLocation(name), num,glm::value_ptr(val[0]));
}

inline void Shader::SetParameterMat3(std::string name, glm::mat3 val)
{
	glUniformMatrix3fv(GetAttributeLocation(name), 1, GL_FALSE, glm::value_ptr(val));
}

inline void Shader::SetParameterMat4(std::string name, glm::mat4 val)
{
	glUniformMatrix4fv(GetAttributeLocation(name), 1, GL_FALSE, glm::value_ptr(val));
}

NOWORK_API void Shader::SetParameterMat4v(std::string name, int num, glm::mat4* val)
{	
	glUniformMatrix4fv(GetAttributeLocation(name), num, GL_FALSE, glm::value_ptr(val[0]));
}

inline void Shader::SetDiffuseColor(glm::vec4 val)
{
	SetParameterVec4("DiffuseColor", val);
}

inline void Shader::SetDiffuseColor(float r, float g, float b, float a /*= 1*/)
{
	SetDiffuseColor(glm::vec4(r, g, b, a));
}

inline void Shader::SetParameterTexture(std::string name, TexturePtr tex, uint32_t slot)
{
	if (!tex) return;
	tex->Bind(slot);
	
	glUniform1i(GetAttributeLocation(name), slot);

}

void Shader::SetParameterTexturev(std::string name, int num, TexturePtr *tex, uint32_t *slots)
{
	if (!tex) return;
	for (int i = 0; i < num; i++)
		tex[i]->Bind(slots[i]);

	glUniform1iv(GetAttributeLocation(name), num, (GLint*)slots);
}

void Shader::SetParameterTexturev(std::string name, int num, RenderTexturePtr *tex, uint32_t *slots)
{
	if (!tex) return;
	for (int i = 0; i < num; i++)
		tex[i]->Bind(slots[i]);

	glUniform1iv(GetAttributeLocation(name), num, (GLint*)slots);
}

void Shader::SetTexture(TexturePtr tex)
{
	SetParameterTexture("Texture", tex, 0);
}

NOWORK_API void Shader::BindAttributeLocation(unsigned int id, const std::string &name)
{
	glBindAttribLocation(m_ShaderObject, id, name.c_str());
}

int Shader::GetAttributeLocation(const std::string& name)
{
	auto res = m_ParamLocations.find(name);
	if (res != m_ParamLocations.end()) //key already requested?
		return res->second;

	//not requested yet
	GLint loc = glGetUniformLocation(m_ShaderObject, name.c_str());
	m_ParamLocations[name] = loc;
	return loc;
}

void Shader::DoAsyncWork(int mode, void *params)
{
	std::string* arr = static_cast<std::string*>(params);
	CompileShaders(arr[0], arr[1]);
}

bool Shader::CompileShaders(const std::string& vs, const std::string& fs)
{
	m_VSObject = glCreateShader(GL_VERTEX_SHADER);
	m_FSObject = glCreateShader(GL_FRAGMENT_SHADER);

	GLchar const* filesVS[]{vs.c_str()};
	GLchar const* filesFS[]{fs.c_str()};

	glShaderSource(m_VSObject, 1, filesVS, 0);
	glShaderSource(m_FSObject, 1, filesFS, 0);

	glCompileShader(m_VSObject);
	if (!ValidateShader(m_VSObject))
	{
		return false;
	}
	glCompileShader(m_FSObject);
	if (!ValidateShader(m_FSObject))
	{
		return false;
	}
	m_ShaderObject = glCreateProgram();
	glAttachShader(m_ShaderObject, m_FSObject);
	glAttachShader(m_ShaderObject, m_VSObject);

	BindAttributeLocation(0, "vertexPosition"); //bind input variables to their location ids
	BindAttributeLocation(1, "vertexNormal");   //this is important so the variable ids match 
	BindAttributeLocation(2, "vertexUV");       //the input ids from the mesh
	BindAttributeLocation(3, "vertexColor");    //normally the driver sets them in order of definition but we cant be sure

	glLinkProgram(m_ShaderObject);
	if (!ValidateProgram(m_ShaderObject))
	{
		return false;
	}

	glBindAttribLocation(m_ShaderObject, MODEL_VERTEX_LOCATION, "vertexPosition");
	glBindAttribLocation(m_ShaderObject, MODEL_NORMAL_LOCATION, "vertexNormal");
	glBindAttribLocation(m_ShaderObject, MODEL_TEXCOORD_LOCATION, "vertexUV");

	return true;
}

std::string Shader::PreprocessIncludes(const std::string &source, const std::string &filename, const std::string& path, int level /*= 0*/)
{
	if (level > 32)
	{
		LOG_ERROR("header inclusion depth limit reached, might be caused by cyclic header inclusion. Caused in " << path);
		return source;
	}
	static const std::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");
	std::stringstream input;
	std::stringstream output;
	input << source;

	size_t line_number = 1;
	std::smatch matches;

	std::string line;
	while (std::getline(input, line))
	{
		bool dontPrintLine = false;
		if (level == 0 && line_number == 1) { //dont print #line at beginning of shader code, so we dont get trouble with #version strings and stuff
			dontPrintLine = true;
		}
		if (std::regex_search(line, matches, re))
		{
			std::string include_file = matches[1];
			std::string include_string;

			try
			{
				include_string = FileSystem::LoadTextFile(path + "/" + include_file);
			}
			catch (std::exception &e)
			{
				LOG_ERROR(filename << "(" << line_number << ") : fatal error: cannot open include file " << include_file);
				return "";
			}
			output << PreprocessIncludes(include_string, include_file, path, level + 1) << std::endl;
		}
		else
		{
			if (!dontPrintLine) output << "#line " << line_number << " \"" << filename << "\"" << std::endl;
			output << line << std::endl;
		}
		++line_number;
	}
	return output.str();
}

