#include "CWaylandLibrary.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

#include <dlfcn.h>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <string>

namespace irr
{
#define IRR_WL_LOAD(handle, member, symbol) if (!loadSymbol(handle, &member, symbol)) return false

	CWaylandLibrary::CWaylandLibrary() : Decor(0), Error(0)
	{
	}

	CWaylandLibrary::~CWaylandLibrary()
	{
#ifndef _IRR_COMPILE_WITH_WAYLAND_LINKED_DEVICE_
		if (Decor) dlclose(Decor);
#endif
	}

	bool CWaylandLibrary::openLibrary(void*& handle, const char* name)
	{
		handle = dlopen(name, RTLD_NOW | RTLD_LOCAL);
		if (!handle)
		{
			Error = dlerror();
			return false;
		}
		return true;
	}

	bool CWaylandLibrary::loadSymbol(void* handle, void* target, const char* name)
	{
		void* symbol = dlsym(handle, name);
		if (!symbol)
		{
			Error = dlerror();
			return false;
		}
		std::memcpy(target, &symbol, sizeof(symbol));
		return true;
	}

#define IRR_WL_DIRECT(member, symbol) member = &symbol
	bool CWaylandLibrary::load()
	{
		IRR_WL_DIRECT(DisplayConnect, wl_display_connect);
		IRR_WL_DIRECT(DisplayDisconnect, wl_display_disconnect);
		IRR_WL_DIRECT(DisplayDispatch, wl_display_dispatch);
		IRR_WL_DIRECT(DisplayDispatchPending, wl_display_dispatch_pending);
		IRR_WL_DIRECT(DisplayRoundtrip, wl_display_roundtrip);
		IRR_WL_DIRECT(DisplayFlush, wl_display_flush);
		IRR_WL_DIRECT(DisplayGetFd, wl_display_get_fd);
		IRR_WL_DIRECT(ProxyAddListener, wl_proxy_add_listener);
		IRR_WL_DIRECT(ProxyDestroy, wl_proxy_destroy);
		IRR_WL_DIRECT(ProxyMarshal, wl_proxy_marshal);
		IRR_WL_DIRECT(ProxyMarshalConstructor, wl_proxy_marshal_constructor);
		IRR_WL_DIRECT(ProxyMarshalConstructorVersioned, wl_proxy_marshal_constructor_versioned);
		IRR_WL_DIRECT(ProxyGetVersion, wl_proxy_get_version);
		RegistryInterface = &wl_registry_interface;
		CompositorInterface = &wl_compositor_interface;
		SurfaceInterface = &wl_surface_interface;
		SeatInterface = &wl_seat_interface;
		PointerInterface = &wl_pointer_interface;
		KeyboardInterface = &wl_keyboard_interface;
		ShmInterface = &wl_shm_interface;
		OutputInterface = &wl_output_interface;
		DataDeviceManagerInterface = &wl_data_device_manager_interface;
		DataDeviceInterface = &wl_data_device_interface;
		DataSourceInterface = &wl_data_source_interface;
		DataOfferInterface = &wl_data_offer_interface;
		IRR_WL_DIRECT(EglWindowCreate, wl_egl_window_create);
		IRR_WL_DIRECT(EglWindowDestroy, wl_egl_window_destroy);
		IRR_WL_DIRECT(EglWindowResize, wl_egl_window_resize);
		IRR_WL_DIRECT(CursorThemeLoad, wl_cursor_theme_load);
		IRR_WL_DIRECT(CursorThemeDestroy, wl_cursor_theme_destroy);
		IRR_WL_DIRECT(CursorThemeGetCursor, wl_cursor_theme_get_cursor);
		IRR_WL_DIRECT(CursorImageGetBuffer, wl_cursor_image_get_buffer);
		IRR_WL_DIRECT(EglGetDisplay, eglGetDisplay);
		IRR_WL_DIRECT(EglInitialize, eglInitialize);
		IRR_WL_DIRECT(EglTerminate, eglTerminate);
		IRR_WL_DIRECT(EglBindAPI, eglBindAPI);
		IRR_WL_DIRECT(EglChooseConfig, eglChooseConfig);
		IRR_WL_DIRECT(EglCreateWindowSurface, eglCreateWindowSurface);
		IRR_WL_DIRECT(EglDestroySurface, eglDestroySurface);
		IRR_WL_DIRECT(EglCreateContext, eglCreateContext);
		IRR_WL_DIRECT(EglDestroyContext, eglDestroyContext);
		IRR_WL_DIRECT(EglMakeCurrent, eglMakeCurrent);
		IRR_WL_DIRECT(EglSwapBuffers, eglSwapBuffers);
		IRR_WL_DIRECT(EglSwapInterval, eglSwapInterval);
		IRR_WL_DIRECT(EglGetProcAddress, eglGetProcAddress);
		IRR_WL_DIRECT(EglGetError, eglGetError);
		IRR_WL_DIRECT(XkbContextNew, xkb_context_new);
		IRR_WL_DIRECT(XkbContextUnref, xkb_context_unref);
		IRR_WL_DIRECT(XkbKeymapNewFromString, xkb_keymap_new_from_string);
		IRR_WL_DIRECT(XkbKeymapUnref, xkb_keymap_unref);
		IRR_WL_DIRECT(XkbKeymapKeyRepeats, xkb_keymap_key_repeats);
		IRR_WL_DIRECT(XkbStateNew, xkb_state_new);
		IRR_WL_DIRECT(XkbStateUnref, xkb_state_unref);
		IRR_WL_DIRECT(XkbStateKeyGetOneSym, xkb_state_key_get_one_sym);
		IRR_WL_DIRECT(XkbStateKeyGetUtf8, xkb_state_key_get_utf8);
		IRR_WL_DIRECT(XkbStateUpdateMask, xkb_state_update_mask);

#ifdef _IRR_COMPILE_WITH_WAYLAND_LINKED_DEVICE_
		IRR_WL_DIRECT(DecorNew, libdecor_new);
		IRR_WL_DIRECT(DecorUnref, libdecor_unref);
		IRR_WL_DIRECT(DecorDispatch, libdecor_dispatch);
		IRR_WL_DIRECT(DecorDecorate, libdecor_decorate);
		IRR_WL_DIRECT(DecorFrameUnref, libdecor_frame_unref);
		IRR_WL_DIRECT(DecorFrameSetTitle, libdecor_frame_set_title);
		IRR_WL_DIRECT(DecorFrameSetAppId, libdecor_frame_set_app_id);
		IRR_WL_DIRECT(DecorFrameSetCapabilities, libdecor_frame_set_capabilities);
		IRR_WL_DIRECT(DecorFrameUnsetCapabilities, libdecor_frame_unset_capabilities);
		IRR_WL_DIRECT(DecorFrameSetMinContentSize, libdecor_frame_set_min_content_size);
		IRR_WL_DIRECT(DecorFrameSetMaxContentSize, libdecor_frame_set_max_content_size);
		IRR_WL_DIRECT(DecorFrameCommit, libdecor_frame_commit);
		IRR_WL_DIRECT(DecorFrameSetMinimized, libdecor_frame_set_minimized);
		IRR_WL_DIRECT(DecorFrameSetMaximized, libdecor_frame_set_maximized);
		IRR_WL_DIRECT(DecorFrameUnsetMaximized, libdecor_frame_unset_maximized);
		IRR_WL_DIRECT(DecorFrameSetFullscreen, libdecor_frame_set_fullscreen);
		IRR_WL_DIRECT(DecorFrameUnsetFullscreen, libdecor_frame_unset_fullscreen);
		IRR_WL_DIRECT(DecorFrameMap, libdecor_frame_map);
		IRR_WL_DIRECT(DecorStateNew, libdecor_state_new);
		IRR_WL_DIRECT(DecorStateFree, libdecor_state_free);
		IRR_WL_DIRECT(DecorConfigurationGetContentSize, libdecor_configuration_get_content_size);
		IRR_WL_DIRECT(DecorConfigurationGetWindowState, libdecor_configuration_get_window_state);
		return true;
#else
		if (!openLibrary(Decor, "libdecor-0.so.0"))
			return false;

		IRR_WL_LOAD(Decor, DecorNew, "libdecor_new");
		IRR_WL_LOAD(Decor, DecorUnref, "libdecor_unref");
		IRR_WL_LOAD(Decor, DecorDispatch, "libdecor_dispatch");
		IRR_WL_LOAD(Decor, DecorDecorate, "libdecor_decorate");
		IRR_WL_LOAD(Decor, DecorFrameUnref, "libdecor_frame_unref");
		IRR_WL_LOAD(Decor, DecorFrameSetTitle, "libdecor_frame_set_title");
		IRR_WL_LOAD(Decor, DecorFrameSetAppId, "libdecor_frame_set_app_id");
		IRR_WL_LOAD(Decor, DecorFrameSetCapabilities, "libdecor_frame_set_capabilities");
		IRR_WL_LOAD(Decor, DecorFrameUnsetCapabilities, "libdecor_frame_unset_capabilities");
		IRR_WL_LOAD(Decor, DecorFrameSetMinContentSize, "libdecor_frame_set_min_content_size");
		IRR_WL_LOAD(Decor, DecorFrameSetMaxContentSize, "libdecor_frame_set_max_content_size");
		IRR_WL_LOAD(Decor, DecorFrameCommit, "libdecor_frame_commit");
		IRR_WL_LOAD(Decor, DecorFrameSetMinimized, "libdecor_frame_set_minimized");
		IRR_WL_LOAD(Decor, DecorFrameSetMaximized, "libdecor_frame_set_maximized");
		IRR_WL_LOAD(Decor, DecorFrameUnsetMaximized, "libdecor_frame_unset_maximized");
		IRR_WL_LOAD(Decor, DecorFrameSetFullscreen, "libdecor_frame_set_fullscreen");
		IRR_WL_LOAD(Decor, DecorFrameUnsetFullscreen, "libdecor_frame_unset_fullscreen");
		IRR_WL_LOAD(Decor, DecorFrameMap, "libdecor_frame_map");
		IRR_WL_LOAD(Decor, DecorStateNew, "libdecor_state_new");
		IRR_WL_LOAD(Decor, DecorStateFree, "libdecor_state_free");
		IRR_WL_LOAD(Decor, DecorConfigurationGetContentSize, "libdecor_configuration_get_content_size");
		IRR_WL_LOAD(Decor, DecorConfigurationGetWindowState, "libdecor_configuration_get_window_state");
		return true;
#endif
	}

