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

// Uncomment to enable verbose debug output
#define LOG_NDEBUG 0

#undef LOG_TAG
#define LOG_TAG "MediaCompatibilityLayer"

#include "media_compatibility_layer.h"

#include <fcntl.h>
#include <sys/stat.h>

#include <media/mediaplayer.h>

#include <binder/ProcessState.h>
#include <gui/SurfaceTexture.h>

#include <utils/Log.h>

#define REPORT_FUNCTION() ALOGV("%s \n", __PRETTY_FUNCTION__);

struct FrameAvailableListener : public android::SurfaceTexture::FrameAvailableListener
{
public:
    FrameAvailableListener()
        : set_video_texture_needs_update_cb(NULL),
          video_texture_needs_update_context(NULL)
    {
    }

    // From android::SurfaceTexture::FrameAvailableListener
    void onFrameAvailable()
    {
        if (set_video_texture_needs_update_cb != NULL)
            set_video_texture_needs_update_cb(video_texture_needs_update_context);
    }

    void setVideoTextureNeedsUpdateCb(on_video_texture_needs_update cb, void *context)
    {
        set_video_texture_needs_update_cb = cb;
        video_texture_needs_update_context = context;
    }

private:
    on_video_texture_needs_update set_video_texture_needs_update_cb;
    void *video_texture_needs_update_context;
};

class MediaPlayerListenerWrapper : public android::MediaPlayerListener
{
public:
    MediaPlayerListenerWrapper()
        : set_video_size_cb(NULL),
          video_size_context(NULL),
          error_cb(NULL),
          error_context(NULL),
          playback_complete_cb(NULL),
          playback_complete_context(NULL),
          media_prepared_cb(NULL),
          media_prepared_context(NULL)
    {
    }

    void notify(int msg, int ext1, int ext2, const android::Parcel *obj)
    {
        ALOGV("\tmsg: %d, ext1: %d, ext2: %d \n", msg, ext1, ext2);

        switch(msg)
        {
            case android::MEDIA_PREPARED:
                ALOGV("\tMEDIA_PREPARED msg\n");
                if (media_prepared_cb != NULL)
                    media_prepared_cb(media_prepared_context);
                else
                    ALOGW("Failed to signal media prepared, callback not set.");

                break;
            case android::MEDIA_PLAYBACK_COMPLETE:
                ALOGV("\tMEDIA_PLAYBACK_COMPLETE msg\n");
                if (playback_complete_cb != NULL)
                    playback_complete_cb(playback_complete_context);
                else
                    ALOGW("Failed to signal end of playback, callback not set.");

                break;
            case android::MEDIA_BUFFERING_UPDATE:
                ALOGV("\tMEDIA_BUFFERING_UPDATE msg\n");
                break;
            case android::MEDIA_SEEK_COMPLETE:
                ALOGV("\tMEDIA_SEEK_COMPLETE msg\n");
                break;
            case android::MEDIA_SET_VIDEO_SIZE:
                ALOGV("\tMEDIA_SET_VIDEO_SIZE msg\n");
                if (set_video_size_cb != NULL)
                    set_video_size_cb(ext2, ext1, video_size_context);
                else
                    ALOGE("Failed to set video size. set_video_size_cb is NULL.");

                break;
            case android::MEDIA_TIMED_TEXT:
                ALOGV("\tMEDIA_TIMED_TEXT msg\n");
                break;
            case android::MEDIA_ERROR:
                ALOGV("\tMEDIA_ERROR msg\n");
                // TODO: Extend this cb to include the error message
                if (error_cb != NULL)
                    error_cb(error_context);
                else
                    ALOGE("Failed to signal error to app layer, callback not set.");

                break;
            case android::MEDIA_INFO:
                ALOGV("\tMEDIA_INFO msg\n");
                break;
            default:
                ALOGV("\tUnknown media msg\n");
        }
    }

    void setVideoSizeCb(on_msg_set_video_size cb, void *context)
    {
        REPORT_FUNCTION();

        set_video_size_cb = cb;
        video_size_context = context;
    }

    void setErrorCb(on_msg_error cb, void *context)
    {
        REPORT_FUNCTION();

        error_cb = cb;
        error_context = context;
    }

