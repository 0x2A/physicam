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
 *	@file transform.cpp
 */

#include <physicam/transform.h>

namespace PhysiCam
{


	Transform::Transform()
	{}

	void Transform::Translate(float x, float y, float z, bool world /*= false*/)
	{
		Translate(glm::vec3(x, y, z));
	}

	void Transform::Translate(const glm::vec3 &dir, bool world /*= false*/)
	{
		glm::mat4 transMatrix = glm::translate(-dir);
		if (world)
			m_Transform = m_Transform * transMatrix;
		else
			m_Transform = transMatrix * m_Transform;
	}

	void Transform::Transformate(const glm::mat4& mat, bool world /*= false*/)
	{
		if (world)
			m_Transform = m_Transform * mat;
		else
			m_Transform = mat * m_Transform;
	}

	void Transform::SetPosition(const glm::vec3 &pos)
	{

	}

	void Transform::SetYPosition(const float &pos)
	{

	}

	void Transform::Rotate(float x, float y, float z, bool world /*= false*/)
	{
		Rotate(glm::vec3(x, y, z));
	}

	void Transform::Rotate(const glm::vec3 &dir, bool world /*= false*/)
	{
		glm::mat4 rotMatrix = glm::rotate(-dir.x, glm::vec3(1, 0, 0));
		rotMatrix *= glm::rotate(-dir.y, glm::vec3(0, 1, 0));
		rotMatrix *= glm::rotate(-dir.z, glm::vec3(0, 0, 1));

		if (world)
			m_Transform = m_Transform * rotMatrix;
		else
			m_Transform = rotMatrix * m_Transform;
	}

	void Transform::Rotate(glm::quat &q, bool world /*= false*/)
	{
		glm::mat4 rotMatrix = glm::mat4_cast(q);
		if (world)
			m_Transform = rotMatrix * m_Transform;
		else
			m_Transform = m_Transform * rotMatrix;
	}

	void Transform::ResetRotation()
	{
		glm::mat4 rot(m_Transform[0][0], m_Transform[0][1], m_Transform[0][2], 0,
			m_Transform[1][0], m_Transform[1][1], m_Transform[1][2], 0.f,
			m_Transform[2][0], m_Transform[2][1], m_Transform[2][2], 0.f,
			0.f, 0.f, 0.f, 1.f);
		rot = glm::inverse(rot);

		m_Transform *= rot;
	}

	void Transform::Scale(const glm::vec3 &scale)
	{
		m_Scale *= glm::scale(scale);
	}

	void Transform::Scale(float x, float y, float z)
	{
		m_Scale *= glm::scale(glm::vec3(x, y, z));
	}

	void Transform::SetScale(float x, float y, float z)
	{
		m_Scale = glm::scale(x, y, z);
	}

	void Transform::SetScale(const glm::vec3 &scale)
	{
		m_Scale = glm::scale(scale);
	}

	glm::vec3 Transform::GetScale()
	{
		return glm::vec3(m_Scale * glm::vec4(1, 1, 1, 0));
	}

	void Transform::Reset()
	{
		m_Transform = glm::mat4();
	}

	glm::vec3 Transform::WorldPosition()
	{
		return glm::vec3(GetModelMatrix() * glm::vec4(0, 0, 0, 1));;
	}

	glm::vec3 Transform::GetRotation()
	{
		float thetaX, thetaY, thetaZ;
		//Decomposing rotation matrix, upper left part from transform matrix
		if (m_Transform[0][0] == 1.0f)
		{
			thetaY = atan2f(m_Transform[0][2], m_Transform[2][3]);
			thetaX = thetaZ = 0;
		}
		else if (m_Transform[0][0] == -1.0f)
		{
			thetaY = atan2f(m_Transform[0][2], m_Transform[2][3]);
			thetaX = thetaZ = 0;
		}
		else
		{
			thetaY = atan2(-m_Transform[2][0], m_Transform[0][0]);
			thetaZ = asin(m_Transform[1][0]);
			thetaX = atan2(-m_Transform[1][2], m_Transform[1][1]);
		}

		thetaX *= RadToDeg;
		thetaY *= RadToDeg;
		thetaZ *= RadToDeg;
		if (thetaX < 0.0f) thetaX += 360.0f;
		if (thetaY < 0.0f) thetaY += 360.0f;
		if (thetaZ < 0.0f) thetaZ += 360.0f;
		return glm::vec3(thetaX, thetaY, thetaZ);
	}

	void Transform::LookAt(glm::vec3 &targetPosition)
	{
		m_Transform = glm::lookAt(WorldPosition(), targetPosition, glm::vec3(0, 1, 0));
	}

	glm::vec3 Transform::Up()
	{
		glm::vec4 up = GetModelMatrix() * glm::vec4(0, 1, 0, 0);
		return glm::vec3(up);
	}

	glm::vec3 Transform::Right()
	{
		glm::vec4 right = GetModelMatrix() * glm::vec4(1, 0, 0, 0);
		return glm::vec3(right);
	}

	glm::vec3 Transform::Forward()
	{
		glm::vec4 forward = GetModelMatrix() * glm::vec4(0, 0, -1, 0);
		return glm::vec3(forward);
	}

	glm::mat4 Transform::operator*(glm::mat4 const& m)
	{
		return GetModelMatrix() * m;
	}

	glm::mat4 Transform::operator*(Transform &m)
	{
		return GetModelMatrix() * m.GetModelMatrix();
	}

	glm::mat4 Transform::GetModelMatrix()
	{
		return m_Transform * m_Scale;
	}

	void Transform::SetModelMatrix(glm::mat4& mat)
	{
		m_Transform = mat;
	}

}