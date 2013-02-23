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

#include <camera_compatibility_layer.h>
#include <camera_compatibility_layer_capabilities.h>

#include <assert.h>
#include <dlfcn.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void *android_dlopen(const char *filename, int flag);
extern void *android_dlsym(void *handle, const char *symbol);

#ifdef __cplusplus
}
#endif

namespace
{

struct CameraBridge
{
    static const char* path_to_library()
    {
        return "/system/lib/libcamera_compat_layer.so";
    }
    
    CameraBridge() : libcamera_handle(android_dlopen(path_to_library(), RTLD_LAZY))
    {
        assert(libcamera_handle && "Error loading camera library from");
    }

    ~CameraBridge()
    {
        // TODO android_dlclose(libcamera_handle);
    }

    void* resolve_symbol(const char* symbol) const
    {
        return android_dlsym(libcamera_handle, symbol);
    }
    
    void* libcamera_handle;
};

static CameraBridge camera_bridge;

}

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/
/*********** Implementation starts here *******************/
/**********************************************************/

#define CAMERA_DLSYM(fptr, sym) if (*(fptr) == NULL) { *(fptr) = (void *) camera_bridge.resolve_symbol(sym); }
    
#define IMPLEMENT_FUNCTION0(return_type, symbol)  \
    return_type symbol()                          \
    {                                             \
        static return_type (*f)() = NULL;         \
        CAMERA_DLSYM(&f, #symbol);                \
        return f();}
    
#define IMPLEMENT_FUNCTION1(return_type, symbol, arg1) \
    return_type symbol(arg1 _1)                        \
    {                                                  \
        static return_type (*f)(arg1) = NULL;          \
        CAMERA_DLSYM(&f, #symbol);                     \
        return f(_1); }

#define IMPLEMENT_VOID_FUNCTION1(symbol, arg1)               \
    void symbol(arg1 _1)                                     \
    {                                                        \
        static void (*f)(arg1) = NULL;                       \
        CAMERA_DLSYM(&f, #symbol);                           \
        f(_1); }

#define IMPLEMENT_FUNCTION2(return_type, symbol, arg1, arg2)    \
    return_type symbol(arg1 _1, arg2 _2)                        \
    {                                                           \
        static return_type (*f)(arg1, arg2) = NULL;             \
        CAMERA_DLSYM(&f, #symbol);                              \
        return f(_1, _2); }

#define IMPLEMENT_VOID_FUNCTION2(symbol, arg1, arg2)            \
    void symbol(arg1 _1, arg2 _2)                               \
    {                                                           \
        static void (*f)(arg1, arg2) = NULL;                    \
        CAMERA_DLSYM(&f, #symbol);                              \
        f(_1, _2); }

#define IMPLEMENT_VOID_FUNCTION3(symbol, arg1, arg2, arg3)      \
    void symbol(arg1 _1, arg2 _2, arg3 _3)                      \
    {                                                           \
        static void (*f)(arg1, arg2, arg3) = NULL;              \
        CAMERA_DLSYM(&f, #symbol);                              \
        f(_1, _2, _3); }


IMPLEMENT_FUNCTION0(int, android_camera_get_number_of_devices);
IMPLEMENT_FUNCTION2(CameraControl*, android_camera_connect_to, CameraType, CameraControlListener*);
IMPLEMENT_VOID_FUNCTION1(android_camera_disconnect, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_delete, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_dump_parameters, CameraControl*);

// Setters
IMPLEMENT_VOID_FUNCTION2(android_camera_set_effect_mode, CameraControl*, EffectMode);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_flash_mode, CameraControl*, FlashMode);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_white_balance_mode, CameraControl*, WhiteBalanceMode);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_scene_mode, CameraControl*, SceneMode);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_auto_focus_mode, CameraControl*, AutoFocusMode);
IMPLEMENT_VOID_FUNCTION3(android_camera_set_picture_size, CameraControl*, int, int);
IMPLEMENT_VOID_FUNCTION3(android_camera_set_preview_size, CameraControl*, int, int);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_display_orientation, CameraControl*, int32_t);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_preview_texture, CameraControl*, int);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_preview_surface, CameraControl*, SfSurface*);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_focus_region, CameraControl*, FocusRegion*);
IMPLEMENT_VOID_FUNCTION1(android_camera_reset_focus_region, CameraControl*);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_preview_fps, CameraControl*, int);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_rotation, CameraControl*, int);
// Getters
IMPLEMENT_VOID_FUNCTION2(android_camera_get_effect_mode, CameraControl*, EffectMode*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_flash_mode, CameraControl*, FlashMode*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_white_balance_mode, CameraControl*, WhiteBalanceMode*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_scene_mode, CameraControl*, SceneMode*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_auto_focus_mode, CameraControl*, AutoFocusMode*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_max_zoom, CameraControl*, int*);
IMPLEMENT_VOID_FUNCTION3(android_camera_get_picture_size, CameraControl*, int*, int*);
IMPLEMENT_VOID_FUNCTION3(android_camera_get_preview_size, CameraControl*, int*, int*);
IMPLEMENT_VOID_FUNCTION3(android_camera_get_preview_fps_range, CameraControl*, int*, int*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_preview_fps, CameraControl*, int*);
IMPLEMENT_VOID_FUNCTION2(android_camera_get_preview_texture_transformation, CameraControl*, float*);

// Enumerators
IMPLEMENT_VOID_FUNCTION3(android_camera_enumerate_supported_picture_sizes, CameraControl*, size_callback, void*);
IMPLEMENT_VOID_FUNCTION3(android_camera_enumerate_supported_preview_sizes, CameraControl*, size_callback, void*);

IMPLEMENT_VOID_FUNCTION1(android_camera_update_preview_texture, CameraControl*);

IMPLEMENT_VOID_FUNCTION1(android_camera_start_preview, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_stop_preview, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_start_autofocus, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_stop_autofocus, CameraControl*);

IMPLEMENT_VOID_FUNCTION2(android_camera_start_zoom, CameraControl*, int32_t);
IMPLEMENT_VOID_FUNCTION2(android_camera_set_zoom, CameraControl*, int32_t);
IMPLEMENT_VOID_FUNCTION1(android_camera_stop_zoom, CameraControl*);
IMPLEMENT_VOID_FUNCTION1(android_camera_take_snapshot, CameraControl*);

#ifdef __cplusplus
}
#endif