    void setPlaybackCompleteCb(on_playback_complete cb, void *context)
    {
        REPORT_FUNCTION();

        playback_complete_cb = cb;
        playback_complete_context = context;
    }

    void setMediaPreparedCb(on_media_prepared cb, void *context)
    {
        REPORT_FUNCTION();

        media_prepared_cb = cb;
        media_prepared_context = context;
    }

private:
    on_msg_set_video_size set_video_size_cb;
    void *video_size_context;
    on_msg_error error_cb;
    void *error_context;
    on_playback_complete playback_complete_cb;
    void *playback_complete_context;
    on_media_prepared media_prepared_cb;
    void *media_prepared_context;
};

// ----- MediaPlayer Wrapper ----- //

struct MediaPlayerWrapper : public android::MediaPlayer
{
public:
    MediaPlayerWrapper()
        : MediaPlayer(),
          texture(NULL),
          media_player_listener(new MediaPlayerListenerWrapper()),
          frame_listener(new FrameAvailableListener),
          left_volume(1), // Set vol to 100% for this track by default
          right_volume(1),
          source_fd(-1)
    {
        setListener(media_player_listener);
        // Update the live volume with the cached values
        MediaPlayer::setVolume(left_volume, right_volume);
    }

    ~MediaPlayerWrapper()
    {
        reset();
        source_fd = -1;
    }

    android::status_t setVideoSurfaceTexture(const android::sp<android::SurfaceTexture> &surfaceTexture)
    {
        REPORT_FUNCTION();

        surfaceTexture->getBufferQueue()->setBufferCount(5);
        texture = surfaceTexture;
        texture->setFrameAvailableListener(frame_listener);

        return MediaPlayer::setVideoSurfaceTexture(surfaceTexture->getBufferQueue());
    }

    void updateSurfaceTexture()
    {
        assert(texture != NULL);
        texture->updateTexImage();
    }

    void get_transformation_matrix_for_surface_texture(GLfloat* matrix)
    {
        assert(texture != NULL);
        texture->getTransformMatrix(matrix);
    }

    void setVideoSizeCb(on_msg_set_video_size cb, void *context)
    {
        REPORT_FUNCTION();

        assert(media_player_listener != NULL);
        media_player_listener->setVideoSizeCb(cb, context);
    }

    void setVideoTextureNeedsUpdateCb(on_video_texture_needs_update cb, void *context)
    {
        REPORT_FUNCTION();

        assert(frame_listener != NULL);
        frame_listener->setVideoTextureNeedsUpdateCb(cb, context);
    }

    void setErrorCb(on_msg_error cb, void *context)
    {
        REPORT_FUNCTION();

        assert(media_player_listener != NULL);
        media_player_listener->setErrorCb(cb, context);
    }

    void setPlaybackCompleteCb(on_playback_complete cb, void *context)
    {
        REPORT_FUNCTION();

        assert(media_player_listener != NULL);
        media_player_listener->setPlaybackCompleteCb(cb, context);
    }

    void setMediaPreparedCb(on_media_prepared cb, void *context)
    {
        REPORT_FUNCTION();

        assert(media_player_listener != NULL);
        media_player_listener->setMediaPreparedCb(cb, context);
    }

    void getVolume(float *leftVolume, float *rightVolume)
    {
        *leftVolume = left_volume;
        *rightVolume = right_volume;
    }

    android::status_t setVolume(float leftVolume, float rightVolume)
    {
        REPORT_FUNCTION();

        left_volume = leftVolume;
        right_volume = rightVolume;
        return MediaPlayer::setVolume(leftVolume, rightVolume);
    }

    int getSourceFd() const { return source_fd; }
    void setSourceFd(int fd) { source_fd = fd; }

private:
    android::sp<android::SurfaceTexture> texture;
    android::sp<MediaPlayerListenerWrapper> media_player_listener;
    android::sp<FrameAvailableListener> frame_listener;
    float left_volume;
    float right_volume;
    int source_fd;
}; // MediaPlayerWrapper

using namespace android;

// ----- Media Player C API Implementation ----- //

void android_media_set_video_size_cb(MediaPlayerWrapper *mp, on_msg_set_video_size cb, void *context)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return;
    }

    mp->setVideoSizeCb(cb, context);
}

