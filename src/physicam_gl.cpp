/*
	PhysiCam - Physically based camera
	Copyright (C) 2015 Frank K�hnke

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
 *	@file physicam_gl.cpp
 */

#include <physicam/physicam_gl.h>
#include <gl/glew.h>

namespace PhysiCam
{

	bool GL::HasInternalFormatQuery = false;
	bool GL::HasTextureStorage = false;
	bool GL::HasDirectStateAccess = false;
	bool GL::HasAnisotropicFiltering = false;


	void GL::ValidateExtensions()
	{
		HasDirectStateAccess = ExtensionAvailable("GL_EXT_direct_state_access");
		HasTextureStorage = ExtensionAvailable("GL_ARB_texture_storage");
		HasInternalFormatQuery = ExtensionAvailable("GL_ARB_internalformat_query2");;
		HasAnisotropicFiltering = ExtensionAvailable("GL_EXT_texture_filter_anisotropic");
	}

	bool GL::ExtensionAvailable(const std::string& name)
	{
		return (bool)glewIsSupported(name.c_str());
	}

}