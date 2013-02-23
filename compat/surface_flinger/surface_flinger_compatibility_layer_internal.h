/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#ifndef SURFACE_FLINGER_COMPATIBILITY_LAYER_INTERNAL_H_
#define SURFACE_FLINGER_COMPATIBILITY_LAYER_INTERNAL_H_

#ifndef __ANDROID__
#error "Using this header is only safe when compiling for Android."
#endif

#include <utils/misc.h>

#include <gui/SurfaceComposerClient.h>
#include <ui/PixelFormat.h>
#include <ui/Region.h>
#include <ui/Rect.h>

#include <cassert>
#include <cstdio>

struct SfClient
{
    android::sp<android::SurfaceComposerClient> client;
    EGLDisplay egl_display;
    EGLConfig egl_config;
    EGLContext egl_context;
    bool egl_support;
};

struct SfSurface
{
    SfSurface() : client(NULL)
    {
    }

    SfClient* client;
    android::sp<android::SurfaceControl> surface_control;
    android::sp<android::Surface> surface;
    EGLSurface egl_surface;
};

#endif // SURFACE_FLINGER_COMPATIBILITY_LAYER_INTERNAL_H_
