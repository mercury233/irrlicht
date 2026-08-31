// Minimal libdecor 0.2 public ABI declarations used by the dynamic Wayland backend.
// Derived from libdecor.h (MIT license).
// Copyright (c) 2017-2018 Red Hat Inc.; Copyright (c) 2018 Jonas Adahl.
#ifndef IRR_C_LIBDECOR_DYNAMIC_H_INCLUDED
#define IRR_C_LIBDECOR_DYNAMIC_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>

struct wl_display;
struct wl_surface;
struct wl_seat;
struct wl_output;
struct xdg_surface;
struct xdg_toplevel;
struct libdecor;
struct libdecor_frame;
struct libdecor_configuration;
struct libdecor_state;

enum libdecor_error
{
	LIBDECOR_ERROR_COMPOSITOR_INCOMPATIBLE,
	LIBDECOR_ERROR_INVALID_FRAME_CONFIGURATION
};

enum libdecor_window_state
{
	LIBDECOR_WINDOW_STATE_NONE = 0,
	LIBDECOR_WINDOW_STATE_ACTIVE = 1 << 0,
	LIBDECOR_WINDOW_STATE_MAXIMIZED = 1 << 1,
	LIBDECOR_WINDOW_STATE_FULLSCREEN = 1 << 2,
	LIBDECOR_WINDOW_STATE_TILED_LEFT = 1 << 3,
	LIBDECOR_WINDOW_STATE_TILED_RIGHT = 1 << 4,
	LIBDECOR_WINDOW_STATE_TILED_TOP = 1 << 5,
	LIBDECOR_WINDOW_STATE_TILED_BOTTOM = 1 << 6,
	LIBDECOR_WINDOW_STATE_SUSPENDED = 1 << 7
};

enum libdecor_capabilities
{
	LIBDECOR_ACTION_MOVE = 1 << 0,
	LIBDECOR_ACTION_RESIZE = 1 << 1,
	LIBDECOR_ACTION_MINIMIZE = 1 << 2,
	LIBDECOR_ACTION_FULLSCREEN = 1 << 3,
	LIBDECOR_ACTION_CLOSE = 1 << 4
};

struct libdecor_interface
{
	void (*error)(struct libdecor*, enum libdecor_error, const char*);
	void (*reserved0)(void); void (*reserved1)(void); void (*reserved2)(void);
	void (*reserved3)(void); void (*reserved4)(void); void (*reserved5)(void);
	void (*reserved6)(void); void (*reserved7)(void); void (*reserved8)(void);
	void (*reserved9)(void);
};

struct libdecor_frame_interface
{
	void (*configure)(struct libdecor_frame*, struct libdecor_configuration*, void*);
	void (*close)(struct libdecor_frame*, void*);
	void (*commit)(struct libdecor_frame*, void*);
	void (*dismiss_popup)(struct libdecor_frame*, const char*, void*);
	void (*reserved0)(void); void (*reserved1)(void); void (*reserved2)(void);
	void (*reserved3)(void); void (*reserved4)(void); void (*reserved5)(void);
	void (*reserved6)(void); void (*reserved7)(void); void (*reserved8)(void);
	void (*reserved9)(void);
};

#ifdef __cplusplus
extern "C" {
#endif
struct libdecor* libdecor_new(struct wl_display*, struct libdecor_interface*);
void libdecor_unref(struct libdecor*);
int libdecor_dispatch(struct libdecor*, int);
struct libdecor_frame* libdecor_decorate(struct libdecor*, struct wl_surface*, struct libdecor_frame_interface*, void*);
void libdecor_frame_unref(struct libdecor_frame*);
void libdecor_frame_set_title(struct libdecor_frame*, const char*);
void libdecor_frame_set_app_id(struct libdecor_frame*, const char*);
void libdecor_frame_set_capabilities(struct libdecor_frame*, enum libdecor_capabilities);
void libdecor_frame_unset_capabilities(struct libdecor_frame*, enum libdecor_capabilities);
void libdecor_frame_set_min_content_size(struct libdecor_frame*, int, int);
void libdecor_frame_set_max_content_size(struct libdecor_frame*, int, int);
void libdecor_frame_commit(struct libdecor_frame*, struct libdecor_state*, struct libdecor_configuration*);
void libdecor_frame_set_minimized(struct libdecor_frame*);
void libdecor_frame_set_maximized(struct libdecor_frame*);
void libdecor_frame_unset_maximized(struct libdecor_frame*);
void libdecor_frame_set_fullscreen(struct libdecor_frame*, struct wl_output*);
void libdecor_frame_unset_fullscreen(struct libdecor_frame*);
void libdecor_frame_map(struct libdecor_frame*);
struct libdecor_state* libdecor_state_new(int, int);
void libdecor_state_free(struct libdecor_state*);
bool libdecor_configuration_get_content_size(struct libdecor_configuration*, struct libdecor_frame*, int*, int*);
bool libdecor_configuration_get_window_state(struct libdecor_configuration*, enum libdecor_window_state*);
#ifdef __cplusplus
}
#endif

#endif
