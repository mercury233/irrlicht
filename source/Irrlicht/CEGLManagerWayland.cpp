#include "CEGLManagerWayland.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

#include "COpenGLExtensionHandler.h"

namespace irr
{
namespace video
{
	static CWaylandLibrary* ActiveWaylandLibrary = 0;

	static void* waylandGetProcAddress(const char* name)
	{
		if (!ActiveWaylandLibrary)
			return 0;
		return reinterpret_cast<void*>(ActiveWaylandLibrary->EglGetProcAddress(name));
	}

	CEGLManagerWayland::CEGLManagerWayland(CWaylandLibrary& library, wl_display* display, wl_surface* surface, wl_egl_window* window) :
		Library(library), WaylandDisplay(display), WaylandSurface(surface), NativeWindow(window),
		Display(EGL_NO_DISPLAY), Config(0), Surface(EGL_NO_SURFACE), Context(EGL_NO_CONTEXT)
	{
		Exposed.OpenGLWayland.WaylandDisplay = display;
		Exposed.OpenGLWayland.WaylandSurface = surface;
		Exposed.OpenGLWayland.EGLDisplay = 0;
		Exposed.OpenGLWayland.EGLContext = 0;
	}

	CEGLManagerWayland::~CEGLManagerWayland()
	{
		terminate();
	}

	bool CEGLManagerWayland::initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData&)
	{
		Params = params;
		Display = Library.EglGetDisplay(reinterpret_cast<EGLNativeDisplayType>(WaylandDisplay));
		if (Display == EGL_NO_DISPLAY)
			return false;
		EGLint major = 0, minor = 0;
		if (!Library.EglInitialize(Display, &major, &minor) || !Library.EglBindAPI(EGL_OPENGL_API))
			return false;

		EGLint attributes[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
			EGL_RED_SIZE, Params.Bits == 16 ? 5 : 8,
			EGL_GREEN_SIZE, Params.Bits == 16 ? 6 : 8,
			EGL_BLUE_SIZE, Params.Bits == 16 ? 5 : 8,
			EGL_ALPHA_SIZE, Params.WithAlphaChannel ? 8 : 0,
			EGL_DEPTH_SIZE, Params.ZBufferBits,
			EGL_STENCIL_SIZE, Params.Stencilbuffer ? 8 : 0,
			EGL_SAMPLE_BUFFERS, Params.AntiAlias ? 1 : 0,
			EGL_SAMPLES, Params.AntiAlias,
			EGL_NONE
		};
		EGLint count = 0;
		if (!Library.EglChooseConfig(Display, attributes, &Config, 1, &count) || count < 1)
		{
			attributes[17] = 0;
			attributes[19] = 0;
			if (!Library.EglChooseConfig(Display, attributes, &Config, 1, &count) || count < 1)
				return false;
		}
		Exposed.OpenGLWayland.EGLDisplay = Display;
		return true;
	}

	void CEGLManagerWayland::terminate()
	{
		destroyContext();
		destroySurface();
		if (Display != EGL_NO_DISPLAY)
		{
			Library.EglTerminate(Display);
			Display = EGL_NO_DISPLAY;
		}
		if (ActiveWaylandLibrary == &Library)
		{
			ActiveWaylandLibrary = 0;
			COpenGLExtensionHandler::setProcAddressLoader(0);
		}
	}

	bool CEGLManagerWayland::generateSurface()
	{
		if (Surface == EGL_NO_SURFACE)
			Surface = Library.EglCreateWindowSurface(Display, Config, reinterpret_cast<EGLNativeWindowType>(NativeWindow), 0);
		return Surface != EGL_NO_SURFACE;
	}

	void CEGLManagerWayland::destroySurface()
	{
		if (Surface != EGL_NO_SURFACE)
		{
			Library.EglDestroySurface(Display, Surface);
			Surface = EGL_NO_SURFACE;
		}
	}

	bool CEGLManagerWayland::generateContext()
	{
		if (Context == EGL_NO_CONTEXT)
			Context = Library.EglCreateContext(Display, Config, EGL_NO_CONTEXT, 0);
		if (Context == EGL_NO_CONTEXT || !Library.EglMakeCurrent(Display, Surface, Surface, Context))
			return false;
		Library.EglSwapInterval(Display, Params.Vsync ? 1 : 0);
		ActiveWaylandLibrary = &Library;
		COpenGLExtensionHandler::setProcAddressLoader(waylandGetProcAddress);
		Exposed.OpenGLWayland.EGLContext = Context;
		return true;
	}

	void CEGLManagerWayland::destroyContext()
	{
		if (Context != EGL_NO_CONTEXT)
		{
			Library.EglMakeCurrent(Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
			Library.EglDestroyContext(Display, Context);
			Context = EGL_NO_CONTEXT;
			Exposed.OpenGLWayland.EGLContext = 0;
		}
	}

	const SExposedVideoData& CEGLManagerWayland::getContext() const
	{
		return Exposed;
	}

	bool CEGLManagerWayland::activateContext(const SExposedVideoData& videoData, bool restorePrimaryOnZero)
	{
		if (!videoData.OpenGLWayland.EGLContext && !restorePrimaryOnZero)
			return Library.EglMakeCurrent(Display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) == EGL_TRUE;
		return Library.EglMakeCurrent(Display, Surface, Surface, Context) == EGL_TRUE;
	}

	bool CEGLManagerWayland::swapBuffers()
	{
		return Library.EglSwapBuffers(Display, Surface) == EGL_TRUE;
	}
}
}

#endif
