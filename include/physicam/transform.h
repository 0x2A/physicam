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
 *	@file transform.h
 */

#pragma once

#include <physicam/physicam_def.h>
#include <physicam/physicam_math.h>

#include <vector>

namespace PhysiCam
{
	class PHYSICAM_DLL Transform
	{
	public:

		Transform();
		virtual void Translate(float x, float y, float z, bool world = false);
		virtual void Translate(const glm::vec3 &dir, bool world = false);
		virtual void Transformate(const glm::mat4& mat, bool world = false);
		virtual void SetPosition(const glm::vec3 &pos);
		virtual void SetYPosition(const float &pos);
		virtual void Rotate(float x, float y, float z, bool world = false);
		virtual void Rotate(const glm::vec3 &dir, bool world = false);
		virtual void Rotate(glm::quat &q, bool world = false);
		virtual void ResetRotation();
		virtual void Scale(const glm::vec3 &scale);
		virtual void Scale(float x, float y, float z);
		virtual void SetScale(float x, float y, float z);
		virtual void SetScale(const glm::vec3 &scale);
		virtual glm::vec3 GetScale();

		virtual void Reset();

		virtual glm::vec3 WorldPosition();
		glm::vec3 GetRotation();
		void LookAt(glm::vec3 &targetPosition);

		virtual glm::vec3 Up();
		virtual glm::vec3 Right();
		virtual glm::vec3 Forward();

		glm::mat4 operator*(glm::mat4 const&);
		glm::mat4 operator*(Transform &);

		virtual glm::mat4 GetModelMatrix();

		//overrides the model matrix (useful for custom matrix calculations)
		void SetModelMatrix(glm::mat4&);


	protected:

		glm::mat4 m_Transform = glm::mat4(), m_Scale;
		std::vector<Transform*> m_ChildTransforms;
	};
}