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
 *	@file arch.h
 */

#pragma once

#if defined(_WIN32) || defined(_WIN64) //Microsoft Windows

	#define PHYSICAM_ENV_WIN 1

	#if defined(_M_X64)
		#define PHYSICAM_ARCH_AMD64 1
		#define PHYSICAM_CPU_64 1
	#elif defined(_M_IX86)
		#define PHYSICAM_ARCH_IX86 1
		#define PHYSICAM_CPU_32 1
	#elif defined(_M_ARM)
		#define PHYSICAM_ARCH_ARM 1
		#define PHYSICAM_CPU_32 1
	#else
		#error Unsupported architecture!
	#endif

	#if defined(_MSC_VER)
	#define PHYSICAM_COMPILER_MSC 1
	#elif defined(__GNUC__)
	#define PHYSICAM_COMPILER_GNU 1
	#elif defined(__ICL)
	#define PHYSICAM_COMPILER_INTEL 1
	#else
	#error Unsupported compiler!
	#endif

#elif defined(__APPLE__) || defined(__MACH__) //Apple MacOS/X

	#include "TargetConditionals.h"

	#define PHYSICAM_ENV_MAC 1

	#if defined(__i386__)
		#define PHYSICAM_ARCH_IX86 1
		#define PHYSICAM_CPU_32 1
	#elif defined(__x86_64__) || defined(__x86_64)
		#define PHYSICAM_ARCH_AMD64 1
		#define PHYSICAM_CPU_64 1
	#elif defined(__arm__)
		#define PHYSICAM_ARCH_ARM 1
		#define PHYSICAM_CPU_32 1
	#else
		#error Unsupported architecture!
	#endif

	#if defined(__GNUC__)
		#define PHYSICAM_COMPILER_GNU 1
	#endif

	#if defined(__clang__)
		#define PHYSICAM_COMPILER_CLANG 1
	#endif

	#if !defined(PHYSICAM_COMPILER_GNU) && !defined(PHYSICAM_COMPILER_CLANG)
		#error Unsupported compiler!
	#endif

#elif defined(__linux__) || defined(__CYGWIN__) //Linux

	#define PHYSICAM_ENV_LINUX 1

	#if defined(__i386__)
		#define PHYSICAM_ARCH_IX86 1
		#define PHYSICAM_CPU_32 1
	#elif defined(__x86_64__) || defined(__x86_64)
		#define PHYSICAM_ARCH_AMD64 1
		#define PHYSICAM_CPU_64 1
	#elif defined(__arm__)
		#define PHYSICAM_ARCH_ARM 1
		#define PHYSICAM_CPU_32 1
	#else
		#error Unsupported architecture!
	#endif

	#if defined(__GNUC__)
		#define PHYSICAM_COMPILER_GNU 1
	#else
		#error Unsupported compiler!
	#endif

#else
#error Unsupported platform!
#endif

//Compiler Specifics

#if defined(_PHYSICAM_SHARED_)
	#if defined(_PHYSICAM_INTERN_)
		#if defined(PHYSICAM_COMPILER_MSC) || defined(PHYSICAM_COMPILER_INTEL)
			#define PHYSICAM_DLL __declspec(dllexport)
		#elif defined(PHYSICAM_COMPILER_GNU) && (__GNUC__ >= 4)
			#define PHYSICAM_DLL __attribute__((visibility("default")))
		#else
			#define PHYSICAM_DLL
		#endif
	#else
		#if defined(PHYSICAM_COMPILER_MSC) || defined(PHYSICAM_COMPILER_INTEL)
			#define PHYSICAM_DLL __declspec(dllimport)
		#elif defined(PHYSICAM_COMPILER_GNU) && (__GNUC__ >= 4)
			#define PHYSICAM_DLL __attribute__((visibility("default")))
		#else
		#define PHYSICAM_DLL
		#endif
	#endif
#else
	#define PHYSICAM_DLL
#endif

//deprecate unnecessary compiler warnings from msvc
#if defined(PHYSICAM_COMPILER_MSC)
	#pragma warning(disable : 4251)	//'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
#endif