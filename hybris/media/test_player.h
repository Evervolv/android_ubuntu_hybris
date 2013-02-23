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

#ifndef TEST_PLAYER_H_
#define TEST_PLAYER_H_

#include <errno.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
//#include <utils/threads.h>

namespace android {

enum {
    OK          = 0,
    NO_ERROR    = 0,
    BAD_VALUE   = -EINVAL,
};

#if 0
class RenderInput;

class WindowRenderer
{
public:
    WindowRenderer(int width, int height);
    ~WindowRenderer();

private:
    // The GL thread functions
    static int threadStart(void* self);
    void glThread();

    // These variables are used to communicate between the GL thread and
    // other threads.
    Mutex mLock;
    Condition mCond;
    enum {
        CMD_IDLE,
        CMD_RENDER_INPUT,
        CMD_RESERVE_TEXTURE,
        CMD_DELETE_TEXTURE,
        CMD_QUIT,
    };
    int mThreadCmd;
    RenderInput* mThreadRenderInput;
    GLuint mThreadTextureId;
};
#endif
} // android

#endif
