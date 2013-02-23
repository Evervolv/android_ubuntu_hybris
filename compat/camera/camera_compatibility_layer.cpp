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
#include "camera_compatibility_layer.h"
#include "camera_compatibility_layer_capabilities.h"
#include "camera_compatibility_layer_configuration_translator.h"

#include <surface_flinger_compatibility_layer_internal.h>

#include <binder/ProcessState.h>
#include <camera/Camera.h>
#include <camera/CameraParameters.h>
#include <gui/SurfaceTexture.h>

#undef LOG_TAG
#define LOG_TAG "CameraCompatibilityLayer"
#include <utils/KeyedVector.h>
#include <utils/Log.h>


#define REPORT_FUNCTION() ALOGV("%s \n", __PRETTY_FUNCTION__);

struct CameraControl : public android::CameraListener,
    public android::SurfaceTexture::FrameAvailableListener
{
    android::Mutex guard;
    CameraControlListener* listener;
    android::sp<android::Camera> camera;
    android::CameraParameters camera_parameters;
    android::sp<android::SurfaceTexture> preview_texture;

    // From android::SurfaceTexture::FrameAvailableListener
    void onFrameAvailable()
    {
        REPORT_FUNCTION();
        if (listener)
            listener->on_preview_texture_needs_update_cb(listener->context);
    }

    // From android::CameraListener
    void notify(int32_t msg_type, int32_t ext1, int32_t ext2)
    {
        REPORT_FUNCTION();
        printf("\text1: %d, ext2: %d \n", ext1, ext2);
        if (!listener)
            return;

        switch(msg_type)
        {
        case CAMERA_MSG_ERROR:
            if (listener->on_msg_error_cb)
                listener->on_msg_error_cb(listener->context);
            break;
        case CAMERA_MSG_SHUTTER:
            if (listener->on_msg_shutter_cb)
                listener->on_msg_shutter_cb(listener->context);
            break;
        case CAMERA_MSG_ZOOM:
            if (listener->on_msg_zoom_cb)
                listener->on_msg_zoom_cb(listener->context, ext1);
            break;
        case CAMERA_MSG_FOCUS:
            if (listener->on_msg_focus_cb)
                listener->on_msg_focus_cb(listener->context);
            break;
        default:
            break;
        }
    }

    void postData(
        int32_t msg_type,
        const android::sp<android::IMemory>& data,
        camera_frame_metadata_t* metadata)
    {
        REPORT_FUNCTION()
        if (!listener)
            return;

        switch(msg_type)
        {
        case CAMERA_MSG_RAW_IMAGE:
            if (listener->on_data_raw_image_cb)
                listener->on_data_raw_image_cb(data->pointer(), data->size(), listener->context);
            break;
        case CAMERA_MSG_COMPRESSED_IMAGE:
            if (listener->on_data_compressed_image_cb)
                listener->on_data_compressed_image_cb(data->pointer(), data->size(), listener->context);
            break;
        default:
            break;
        }

        camera->releaseRecordingFrame(data);
    }

    void postDataTimestamp(
        nsecs_t timestamp,
        int32_t msg_type,
        const android::sp<android::IMemory>& data)
    {
        REPORT_FUNCTION()
        (void) timestamp;
        (void) msg_type;
        (void) data;
    }
};
namespace
{


android::sp<CameraControl> camera_control_instance;
}

int android_camera_get_number_of_devices()
{
    REPORT_FUNCTION()
    return android::Camera::getNumberOfCameras();
}

CameraControl* android_camera_connect_to(CameraType camera_type, CameraControlListener* listener)
{
    REPORT_FUNCTION()

    int32_t camera_id;
    int32_t camera_count = camera_id = android::Camera::getNumberOfCameras();
    for (camera_id = 0; camera_id < camera_count; camera_id++)
    {
        android::CameraInfo ci;
        android::Camera::getCameraInfo(camera_id, &ci);

        if (ci.facing == camera_type)
            break;
    }

    if (camera_id == camera_count)
        return NULL;

    CameraControl* cc = new CameraControl();
    cc->listener = listener;
    cc->camera = android::Camera::connect(camera_id);

    if(cc->camera == NULL)
        return NULL;

    cc->camera_parameters = android::CameraParameters(cc->camera->getParameters());

    camera_control_instance = cc;
    cc->camera->setListener(camera_control_instance);
    cc->camera->lock();

    // TODO: Move this to a more generic component
    android::ProcessState::self()->startThreadPool();

    return cc;
}

void android_camera_disconnect(CameraControl* control)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);
    control->camera->disconnect();
    control->camera->unlock();
}

void android_camera_delete(CameraControl* control)
{
    delete control;
}

void android_camera_dump_parameters(CameraControl* control)
{
    REPORT_FUNCTION();
    assert(control);

    printf("%s \n", control->camera->getParameters().string());
}

