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
 *	@file Vertex.h
 */

#pragma once

#include <physicam/physicam_def.h>
#include <physicam/physicam_math.h>

#include <vector>

namespace PhysiCam
{
	class PHYSICAM_DLL Vertex
	{
	public:
		Vertex(glm::vec3 _position, glm::vec3 _normal, glm::vec2 _texCoord) :
			position(_position), normal(_normal), texCoord(_texCoord)
		{
			if (glm::length(normal) != 0)
				glm::normalize(normal);

			color = glm::vec4(1, 1, 1, 1);
		}

		Vertex(glm::vec3 _position, glm::vec3 _normal) : Vertex(_position, _normal, glm::vec2())
		{}

		Vertex(glm::vec3 _position) : Vertex(_position, glm::vec3())
		{}

		Vertex(glm::vec3 _position, glm::vec3 _normal, glm::vec2 _texCoord, glm::vec4 _color) :Vertex(_position, _normal, _texCoord)
		{
			color = _color;
		}

		Vertex() : position(0), normal(0), texCoord(0)
		{
			color = glm::vec4(1, 1, 1, 1);
		}

		glm::vec3 position; //Position of the vertex
		glm::vec3 normal; //Normal of the vertex
		glm::vec2 texCoord; //Texture coordinate of the vertex
		glm::vec4 color; //Color of the vertex
	private:
	};

	//list of vertices for mesh creation
	typedef std::vector<Vertex> VertexList;

}