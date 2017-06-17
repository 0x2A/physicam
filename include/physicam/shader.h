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
 *	@file shader.h
 */


#pragma once

#include <physicam/physicam_def.h>
#include <physicam/physicam_math.h>

#include <string>
#include <map>
#include <memory>

namespace PhysiCam
{

	class Shader;
	typedef std::shared_ptr<Shader> ShaderPtr;
	class PHYSICAM_DLL Shader
	{
	public:

		~Shader();
		static ShaderPtr Create(const std::string& vs, const std::string& fs);
		static ShaderPtr Load(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

		void Bind();
		void Reload();

		void SetParameterf(std::string name, float val);
		void SetParameterfv(std::string name, int count, float* val);
		void SetParameteri(std::string name, int val);
		void SetParameteriv(std::string name, int count, int *val);
		void SetParameterVec2(std::string name,glm::vec2 val);
		void SetParameterVec3(std::string name, glm::vec3 val);
		void SetParameterVec4(std::string name, glm::vec4 val);
		void SetParameterMat3(std::string name, glm::mat3 val);
		void SetParameterMat4(std::string name, glm::mat4 val);
		//void SetParameterTexture(std::string name, Texture* tex, uint32_t slot);

		void BindAttributeLocation(unsigned int id, const std::string &name);
		int GetAttributeLocation(const std::string& name);

		void BindFragdataLocation(unsigned int colorId, const std::string &name);
		//void SetTexture(Texture* tex);


	protected:

		Shader();

		static bool ValidateShader(unsigned int shader, const char* file = 0);
		static bool ValidateProgram(unsigned int program);

	private:
		//shader source code
		std::string m_VertexSrc;
		std::string m_FragmentSrc;

		//opengl object ids
		unsigned int m_ShaderObject;
		unsigned int m_VSObject;
		unsigned int m_FSObject;
		

		//buffer for shader parameter locations
		std::map<std::string, int> m_ParamLocations;
	};
}