void android_camera_set_flash_mode(CameraControl* control, FlashMode mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);
    control->camera_parameters.set(
        android::CameraParameters::KEY_FLASH_MODE,
        flash_modes[mode]);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_get_flash_mode(CameraControl* control, FlashMode* mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);
    static const char* flash_mode = control->camera_parameters.get(
                            android::CameraParameters::KEY_FLASH_MODE);
    if (flash_mode)
        *mode = flash_modes_lut.valueFor(android::String8(flash_mode));
    else
        *mode = FLASH_MODE_OFF;
}

void android_camera_set_white_balance_mode(CameraControl* control, WhiteBalanceMode mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    control->camera_parameters.set(
        android::CameraParameters::KEY_WHITE_BALANCE,
        white_balance_modes[mode]);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_get_white_balance_mode(CameraControl* control, WhiteBalanceMode* mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    *mode = white_balance_modes_lut.valueFor(
                android::String8(
                    control->camera_parameters.get(
                        android::CameraParameters::KEY_WHITE_BALANCE)));
}

void android_camera_set_scene_mode(CameraControl* control, SceneMode mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    control->camera_parameters.set(
        android::CameraParameters::KEY_SCENE_MODE,
        scene_modes[mode]);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_get_scene_mode(CameraControl* control, SceneMode* mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    *mode = scene_modes_lut.valueFor(
                android::String8(
                    control->camera_parameters.get(
                        android::CameraParameters::KEY_SCENE_MODE)));
}

void android_camera_set_auto_focus_mode(CameraControl* control, AutoFocusMode mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    control->camera_parameters.set(
        android::CameraParameters::KEY_FOCUS_MODE,
        auto_focus_modes[mode]);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_get_auto_focus_mode(CameraControl* control, AutoFocusMode* mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    *mode = auto_focus_modes_lut.valueFor(
                android::String8(
                    control->camera_parameters.get(
                        android::CameraParameters::KEY_FOCUS_MODE)));
}


void android_camera_set_effect_mode(CameraControl* control, EffectMode mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    control->camera_parameters.set(
        android::CameraParameters::KEY_EFFECT,
        effect_modes[mode]);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_get_effect_mode(CameraControl* control, EffectMode* mode)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    *mode = effect_modes_lut.valueFor(
                android::String8(
                    control->camera_parameters.get(
                        android::CameraParameters::KEY_EFFECT)));
}

void android_camera_get_preview_fps_range(CameraControl* control, int* min, int* max)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    control->camera_parameters.getPreviewFpsRange(min, max);
}

void android_camera_set_preview_fps(CameraControl* control, int fps)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);
    control->camera_parameters.setPreviewFrameRate(fps);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_get_preview_fps(CameraControl* control, int* fps)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);
    *fps = control->camera_parameters.getPreviewFrameRate();
}

void android_camera_enumerate_supported_preview_sizes(CameraControl* control, size_callback cb, void* ctx)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);
    android::Vector<android::Size> sizes;
    control->camera_parameters.getSupportedPreviewSizes(sizes);

    for(unsigned int i = 0; i < sizes.size(); i++)
    {
        cb(ctx, sizes[i].width, sizes[i].height);
    }
}

void android_camera_enumerate_supported_picture_sizes(CameraControl* control, size_callback cb, void* ctx)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);
    android::Vector<android::Size> sizes;
    control->camera_parameters.getSupportedPictureSizes(sizes);

    for(unsigned int i = 0; i < sizes.size(); i++)
    {
        cb(ctx, sizes[i].width, sizes[i].height);
    }
}

void android_camera_get_preview_size(CameraControl* control, int* width, int* height)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    control->camera_parameters.getPreviewSize(width, height);
}

void android_camera_set_preview_size(CameraControl* control, int width, int height)
{

    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    control->camera_parameters.setPreviewSize(width, height);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_get_picture_size(CameraControl* control, int* width, int* height)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    control->camera_parameters.getPictureSize(width, height);
}

void android_camera_set_picture_size(CameraControl* control, int width, int height)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    control->camera_parameters.setPictureSize(width, height);
    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_get_current_zoom(CameraControl* control, int* zoom)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    *zoom = control->camera_parameters.getInt(android::CameraParameters::KEY_ZOOM);
}

void android_camera_get_max_zoom(CameraControl* control, int* zoom)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    *zoom = control->camera_parameters.getInt(android::CameraParameters::KEY_MAX_ZOOM);
}

void android_camera_set_display_orientation(CameraControl* control, int32_t clockwise_rotation_degree)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);
    static const int32_t ignored_parameter = 0;
    control->camera->sendCommand(CAMERA_CMD_SET_DISPLAY_ORIENTATION, clockwise_rotation_degree, ignored_parameter);
}

