/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <utils/misc.h>

#include <gui/SurfaceComposerClient.h>
#include <ui/Region.h>
#include <ui/Rect.h>

using namespace android;

static sp<SurfaceComposerClient> android_client;
static sp<Surface> android_surface;
static sp<SurfaceControl> android_surface_control;

void hw_setup(sp<Surface> surf, int w, int h);
void hw_render();
void hw_step();
