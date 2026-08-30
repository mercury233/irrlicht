// Wayland protocol declarations generated from wayland-protocols XML.
// The wrappers deliberately use wl_proxy_marshal_constructor (Wayland 1.18 ABI).
#ifndef IRR_C_WAYLAND_PROTOCOLS_H_INCLUDED
#define IRR_C_WAYLAND_PROTOCOLS_H_INCLUDED

#include "CWaylandLibrary.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

struct wp_fractional_scale_manager_v1;
struct wp_fractional_scale_v1;
struct wp_viewporter;
struct wp_viewport;
struct zwp_text_input_manager_v3;
struct zwp_text_input_v3;

extern const wl_interface wp_fractional_scale_manager_v1_interface;
extern const wl_interface wp_fractional_scale_v1_interface;
extern const wl_interface wp_viewporter_interface;
extern const wl_interface wp_viewport_interface;
extern const wl_interface zwp_text_input_manager_v3_interface;
extern const wl_interface zwp_text_input_v3_interface;

struct wp_fractional_scale_v1_listener
{
	void (*preferred_scale)(void*, wp_fractional_scale_v1*, uint32_t);
};

struct zwp_text_input_v3_listener
{
	void (*enter)(void*, zwp_text_input_v3*, wl_surface*);
	void (*leave)(void*, zwp_text_input_v3*, wl_surface*);
	void (*preedit_string)(void*, zwp_text_input_v3*, const char*, int32_t, int32_t);
	void (*commit_string)(void*, zwp_text_input_v3*, const char*);
	void (*delete_surrounding_text)(void*, zwp_text_input_v3*, uint32_t, uint32_t);
	void (*done)(void*, zwp_text_input_v3*, uint32_t);
};

namespace irr
{
	inline wp_fractional_scale_v1* waylandGetFractionalScale(CWaylandLibrary& lib, wp_fractional_scale_manager_v1* manager, wl_surface* surface)
	{
		return reinterpret_cast<wp_fractional_scale_v1*>(lib.ProxyMarshalConstructor(reinterpret_cast<wl_proxy*>(manager), 1, &wp_fractional_scale_v1_interface, 0, surface));
	}

	inline wp_viewport* waylandGetViewport(CWaylandLibrary& lib, wp_viewporter* viewporter, wl_surface* surface)
	{
		return reinterpret_cast<wp_viewport*>(lib.ProxyMarshalConstructor(reinterpret_cast<wl_proxy*>(viewporter), 1, &wp_viewport_interface, 0, surface));
	}

	inline void waylandViewportSetDestination(CWaylandLibrary& lib, wp_viewport* viewport, int32_t width, int32_t height)
	{
		lib.ProxyMarshal(reinterpret_cast<wl_proxy*>(viewport), 2, width, height);
	}

	inline zwp_text_input_v3* waylandGetTextInput(CWaylandLibrary& lib, zwp_text_input_manager_v3* manager, wl_seat* seat)
	{
		return reinterpret_cast<zwp_text_input_v3*>(lib.ProxyMarshalConstructor(reinterpret_cast<wl_proxy*>(manager), 1, &zwp_text_input_v3_interface, 0, seat));
	}

	inline void waylandTextInputEnable(CWaylandLibrary& lib, zwp_text_input_v3* input)
	{
		lib.ProxyMarshal(reinterpret_cast<wl_proxy*>(input), 1);
		lib.ProxyMarshal(reinterpret_cast<wl_proxy*>(input), 7);
	}

	inline void waylandTextInputDisable(CWaylandLibrary& lib, zwp_text_input_v3* input)
	{
		lib.ProxyMarshal(reinterpret_cast<wl_proxy*>(input), 2);
		lib.ProxyMarshal(reinterpret_cast<wl_proxy*>(input), 7);
	}

	inline void waylandTextInputSetCursorRectangle(CWaylandLibrary& lib, zwp_text_input_v3* input, int32_t x, int32_t y, int32_t w, int32_t h)
	{
		lib.ProxyMarshal(reinterpret_cast<wl_proxy*>(input), 6, x, y, w, h);
		lib.ProxyMarshal(reinterpret_cast<wl_proxy*>(input), 7);
	}
}

#endif
#endif
