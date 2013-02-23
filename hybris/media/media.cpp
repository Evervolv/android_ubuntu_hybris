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
 * Authored by: Jim Hodapp <jim.hodapp@canonical.com>
 */

#include <media_compatibility_layer.h>

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

struct MediaPlayerBridge
{
    static const char* path_to_library()
    {
        return "/system/lib/libmedia_compat_layer.so";
    }

    MediaPlayerBridge() : libmediaplayer_handle(android_dlopen(path_to_library(), RTLD_LAZY))
    {
        assert(libmediaplayer_handle && "Error loading media player library from /system/lib/libmedia_compat_layer.so");
    }

    ~MediaPlayerBridge()
    {
        // TODO android_dlclose(libmediaplayer_handle);
    }

    void* resolve_symbol(const char* symbol) const
    {
        return android_dlsym(libmediaplayer_handle, symbol);
    }

    void* libmediaplayer_handle;
};

static MediaPlayerBridge media_player_bridge;

}

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/
/*********** Implementation starts here *******************/
/**********************************************************/

#define MEDIA_PLAYER_DLSYM(fptr, sym) if (*(fptr) == NULL) { *(fptr) = (void *) media_player_bridge.resolve_symbol(sym); }

#define IMPLEMENT_VOID_FUNCTION0(symbol)                        \
    void symbol()                                               \
    {                                                           \
        static void (*f)() = NULL;                              \
        MEDIA_PLAYER_DLSYM(&f, #symbol);                        \
        f(); }

#define IMPLEMENT_FUNCTION0(return_type, symbol)                \
    return_type symbol()                                        \
    {                                                           \
        static return_type (*f)() = NULL;                       \
        MEDIA_PLAYER_DLSYM(&f, #symbol);                        \
        return f();}

#define IMPLEMENT_FUNCTION1(return_type, symbol, arg1)          \
    return_type symbol(arg1 _1)                                 \
    {                                                           \
        static return_type (*f)(arg1) = NULL;                   \
        MEDIA_PLAYER_DLSYM(&f, #symbol);                        \
        return f(_1); }

#define IMPLEMENT_VOID_FUNCTION1(symbol, arg1)                  \
    void symbol(arg1 _1)                                        \
    {                                                           \
        static void (*f)(arg1) = NULL;                          \
        MEDIA_PLAYER_DLSYM(&f, #symbol);                        \
        f(_1); }

#define IMPLEMENT_FUNCTION2(return_type, symbol, arg1, arg2)    \
    return_type symbol(arg1 _1, arg2 _2)                        \
    {                                                           \
        static return_type (*f)(arg1, arg2) = NULL;             \
        MEDIA_PLAYER_DLSYM(&f, #symbol);                        \
        return f(_1, _2); }

#define IMPLEMENT_VOID_FUNCTION2(symbol, arg1, arg2)            \
    void symbol(arg1 _1, arg2 _2)                               \
    {                                                           \
        static void (*f)(arg1, arg2) = NULL;                    \
        MEDIA_PLAYER_DLSYM(&f, #symbol);                        \
        f(_1, _2); }

#define IMPLEMENT_VOID_FUNCTION3(symbol, arg1, arg2, arg3)      \
    void symbol(arg1 _1, arg2 _2, arg3 _3)                      \
    {                                                           \
        static void (*f)(arg1, arg2, arg3) = NULL;              \
        MEDIA_PLAYER_DLSYM(&f, #symbol);                        \
        f(_1, _2, _3); }


IMPLEMENT_FUNCTION0(MediaPlayerWrapper *, android_media_new_player);
IMPLEMENT_VOID_FUNCTION1(android_media_update_surface_texture, MediaPlayerWrapper *);
IMPLEMENT_FUNCTION1(int, android_media_play, MediaPlayerWrapper *);
IMPLEMENT_FUNCTION1(int, android_media_pause, MediaPlayerWrapper *);
IMPLEMENT_FUNCTION1(int, android_media_stop, MediaPlayerWrapper *);
IMPLEMENT_FUNCTION1(bool, android_media_is_playing, MediaPlayerWrapper *);
IMPLEMENT_FUNCTION2(int, android_media_seek_to, MediaPlayerWrapper *, int);

// Setters
IMPLEMENT_FUNCTION2(int, android_media_set_data_source, MediaPlayerWrapper *, const char*);
IMPLEMENT_FUNCTION2(int, android_media_set_preview_texture, MediaPlayerWrapper *, int);
IMPLEMENT_FUNCTION2(int, android_media_set_volume, MediaPlayerWrapper *, int);

// Getters
IMPLEMENT_VOID_FUNCTION2(android_media_surface_texture_get_transformation_matrix, MediaPlayerWrapper *, GLfloat*);
IMPLEMENT_FUNCTION2(int, android_media_get_current_position, MediaPlayerWrapper *, int*);
IMPLEMENT_FUNCTION2(int, android_media_get_duration, MediaPlayerWrapper *, int*);
IMPLEMENT_FUNCTION2(int, android_media_get_volume, MediaPlayerWrapper *, int*);

// Callbacks
IMPLEMENT_VOID_FUNCTION3(android_media_set_video_size_cb, MediaPlayerWrapper *, on_msg_set_video_size, void*);
IMPLEMENT_VOID_FUNCTION3(android_media_set_video_texture_needs_update_cb, MediaPlayerWrapper *, on_video_texture_needs_update, void*);
IMPLEMENT_VOID_FUNCTION3(android_media_set_error_cb, MediaPlayerWrapper *, on_msg_error, void*);
IMPLEMENT_VOID_FUNCTION3(android_media_set_playback_complete_cb, MediaPlayerWrapper *, on_playback_complete, void*);
IMPLEMENT_VOID_FUNCTION3(android_media_set_media_prepared_cb, MediaPlayerWrapper *, on_media_prepared, void*);

#ifdef __cplusplus
}
#endif
