#ifndef IRR_C_EGL_MANAGER_WAYLAND_H_INCLUDED
#define IRR_C_EGL_MANAGER_WAYLAND_H_INCLUDED

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

#include "IContextManager.h"
#include "CWaylandLibrary.h"

namespace irr
{
namespace video
{
	class CEGLManagerWayland : public IContextManager
	{
	public:
		CEGLManagerWayland(CWaylandLibrary& library, wl_display* display, wl_surface* surface, wl_egl_window* window);
		virtual ~CEGLManagerWayland();
		virtual bool initialize(const SIrrlichtCreationParameters& params, const SExposedVideoData& data) IRR_OVERRIDE;
		virtual void terminate() IRR_OVERRIDE;
		virtual bool generateSurface() IRR_OVERRIDE;
		virtual void destroySurface() IRR_OVERRIDE;
		virtual bool generateContext() IRR_OVERRIDE;
		virtual void destroyContext() IRR_OVERRIDE;
		virtual const SExposedVideoData& getContext() const IRR_OVERRIDE;
		virtual bool activateContext(const SExposedVideoData& videoData, bool restorePrimaryOnZero) IRR_OVERRIDE;
		virtual bool swapBuffers() IRR_OVERRIDE;

	private:
		CWaylandLibrary& Library;
		wl_display* WaylandDisplay;
		wl_surface* WaylandSurface;
		wl_egl_window* NativeWindow;
		SIrrlichtCreationParameters Params;
		EGLDisplay Display;
		EGLConfig Config;
		EGLSurface Surface;
		EGLContext Context;
		SExposedVideoData Exposed;
	};
}
}

#endif
#endif