	bool CWaylandLibrary::hasDecorationPlugin() const
	{
		std::string directories;
		const char* configured = std::getenv("LIBDECOR_PLUGIN_DIR");
		if (configured && *configured)
			directories = configured;
		else
		{
			Dl_info info = {};
			if (!dladdr(reinterpret_cast<const void*>(DecorNew), &info) || !info.dli_fname)
				return false;
			directories = info.dli_fname;
			const std::string::size_type slash = directories.find_last_of('/');
			if (slash == std::string::npos)
				return false;
			directories.erase(slash + 1);
			directories += "libdecor/plugins-1";
		}

		std::string::size_type start = 0;
		while (start <= directories.size())
		{
			const std::string::size_type end = directories.find(':', start);
			const std::string directory = directories.substr(start, end == std::string::npos ? end : end - start);
			DIR* handle = directory.empty() ? 0 : opendir(directory.c_str());
			if (handle)
			{
				dirent* entry = 0;
				while ((entry = readdir(handle)))
				{
					const std::string name(entry->d_name);
					if (name.size() > 3 && name.compare(0, 9, "libdecor-") == 0 &&
						name.compare(name.size() - 3, 3, ".so") == 0 && name.find("dummy") == std::string::npos)
					{
						closedir(handle);
						return true;
					}
				}
				closedir(handle);
			}
			if (end == std::string::npos)
				break;
			start = end + 1;
		}
		return false;
	}

#undef IRR_WL_DIRECT
#undef IRR_WL_LOAD
}

#endif
