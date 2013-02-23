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

#ifndef CAMERA_COMPATIBILITY_LAYER_H_
#define CAMERA_COMPATIBILITY_LAYER_H_

//#include "camera_compatibility_layer_capabilities.h"

#include <stdint.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

    // Forward declarations
    struct SfSurface;

    typedef enum
    {
        // This camera has higher quality and features like high resolution and flash
        BACK_FACING_CAMERA_TYPE,
        // The camera that is on the same side as the touch display. Usually used for video calls
        FRONT_FACING_CAMERA_TYPE
    } CameraType;

    struct CameraControl;

    struct CameraControlListener
    {
        typedef void (*on_msg_error)(void* context);
        typedef void (*on_msg_shutter)(void* context);
        typedef void (*on_msg_focus)(void* context);
        typedef void (*on_msg_zoom)(void* context, int32_t new_zoom_level);

        typedef void (*on_data_raw_image)(void* data, uint32_t data_size, void* context);
        typedef void (*on_data_compressed_image)(void* data, uint32_t data_size, void* context);
        typedef void (*on_preview_texture_needs_update)(void* context);

        // Called whenever an error occurs while the camera HAL executes a command
        on_msg_error on_msg_error_cb;
        // Called while taking a picture when the shutter has been triggered
        on_msg_shutter on_msg_shutter_cb;
        // Called while focusing
        on_msg_focus on_msg_focus_cb;
        // Called while zooming
        on_msg_zoom on_msg_zoom_cb;

        // Raw image data (Bayer pattern) is reported over this callback
        on_data_raw_image on_data_raw_image_cb;
        // JPEG-compressed image is reported over this callback
        on_data_compressed_image on_data_compressed_image_cb;

        // If a texture has been set as a destination for preview frames,
        // this callback is invoked whenever a new buffer from the camera is available
        // and needs to be uploaded to the texture by means of calling
        // android_camera_update_preview_texture. Please note that this callback can
        // be invoked on any thread, and android_camera_update_preview_texture must only
        // be called on the thread that setup the EGL/GL context.
        on_preview_texture_needs_update on_preview_texture_needs_update_cb;

        void* context;
    };

    // Initializes a connection to the camera, returns NULL on error.
    CameraControl* android_camera_connect_to(CameraType camera_type, CameraControlListener* listener);

    // Disconnects the camera and deletes the pointer
    void android_camera_disconnect(CameraControl* control);

    // Deletes the CameraControl
    void android_camera_delete(CameraControl* control);

    // Passes the rotation r of the display in [°] relative to the camera to the camera HAL. r \in [0, 359].
    void android_camera_set_display_orientation(CameraControl* control, int32_t clockwise_rotation_degree);

    // Prepares the camera HAL to display preview images to the
    // supplied surface/texture in a H/W-acclerated way.  New frames
    // are reported via the
    // 'on_preview_texture_needs_update'-callback, and clients of this
    // API should invoke android_camera_update_preview_texture
    // subsequently. Please note that the texture is bound automatically by the underlying
    // implementation.
    void android_camera_set_preview_texture(CameraControl* control, int texture_id);

    // Reads out the transformation matrix that needs to be applied when displaying the texture
    void android_camera_get_preview_texture_transformation(CameraControl* control, float m[16]);

    // Updates the texture to the last received frame and binds the texture
    void android_camera_update_preview_texture(CameraControl* control);

    // Prepares the camera HAL to display preview images to the supplied surface/texture in a H/W-acclerated way.
    void android_camera_set_preview_surface(CameraControl* control, SfSurface* surface);

    // Starts the camera preview
    void android_camera_start_preview(CameraControl* control);

    // Stops the camera preview
    void android_camera_stop_preview(CameraControl* control);

    // Starts an autofocus operation of the camera, results are reported via callback.
    void android_camera_start_autofocus(CameraControl* control);

    // Stops an ongoing autofocus operation.
    void android_camera_stop_autofocus(CameraControl* control);

    // Starts a zooming operation, zoom values are absolute, valid values [1, \infty].
    void android_camera_start_zoom(CameraControl* control, int32_t zoom);

    // Stops an ongoing zoom operation.
    void android_camera_stop_zoom(CameraControl* control);

    // Adjust the zoom level immediately as opposed to smoothly zoomin gin.
    void android_camera_set_zoom(CameraControl* control, int32_t zoom);

    // Takes a picture and reports back image data via
    // callback. Please note that this stops the preview and thus, the
    // preview needs to be restarted after the picture operation has
    // completed. Ideally, this is done from the raw data callback.
    void android_camera_take_snapshot(CameraControl* control);

#ifdef __cplusplus
}
#endif

#endif // CAMERA_COMPATIBILITY_LAYER_H_
