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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "glcommon.h"

sp<Surface> sf_client_init(int x, int y, int w, int h, float alpha)
{
    int fmt = PIXEL_FORMAT_RGBA_8888;

    android_client = new SurfaceComposerClient;
    if (android_client == NULL)
    {
        printf("failed to create client\n");
        return 0;
    }

    android_surface_control = android_client->createSurface(
                                  String8("test"), 0, w, h, fmt, 0x300);
    android_surface = android_surface_control->getSurface();

    if (android_surface == NULL)
    {
        printf("failed to create surface controller\n");
        return 0;
    }
    if (android_surface_control == NULL)
    {
        printf("failed to get surface\n");
        return 0;
    }

    android_client->openGlobalTransaction();
    android_surface_control->setPosition(x, y);
    android_surface_control->setLayer(INT_MAX);
    android_surface_control->setAlpha(alpha);
    android_client->closeGlobalTransaction();

    return android_surface;
}

//position x, position y, width of window, height of window
static void demo_render_loop(int x, int y, int w, int h)
{
    sp<Surface> surf = sf_client_init(x, y, w, h, 0.5f);

    hw_setup(surf, w, h);
    for(;;)
    {
        hw_render();
        hw_step();
    }
}

int main(int argc, char **argv)
{
    demo_render_loop(200, 200, 500, 500);

    /* these are static sp's so we have to set them to null to clean them up */
    android_client = NULL;
    android_surface = NULL;
    android_surface_control = NULL;
    return 0;
}
