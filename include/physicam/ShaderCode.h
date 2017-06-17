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
 *	@file ShaderCode.h
 */

//for easier code writing

#pragma once

#include <string>





namespace PhysiCam
{
	extern const std::string ScreenAlignedVertSrc;
	extern const std::string BlitScreenSrc;
	extern const std::string LensDistortionSrc;
	extern const std::string DownsampleScreenSrc;
	extern const std::string BrightPassSrc;
	extern const std::string IncrGaussBlurSrc;
	extern const std::string BlurHorizontalSrc;
	extern const std::string BlurVerticalSrc;
	extern const std::string BloomComposeSrc;
	extern const std::string LenseFlareSrc;
	extern const std::string BloomLenseComposeSrc;
	extern const std::string ToneMapperSrc;
	extern const std::string DoFSrc;
}