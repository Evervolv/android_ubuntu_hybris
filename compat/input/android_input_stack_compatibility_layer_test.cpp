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
#include "input_stack_compatibility_layer.h"

#include <gtest/gtest.h>
#include <utils/threads.h>

#include <signal.h>

namespace
{

bool g_stop = false;

void signal_handler(int)
{
    g_stop = true;
}

void on_new_event(Event* event, void* context)
{
    printf("%s", __PRETTY_FUNCTION__);

    printf("\tEventType: %d \n", event->type);
    printf("\tdevice_id: %d \n", event->device_id);
    printf("\tsource_id: %d \n", event->source_id);
    printf("\taction: %d \n", event->action);
    printf("\tflags: %d \n", event->flags);
    printf("\tmeta_state: %d \n", event->meta_state);
    switch(event->type)
    {
    case MOTION_EVENT_TYPE:
        printf("\tdetails.motion.event_time: %d\n", event->details.motion.event_time);
        printf("\tdetails.motion.pointer_coords.x: %f\n", event->details.motion.pointer_coordinates[0].x);
        printf("\tdetails.motion.pointer_coords.y: %f\n", event->details.motion.pointer_coordinates[0].y);
        break;
    default:
        break;
    }
}

}

int main(int argc, char** argv)
{
    g_stop = false;
    signal(SIGINT, signal_handler);

    AndroidEventListener listener;
    listener.on_new_event = on_new_event;
    listener.context = NULL;

    InputStackConfiguration config =
    {
enable_touch_point_visualization :
        true,
        default_layer_for_touch_point_visualization : 10000
    };

    android_input_stack_initialize(&listener, &config);
    android_input_stack_start_waiting_for_flag(&g_stop);

    android_input_stack_stop();
    android_input_stack_shutdown();
}

