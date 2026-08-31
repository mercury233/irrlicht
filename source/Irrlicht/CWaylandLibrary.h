#ifndef IRR_C_WAYLAND_LIBRARY_H_INCLUDED
#define IRR_C_WAYLAND_LIBRARY_H_INCLUDED

#include "IrrCompileConfig.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

#include "CLibdecorDynamic.h"
#include <EGL/egl.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>
#include <stdint.h>
#include <string>

namespace irr
{
	class CWaylandLibrary
	{
	public:
		CWaylandLibrary();
		~CWaylandLibrary();
		bool load();
		bool hasDecorationPlugin() const;
		const char* getError() const { return Error.empty() ? 0 : Error.c_str(); }

		wl_display* (*DisplayConnect)(const char*);
		void (*DisplayDisconnect)(wl_display*);
		int (*DisplayDispatch)(wl_display*);
		int (*DisplayDispatchPending)(wl_display*);
		int (*DisplayRoundtrip)(wl_display*);
		int (*DisplayFlush)(wl_display*);
		int (*DisplayGetFd)(wl_display*);
		int (*ProxyAddListener)(wl_proxy*, void (**)(void), void*);
		void (*ProxyDestroy)(wl_proxy*);
		void (*ProxyMarshal)(wl_proxy*, uint32_t, ...);
		wl_proxy* (*ProxyMarshalConstructor)(wl_proxy*, uint32_t, const wl_interface*, ...);
		wl_proxy* (*ProxyMarshalConstructorVersioned)(wl_proxy*, uint32_t, const wl_interface*, uint32_t, ...);
		uint32_t (*ProxyGetVersion)(wl_proxy*);

		const wl_interface* RegistryInterface;
		const wl_interface* CompositorInterface;
		const wl_interface* SurfaceInterface;
		const wl_interface* SeatInterface;
		const wl_interface* PointerInterface;
		const wl_interface* KeyboardInterface;
		const wl_interface* ShmInterface;
		const wl_interface* OutputInterface;
		const wl_interface* DataDeviceManagerInterface;
		const wl_interface* DataDeviceInterface;
		const wl_interface* DataSourceInterface;
		const wl_interface* DataOfferInterface;

		wl_egl_window* (*EglWindowCreate)(wl_surface*, int, int);
		void (*EglWindowDestroy)(wl_egl_window*);
		void (*EglWindowResize)(wl_egl_window*, int, int, int, int);

		wl_cursor_theme* (*CursorThemeLoad)(const char*, int, wl_shm*);
		void (*CursorThemeDestroy)(wl_cursor_theme*);
		wl_cursor* (*CursorThemeGetCursor)(wl_cursor_theme*, const char*);
		wl_buffer* (*CursorImageGetBuffer)(wl_cursor_image*);

		EGLDisplay (*EglGetDisplay)(EGLNativeDisplayType);
		EGLBoolean (*EglInitialize)(EGLDisplay, EGLint*, EGLint*);
		EGLBoolean (*EglTerminate)(EGLDisplay);
		EGLBoolean (*EglBindAPI)(EGLenum);
		EGLBoolean (*EglChooseConfig)(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*);
		EGLSurface (*EglCreateWindowSurface)(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint*);
		EGLBoolean (*EglDestroySurface)(EGLDisplay, EGLSurface);
		EGLContext (*EglCreateContext)(EGLDisplay, EGLConfig, EGLContext, const EGLint*);
		EGLBoolean (*EglDestroyContext)(EGLDisplay, EGLContext);
		EGLBoolean (*EglMakeCurrent)(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
		EGLBoolean (*EglSwapBuffers)(EGLDisplay, EGLSurface);
		EGLBoolean (*EglSwapInterval)(EGLDisplay, EGLint);
		__eglMustCastToProperFunctionPointerType (*EglGetProcAddress)(const char*);
		EGLint (*EglGetError)(void);

		libdecor* (*DecorNew)(wl_display*, libdecor_interface*);
		void (*DecorUnref)(libdecor*);
		int (*DecorDispatch)(libdecor*, int);
		libdecor_frame* (*DecorDecorate)(libdecor*, wl_surface*, libdecor_frame_interface*, void*);
		void (*DecorFrameUnref)(libdecor_frame*);
		void (*DecorFrameSetTitle)(libdecor_frame*, const char*);
		void (*DecorFrameSetAppId)(libdecor_frame*, const char*);
		void (*DecorFrameSetCapabilities)(libdecor_frame*, libdecor_capabilities);
		void (*DecorFrameUnsetCapabilities)(libdecor_frame*, libdecor_capabilities);
		void (*DecorFrameSetMinContentSize)(libdecor_frame*, int, int);
		void (*DecorFrameSetMaxContentSize)(libdecor_frame*, int, int);
		void (*DecorFrameCommit)(libdecor_frame*, libdecor_state*, libdecor_configuration*);
		void (*DecorFrameSetMinimized)(libdecor_frame*);
		void (*DecorFrameSetMaximized)(libdecor_frame*);
		void (*DecorFrameUnsetMaximized)(libdecor_frame*);
		void (*DecorFrameSetFullscreen)(libdecor_frame*, wl_output*);
		void (*DecorFrameUnsetFullscreen)(libdecor_frame*);
		void (*DecorFrameMap)(libdecor_frame*);
		libdecor_state* (*DecorStateNew)(int, int);
		void (*DecorStateFree)(libdecor_state*);
		bool (*DecorConfigurationGetContentSize)(libdecor_configuration*, libdecor_frame*, int*, int*);
		bool (*DecorConfigurationGetWindowState)(libdecor_configuration*, libdecor_window_state*);

		xkb_context* (*XkbContextNew)(xkb_context_flags);
		void (*XkbContextUnref)(xkb_context*);
		xkb_keymap* (*XkbKeymapNewFromString)(xkb_context*, const char*, xkb_keymap_format, xkb_keymap_compile_flags);
		void (*XkbKeymapUnref)(xkb_keymap*);
		int (*XkbKeymapKeyRepeats)(xkb_keymap*, xkb_keycode_t);
		xkb_state* (*XkbStateNew)(xkb_keymap*);
		void (*XkbStateUnref)(xkb_state*);
		xkb_keysym_t (*XkbStateKeyGetOneSym)(xkb_state*, xkb_keycode_t);
		int (*XkbStateKeyGetUtf8)(xkb_state*, xkb_keycode_t, char*, size_t);
		xkb_state_component (*XkbStateUpdateMask)(xkb_state*, xkb_mod_mask_t, xkb_mod_mask_t, xkb_mod_mask_t, xkb_layout_index_t, xkb_layout_index_t, xkb_layout_index_t);

	private:
		bool openLibrary(void*& handle, const char* name);
		bool loadSymbol(void* handle, void* target, const char* name);
		void unload();
		void* WaylandClient;
		void* WaylandEgl;
		void* WaylandCursor;
		void* Xkbcommon;
		void* Egl;
		void* Decor;
		std::string Error;
	};
}

#endif
#endif
