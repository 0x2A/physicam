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
 *	@file physicam_def.h
 */

#pragma once

//Check if cstdint/libc++ is available
#if __clang__ 
	#if ! __has_include( <cstdint> )
	#error "<cstdint> is missing - PhysiCam requires cstdint/libc++"
	#endif
#endif

#include <cstdint>

//version definitions
#define PHYSICAM_VERSION		100
#define PHYSICAM_VERSION_STR	"0.1.0"

#include <physicam/arch.h>

#define DelPtr(a) delete a;\
				  a = nullptr;