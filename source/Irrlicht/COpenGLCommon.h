// Copyright (C) 2015 Patryk Nadrowski
// Reduced a bit by Michael Zeilfelder for 1.8 backport
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef IRR_C_OPENGL_COMMON_H_INCLUDED
#define IRR_C_OPENGL_COMMON_H_INCLUDED

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_OPENGL_

#if defined(_IRR_WINDOWS_API_)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
		#define GL_GLEXT_LEGACY 1
	#endif
	#include <GL/gl.h>
	#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
		#include "glext.h"
	#endif
	#include "wglext.h"

	#ifdef _MSC_VER
		#pragma comment(lib, "OpenGL32.lib")
	#endif

#elif defined(_IRR_COMPILE_WITH_OSX_DEVICE_)
	#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
		#define GL_GLEXT_LEGACY 1
	#endif
	#include <OpenGL/gl.h>
	#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
		#include "glext.h"
	#endif
#elif defined(_IRR_COMPILE_WITH_SDL_DEVICE_) && !defined(_IRR_COMPILE_WITH_X11_DEVICE_)
	#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
		#define GL_GLEXT_LEGACY 1
		#define GLX_GLXEXT_LEGACY 1
	#else
		#define GL_GLEXT_PROTOTYPES 1
		#define GLX_GLXEXT_PROTOTYPES 1
	#endif
	#define NO_SDL_GLEXT
	#include <SDL/SDL_video.h>
	#include <SDL/SDL_opengl.h>
	#include "glext.h"
#else
	#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
		#define GL_GLEXT_LEGACY 1
		#define GLX_GLXEXT_LEGACY 1
	#else
		#define GL_GLEXT_PROTOTYPES 1
		#define GLX_GLXEXT_PROTOTYPES 1
	#endif
	#include <GL/gl.h>
	#include <GL/glx.h>
	#if defined(_IRR_OPENGL_USE_EXTPOINTER_)
	#include "glext.h"
	#undef GLX_ARB_get_proc_address
	#include "glxext.h"
	#endif
#endif

#ifndef GL_ARB_shader_objects
/* GL types for program/shader text and shader object handles */
typedef char GLcharARB;
typedef unsigned int GLhandleARB;
#endif

#ifndef GL_VERSION_2_0
/* GL type for program/shader text */
typedef char GLchar;
#endif


// To check if this header is in the current compile unit (different GL driver implementations use different "GLCommon" headers in Irrlicht)
#define IRR_COMPILE_GL_COMMON

#endif
#endif
