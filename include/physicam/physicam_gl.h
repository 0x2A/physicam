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
 *	@file physicam_gl.h
 */

#pragma once

#include <string>

namespace PhysiCam
{
	class GL
	{
	public:

		static void ValidateExtensions();
		static bool ExtensionAvailable(const std::string& name);

		static bool HasDirectStateAccess;
		static bool HasTextureStorage;
		static bool HasInternalFormatQuery;
		static bool HasAnisotropicFiltering;
	};
}