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
 * Authored by: Michael Frey <michael.frey@canonical.com>
 */

#include <EGL/egl.h>
#include <dlfcn.h>
#include <stddef.h>

#include <input_stack_compatibility_layer.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void *android_dlopen(const char *filename, int flag);
extern void *android_dlsym(void *handle, const char *symbol);

    static void (*_android_input_stack_initialize)(AndroidEventListener* listener, InputStackConfiguration* config) = NULL;
static void (*_android_input_stack_loop_once)() = NULL;
static void (*_android_input_stack_start)() = NULL;
static void (*_android_input_stack_stop)() = NULL;
static void (*_android_input_stack_shutdown)() = NULL;

static void *_libis = NULL;

void _init_androidis()
{
    _libis = (void *) android_dlopen("/system/lib/libis_compat_layer.so", RTLD_LAZY);
}

#define IS_DLSYM(fptr, sym) do { if (_libis == NULL) { _init_androidis(); }; if (*(fptr) == NULL) { *(fptr) = (void *) android_dlsym(_libis, sym); } } while (0) 

static void android_input_stack_initialize(AndroidEventListener* listener, InputStackConfiguration* config )
{
    IS_DLSYM(&_android_input_stack_initialize, "android_input_stack_initialize");
    _android_input_stack_initialize(listener, config);
}

static void android_input_stack_loop_once()
{
    IS_DLSYM(&_android_input_stack_loop_once, "android_input_stack_loop_once");
    _android_input_stack_loop_once();
}

static void android_input_stack_start()
{
    IS_DLSYM(&_android_input_stack_start, "android_input_stack_start");
    _android_input_stack_start();
}

static void android_input_stack_stop()
{
    IS_DLSYM(&_android_input_stack_stop, "android_input_stack_stop");
    _android_input_stack_stop();
}

static void android_input_stack_shutdown()
{
    IS_DLSYM(&_android_input_stack_shutdown, "android_input_stack_shutdown");
    _android_input_stack_shutdown();
}

#ifdef __cplusplus
}
#endif
