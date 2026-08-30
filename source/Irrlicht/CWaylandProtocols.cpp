/* Generated protocol metadata, compatible with wayland-scanner 1.18.0.
 *
 * fractional-scale-v1:
 * Copyright (c) 2022 Kenny Levinsen
 *
 * viewporter:
 * Copyright (c) 2013-2016 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * text-input-v3:
 * Copyright (c) 2012, 2013 Intel Corporation
 * Copyright (c) 2015, 2016 Jan Arne Petersen
 * Copyright (c) 2017, 2018 Red Hat, Inc.
 * Copyright (c) 2018 Purism SPC
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. The copyright holders make no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * Sources:
 * wayland-protocols/staging/fractional-scale/fractional-scale-v1.xml
 * wayland-protocols/stable/viewporter/viewporter.xml
 * wayland-protocols/unstable/text-input/text-input-unstable-v3.xml
 */
#include "CWaylandProtocols.h"

#ifdef _IRR_COMPILE_WITH_WAYLAND_DEVICE_

static const wl_interface* fractional_scale_v1_types[] = {
	0,
	&wp_fractional_scale_v1_interface,
	&wl_surface_interface,
};

static const wl_message wp_fractional_scale_manager_v1_requests[] = {
	{ "destroy", "", fractional_scale_v1_types + 0 },
	{ "get_fractional_scale", "no", fractional_scale_v1_types + 1 },
};

const wl_interface wp_fractional_scale_manager_v1_interface = {
	"wp_fractional_scale_manager_v1", 1,
	2, wp_fractional_scale_manager_v1_requests,
	0, 0,
};

static const wl_message wp_fractional_scale_v1_requests[] = {
	{ "destroy", "", fractional_scale_v1_types + 0 },
};

static const wl_message wp_fractional_scale_v1_events[] = {
	{ "preferred_scale", "u", fractional_scale_v1_types + 0 },
};

const wl_interface wp_fractional_scale_v1_interface = {
	"wp_fractional_scale_v1", 1,
	1, wp_fractional_scale_v1_requests,
	1, wp_fractional_scale_v1_events,
};

static const wl_interface* viewporter_types[] = {
	0,
	0,
	0,
	0,
	&wp_viewport_interface,
	&wl_surface_interface,
};

static const wl_message wp_viewporter_requests[] = {
	{ "destroy", "", viewporter_types + 0 },
	{ "get_viewport", "no", viewporter_types + 4 },
};

const wl_interface wp_viewporter_interface = {
	"wp_viewporter", 1,
	2, wp_viewporter_requests,
	0, 0,
};

static const wl_message wp_viewport_requests[] = {
	{ "destroy", "", viewporter_types + 0 },
	{ "set_source", "ffff", viewporter_types + 0 },
	{ "set_destination", "ii", viewporter_types + 0 },
};

const wl_interface wp_viewport_interface = {
	"wp_viewport", 1,
	3, wp_viewport_requests,
	0, 0,
};

static const wl_interface* text_input_unstable_v3_types[] = {
	0,
	0,
	0,
	0,
	&wl_surface_interface,
	&wl_surface_interface,
	&zwp_text_input_v3_interface,
	&wl_seat_interface,
};

static const wl_message zwp_text_input_v3_requests[] = {
	{ "destroy", "", text_input_unstable_v3_types + 0 },
	{ "enable", "", text_input_unstable_v3_types + 0 },
	{ "disable", "", text_input_unstable_v3_types + 0 },
	{ "set_surrounding_text", "sii", text_input_unstable_v3_types + 0 },
	{ "set_text_change_cause", "u", text_input_unstable_v3_types + 0 },
	{ "set_content_type", "uu", text_input_unstable_v3_types + 0 },
	{ "set_cursor_rectangle", "iiii", text_input_unstable_v3_types + 0 },
	{ "commit", "", text_input_unstable_v3_types + 0 },
};

static const wl_message zwp_text_input_v3_events[] = {
	{ "enter", "o", text_input_unstable_v3_types + 4 },
	{ "leave", "o", text_input_unstable_v3_types + 5 },
	{ "preedit_string", "?sii", text_input_unstable_v3_types + 0 },
	{ "commit_string", "?s", text_input_unstable_v3_types + 0 },
	{ "delete_surrounding_text", "uu", text_input_unstable_v3_types + 0 },
	{ "done", "u", text_input_unstable_v3_types + 0 },
};

const wl_interface zwp_text_input_v3_interface = {
	"zwp_text_input_v3", 1,
	8, zwp_text_input_v3_requests,
	6, zwp_text_input_v3_events,
};

static const wl_message zwp_text_input_manager_v3_requests[] = {
	{ "destroy", "", text_input_unstable_v3_types + 0 },
	{ "get_text_input", "no", text_input_unstable_v3_types + 6 },
};

const wl_interface zwp_text_input_manager_v3_interface = {
	"zwp_text_input_manager_v3", 1,
	2, zwp_text_input_manager_v3_requests,
	0, 0,
};

#endif
