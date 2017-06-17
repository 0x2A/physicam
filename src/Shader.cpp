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
 *	@file Shader.cpp
 */

#include <physicam/shader.h>
#include <fstream>
#include <streambuf>
#include <iostream>

#include <GL/glew.h>

namespace PhysiCam
{
	PhysiCam::Shader::~Shader()
	{
		glDetachShader(m_ShaderObject, m_VSObject);
		glDetachShader(m_ShaderObject, m_FSObject);

		glDeleteShader(m_FSObject);
		glDeleteShader(m_VSObject);
		glDeleteProgram(m_ShaderObject);
	}

	ShaderPtr PhysiCam::Shader::Create(const std::string& vs, const std::string& fs)
	{
		
		ShaderPtr shader = ShaderPtr(new Shader());
		shader->m_VSObject = glCreateShader(GL_VERTEX_SHADER);
		shader->m_FSObject = glCreateShader(GL_FRAGMENT_SHADER);

		GLchar const* filesVS[]{vs.c_str()};
		GLchar const* filesFS[]{fs.c_str()};

		glShaderSource(shader->m_VSObject, 1, filesVS, 0);
		glShaderSource(shader->m_FSObject, 1, filesFS, 0);

		glCompileShader(shader->m_VSObject);
		if (!ValidateShader(shader->m_VSObject))
		{
			shader.reset();
			return shader;
		}
		glCompileShader(shader->m_FSObject);
		if (!ValidateShader(shader->m_FSObject))
		{
			shader.reset();
			return shader;
		}
		shader->m_ShaderObject = glCreateProgram();
		glAttachShader(shader->m_ShaderObject, shader->m_FSObject);
		glAttachShader(shader->m_ShaderObject, shader->m_VSObject);


		glLinkProgram(shader->m_ShaderObject);
		if (!ValidateProgram(shader->m_ShaderObject))
		{
			shader.reset();
		}

		return shader;
	}

	ShaderPtr PhysiCam::Shader::Load(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	{
		std::cout << "Creating shader from file '" << vertexShaderPath << "' and '" << fragmentShaderPath << "'" << std::endl;

		std::ifstream t(vertexShaderPath, std::ios::in);
		if (!t.is_open())
		{
			std::cerr << "Unable to open file '" << vertexShaderPath << "': " << std::strerror(errno) << std::endl;
			return 0;
		}

		std::string vertexSrc((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());

		t.close();

		t = std::ifstream(fragmentShaderPath, std::ios::in);
		if (!t.is_open())
		{
			std::cerr << "Unable to open file '" << vertexShaderPath << "': " << std::strerror(errno) << std::endl;
			return 0;
		}
		std::string fragmentSrc((std::istreambuf_iterator<char>(t)),
			std::istreambuf_iterator<char>());

		t.close();

		return Create(vertexSrc, fragmentSrc);
	}

	void PhysiCam::Shader::Bind()
	{
		glUseProgram(m_ShaderObject);
	}

	void PhysiCam::Shader::Reload()
	{

	}

	void PhysiCam::Shader::SetParameterf(std::string name, float val)
	{
		glUniform1f(GetAttributeLocation(name), val);
	}

	void PhysiCam::Shader::SetParameteri(std::string name, int val)
	{
		glUniform1i(GetAttributeLocation(name), val);
	}

	void Shader::SetParameterfv(std::string name, int count, float* val)
	{
		glUniform1fv(GetAttributeLocation(name), count, val);
	}

	void Shader::SetParameteriv(std::string name, int count, int *val)
	{
		glUniform1iv(GetAttributeLocation(name), count, val);
	}

	void PhysiCam::Shader::SetParameterVec2(std::string name,glm::vec2 val)
	{
		glUniform2f(GetAttributeLocation(name), val.x, val.y);
	}

	void PhysiCam::Shader::SetParameterVec3(std::string name, glm::vec3 val)
	{
		glUniform3f(GetAttributeLocation(name), val.x, val.y, val.z);
	}

	void PhysiCam::Shader::SetParameterVec4(std::string name, glm::vec4 val)
	{
		glUniform4f(GetAttributeLocation(name), val.x, val.y, val.z, val.w);
	}

	void PhysiCam::Shader::SetParameterMat3(std::string name, glm::mat3 val)
	{
		glUniformMatrix3fv(GetAttributeLocation(name), 1, GL_FALSE, glm::value_ptr(val));
	}

	void PhysiCam::Shader::SetParameterMat4(std::string name, glm::mat4 val)
	{
		glUniformMatrix4fv(GetAttributeLocation(name), 1, GL_FALSE, glm::value_ptr(val));
	}

	void PhysiCam::Shader::BindAttributeLocation(unsigned int id, const std::string &name)
	{
		glBindAttribLocation(m_ShaderObject, id, name.c_str());
	}

	int PhysiCam::Shader::GetAttributeLocation(const std::string& name)
	{
		auto res = m_ParamLocations.find(name);
		if (res != m_ParamLocations.end()) //key already requested?
			return res->second;

		//not requested yet
		GLint loc = glGetUniformLocation(m_ShaderObject, name.c_str());
		m_ParamLocations[name] = loc;
		return loc;
	}


	PhysiCam::Shader::Shader()
	{}

	bool Shader::ValidateShader(unsigned int shader, const char* file /*= 0*/)
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
				std::cerr << "Shader " << shader << " (" << (file ? file : "") << ") compile error: " << buffer << std::endl;
				return false;
			}
		}
		return true;
	}

	bool Shader::ValidateProgram(unsigned int program)
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
			std::cerr << "Error validating shader " << program << std::endl;

			glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);
			if (length > 0)
			{
				std::cerr << "Program " << program << " link error: " << buffer << std::endl;
			}
			//return false;
		}


		return true;
	}

	void Shader::BindFragdataLocation(unsigned int colorId, const std::string &name)
	{
		glBindFragDataLocation(m_ShaderObject, colorId, name.c_str());
	}

}

