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
 *	@file Face.h
 */

#pragma once

#include <physicam/physicam_def.h>
#include <vector>

namespace PhysiCam
{
	class PHYSICAM_DLL Face
	{
	public:
		Face(unsigned int a, unsigned int b, unsigned int c)
		{
			Indices[0] = a;
			Indices[1] = b;
			Indices[2] = c;
		}
		unsigned int Indices[3];
	};

	//list of faces (indices) for mesh creation
	typedef std::vector<Face> FaceList;
}