void android_media_set_video_texture_needs_update_cb(MediaPlayerWrapper *mp, on_video_texture_needs_update cb, void *context)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return;
    }

    mp->setVideoTextureNeedsUpdateCb(cb, context);
}

void android_media_set_error_cb(MediaPlayerWrapper *mp, on_msg_error cb, void *context)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return;
    }

    mp->setErrorCb(cb, context);
}

void android_media_set_playback_complete_cb(MediaPlayerWrapper *mp, on_playback_complete cb, void *context)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return;
    }

    mp->setPlaybackCompleteCb(cb, context);
}

void android_media_set_media_prepared_cb(MediaPlayerWrapper *mp, on_media_prepared cb, void *context)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return;
    }

    mp->setMediaPreparedCb(cb, context);
}

MediaPlayerWrapper *android_media_new_player()
{
    REPORT_FUNCTION()

    MediaPlayerWrapper *mp = new MediaPlayerWrapper();
    if (mp == NULL)
    {
        ALOGE("Failed to create new MediaPlayerWrapper instance.");
        return NULL;
    }

    // Required for internal player state processing. Without this, prepare() and start() hang.
    ProcessState::self()->startThreadPool();

    return mp;
}

int android_media_set_data_source(MediaPlayerWrapper *mp, const char* url)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    if (url == NULL)
    {
        ALOGE("url must not be NULL");
        return BAD_VALUE;
    }

    int fd = open(url, O_RDONLY);
    if (fd < 0)
    {
        ALOGE("Failed to open source data at: %s\n", url);
        return BAD_VALUE;
    }

    mp->setSourceFd(fd);

    struct stat st;
    stat(url, &st);

    ALOGD("source file length: %lld\n", st.st_size);

    mp->setDataSource(fd, 0, st.st_size);
    mp->prepare();

    return OK;
}

int android_media_set_preview_texture(MediaPlayerWrapper *mp, int texture_id)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    static const bool allow_synchronous_mode = true;
    // Create a new SurfaceTexture from the texture_id in synchronous mode (don't wait on all data in the buffer)
    mp->setVideoSurfaceTexture(android::sp<android::SurfaceTexture>(
        new android::SurfaceTexture(
            texture_id,
            allow_synchronous_mode)));

    return OK;
}

void android_media_update_surface_texture(MediaPlayerWrapper *mp)
{
    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return;
    }

    mp->updateSurfaceTexture();
}

void android_media_surface_texture_get_transformation_matrix(MediaPlayerWrapper *mp, GLfloat* matrix)
{
    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return;
    }

    mp->get_transformation_matrix_for_surface_texture(matrix);
}

int android_media_play(MediaPlayerWrapper *mp)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    mp->start();
    const char *tmp = mp->isPlaying() ? "yes" : "no";
    ALOGV("Is playing?: %s\n", tmp);

    return OK;
}

int android_media_pause(MediaPlayerWrapper *mp)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    mp->pause();

    return OK;
}

int android_media_stop(MediaPlayerWrapper *mp)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    mp->stop();

    int fd = mp->getSourceFd();
    if (fd > -1)
        close(fd);

    return OK;
}

bool android_media_is_playing(MediaPlayerWrapper *mp)
{
    if (mp != NULL)
    {
        if (mp->isPlaying())
            return true;
    }

    return false;
}

int android_media_seek_to(MediaPlayerWrapper *mp, int msec)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    return mp->seekTo(msec);
}

int android_media_get_current_position(MediaPlayerWrapper *mp, int *msec)
{
    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    return mp->getCurrentPosition(msec);
}

int android_media_get_duration(MediaPlayerWrapper *mp, int *msec)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    return mp->getDuration(msec);
}

int android_media_get_volume(MediaPlayerWrapper *mp, int *volume)
{
    REPORT_FUNCTION()

    if (volume == NULL)
    {
        ALOGE("volume must not be NULL");
        return BAD_VALUE;
    }

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    float left_volume = 0, right_volume = 0;
    mp->getVolume(&left_volume, &right_volume);
    *volume = left_volume * 100;

    return OK;
}

int android_media_set_volume(MediaPlayerWrapper *mp, int volume)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        ALOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    float left_volume = float(volume / 100);
    float right_volume = float(volume / 100);
    return mp->setVolume(left_volume, right_volume);
}