void android_camera_get_preview_texture_transformation(CameraControl* control, float m[16])
{
    REPORT_FUNCTION()
    assert(control);

    if (control->preview_texture == NULL)
        return;

    control->preview_texture->getTransformMatrix(m);
}

void android_camera_update_preview_texture(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);

    control->preview_texture->updateTexImage();
}

void android_camera_set_preview_texture(CameraControl* control, int texture_id)
{
    REPORT_FUNCTION()
    assert(control);

    static const bool allow_synchronous_mode = false;
   
    if (control->preview_texture == NULL) {
        control->preview_texture = android::sp<android::SurfaceTexture>(
                                        new android::SurfaceTexture(
                                            texture_id,
                                            allow_synchronous_mode));
    }

    control->preview_texture->setFrameAvailableListener(
        android::sp<android::SurfaceTexture::FrameAvailableListener>(control));
    control->camera->setPreviewTexture(control->preview_texture->getBufferQueue());
}

void android_camera_set_preview_surface(CameraControl* control, SfSurface* surface)
{
    REPORT_FUNCTION()
    assert(control);
    assert(surface);

    android::Mutex::Autolock al(control->guard);
    control->camera->setPreviewDisplay(surface->surface);
}

void android_camera_start_preview(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);

    android::Mutex::Autolock al(control->guard);
    control->camera->startPreview();
}

void android_camera_stop_preview(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);

    android::Mutex::Autolock al(control->guard);

    if (control->preview_texture != NULL)
        control->preview_texture->abandon();

    control->camera->stopPreview();
}

void android_camera_start_autofocus(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);

    android::Mutex::Autolock al(control->guard);
    control->camera->autoFocus();
}

void android_camera_stop_autofocus(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);

    android::Mutex::Autolock al(control->guard);
    control->camera->cancelAutoFocus();
}

void android_camera_start_zoom(CameraControl* control, int32_t zoom)
{
    REPORT_FUNCTION()
    assert(control);

    static const int ignored_argument = 0;

    android::Mutex::Autolock al(control->guard);
    control->camera->sendCommand(CAMERA_CMD_START_SMOOTH_ZOOM,
                                 zoom,
                                 ignored_argument);
}

// Adjust the zoom level immediately as opposed to smoothly zoomin gin.
void android_camera_set_zoom(CameraControl* control, int32_t zoom)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    control->camera_parameters.set(
        android::CameraParameters::KEY_ZOOM,
        zoom);

    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_stop_zoom(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);

    static const int ignored_argument = 0;

    android::Mutex::Autolock al(control->guard);
    control->camera->sendCommand(CAMERA_CMD_STOP_SMOOTH_ZOOM,
                                 ignored_argument,
                                 ignored_argument);
}

void android_camera_take_snapshot(CameraControl* control)
{
    REPORT_FUNCTION()
    assert(control);
    android::Mutex::Autolock al(control->guard);
    control->camera->takePicture(CAMERA_MSG_SHUTTER | CAMERA_MSG_COMPRESSED_IMAGE);
}

void android_camera_set_preview_format(CameraControl* control, CameraPixelFormat pf)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    control->camera_parameters.set(
        android::CameraParameters::KEY_PREVIEW_FORMAT,
        camera_pixel_formats[pf]);

    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_get_preview_format(CameraControl* control, CameraPixelFormat* pf)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);

    *pf = pixel_formats_lut.valueFor(
              android::String8(
                  control->camera_parameters.get(
                      android::CameraParameters::KEY_PREVIEW_FORMAT)));
}

void android_camera_set_focus_region(
    CameraControl* control,
    FocusRegion* region)
{
    REPORT_FUNCTION()
    assert(control);
    android::Mutex::Autolock al(control->guard);
    static const char* focus_region_pattern = "(%d,%d,%d,%d,%d)";
    static char focus_region[256];
    snprintf(
        focus_region,
        sizeof(focus_region),
        focus_region_pattern,
        region->left,
        region->top,
        region->right,
        region->bottom,
        region->weight);

    control->camera_parameters.set(
        android::CameraParameters::KEY_FOCUS_AREAS,
        focus_region);

    control->camera->setParameters(control->camera_parameters.flatten());
}

void android_camera_reset_focus_region(CameraControl* control)
{
    static FocusRegion region = { 0, 0, 0, 0, 0 };

    android_camera_set_focus_region(control, &region);
}

void android_camera_set_rotation(CameraControl* control, int rotation)
{
    REPORT_FUNCTION();
    assert(control);

    android::Mutex::Autolock al(control->guard);
    control->camera_parameters.set(
        android::CameraParameters::KEY_ROTATION,
        rotation);
    control->camera->setParameters(control->camera_parameters.flatten());
}

