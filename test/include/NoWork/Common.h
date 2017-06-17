#pragma once


#define NOWORK_VERSION	"1.6"

//Either export or import dll functions
#ifdef _NOWORK_INTERNAL_
#define NOWORK_API   __declspec( dllexport ) 
#else
#define NOWORK_API   __declspec( dllimport ) 
#endif

#ifdef _WIN32
#define SHARED(x) \
	__pragma(data_seg(".shared"))\
	x; \
	__pragma(data_seg())
#else
//TODO
#endif

#include <string>
#include <unordered_map>
#include <map>
#include <iostream>
#include <sstream>
#include <thread>
#include <memory>
#define _USE_MATH_DEFINES 
#include <math.h>

#include "GL/gl3w.h"
//only include glfw3 if we are in NoWork context
#ifdef _NOWORK_INTERNAL_
#include <glfw/glfw3.h>
#endif

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/rotate_vector.hpp"


#define GRADTORAD(x) (( x * M_PI ) / 180.0f)
#define RADTOGRAD(x) (( x * 180.0f ) / M_PI)

//using namespace glm;

template<class T>
inline void DelPtr(T* ptr)
{
	delete ptr;
	ptr = nullptr;
}

template<class T>
inline void DelArr(T* ptr)
{
	delete []ptr;
	ptr = nullptr;
}