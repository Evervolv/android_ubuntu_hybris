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
 * Authored by: Thomas Voss
 */

#include <climits>
#include <cstdio>
#include <cstdlib>

#include <unistd.h>

#include "surface_flinger_compatibility_layer.h"

SfSurface* sf_surface_create(SfClient* client, SfSurfaceCreationParameters* params);
EGLSurface sf_surface_get_egl_surface(SfSurface*);
void sf_surface_make_current(SfSurface* surface);

void sf_surface_move_to(SfSurface* surface, int x, int y);
void sf_surface_set_layer(SfSurface* surface, int layer);
void sf_surface_set_alpha(SfSurface* surface, float alpha);


bool setupGraphics();
void hw_render(EGLDisplay, EGLSurface);
void hw_step();

double rotation_increment = 0.01;

int main(int argc, char** argv)
{
    SfClient* sf_client = sf_client_create();

    if (!sf_client)
    {
        printf("Problem creating client ... aborting now.");
        return 1;
    }

    int x = getenv("X") ? atoi(getenv("X")) : 200;
    int y = getenv("Y") ? atoi(getenv("Y")) : 200;

    int w = getenv("W") ? atoi(getenv("W")) : 500;
    int h = getenv("H") ? atoi(getenv("H")) : 500;

    float opacity = getenv("O") ? atof(getenv("O")) : 0.5;
    rotation_increment = getenv("RI") ? atof(getenv("RI")) : 0.01;
    
    SfSurfaceCreationParameters params = 
    {
        x,
        y,
        w,
        h,
        -1, //PIXEL_FORMAT_RGBA_8888,
        INT_MAX,
        opacity,
        "A test surface"
    };
    
    SfSurface* sf_surface = sf_surface_create(sf_client, &params);

    if (!sf_surface)
    {
        printf("Problem creating surface ... aborting now.");
        return 1;
    }

    sf_surface_make_current(sf_surface);

    EGLDisplay disp = sf_client_get_egl_display(sf_client);
    EGLSurface surface = sf_surface_get_egl_surface(sf_surface);

    setupGraphics();

    printf("Turning off screen\n");
    sf_blank(0);
    sleep(1);
    printf("Turning on screen\n");
    sf_unblank(0);
  
    for(;;)
    {
        hw_render(disp, surface);
        hw_step();
    }
}

/* animation state variables */
GLfloat rotation_angle = 0.0f;

static GLuint gProgram;
static GLuint gvPositionHandle, gvColorHandle;
static GLuint rotation_uniform;
static GLint num_vertex = 3;
static const GLfloat triangle[] = {
    -0.125f, -0.125f, 0.0f, 0.5f,
    0.0f,  0.125f, 0.0f, 0.5f,
    0.125f, -0.125f, 0.0f, 0.5f };
static const GLfloat color_triangle[] = {      
    0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 0.0f };
const GLfloat * vertex_data;
const GLfloat * color_data;
static const char gVertexShader[] =
        "attribute vec4 vPosition;\n"
        "attribute vec4 vColor;\n"
        "uniform float angle;\n"
        "varying vec4 colorinfo;\n"
        "void main() {\n"
        "  mat3 rot_z = mat3( vec3( cos(angle),  sin(angle), 0.0),\n"
        "                     vec3(-sin(angle),  cos(angle), 0.0),\n"
        "                     vec3(       0.0,         0.0, 1.0));\n"
        "  gl_Position = vec4(rot_z * vPosition.xyz, 1.0);\n"
        "  colorinfo = vColor;\n"
        "}\n";

static const char gFragmentShader[] = "precision mediump float;\n"
        "varying vec4 colorinfo;\n"
        "void main() {\n"
        "  gl_FragColor = colorinfo;\n"
        "}\n";

/* util functions */
GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    fprintf(stderr, "Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    else
    {
        printf("Error, during shader creation: %i\n", glGetError());
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
	GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
	if (!vertexShader) {
		printf("vertex shader not compiled\n");
		return 0;
	}

	GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
	if (!pixelShader) {
		printf("frag shader not compiled\n");
		return 0;
	}

	GLuint program = glCreateProgram();
	if (program) {
		glAttachShader(program, vertexShader);
		glAttachShader(program, pixelShader);
		glLinkProgram(program);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			GLint bufLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
			if (bufLength) {
				char* buf = (char*) malloc(bufLength);
				if (buf) {
					glGetProgramInfoLog(program, bufLength, NULL, buf);
					fprintf(stderr, "Could not link program:\n%s\n", buf);
					free(buf);
				}
			}
			glDeleteProgram(program);
			program = 0;
		}
	}
	return program;
}

bool setupGraphics() {

    vertex_data = triangle;
    color_data = color_triangle;
    
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram)
    {
        printf("error making program\n");
            return 0;
    }
    
    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    gvColorHandle = glGetAttribLocation(gProgram, "vColor");
    
    rotation_uniform = glGetUniformLocation(gProgram, "angle");
    
    return true;
}

void hw_render(EGLDisplay displ, EGLSurface surface) {
    glUseProgram(gProgram); 
    
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    glUniform1fv(rotation_uniform,1, &rotation_angle);
    
    glVertexAttribPointer(gvColorHandle,    num_vertex, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, color_data);  
    glVertexAttribPointer(gvPositionHandle, num_vertex, GL_FLOAT, GL_FALSE, 0, vertex_data);  
    glEnableVertexAttribArray(gvPositionHandle);
    glEnableVertexAttribArray(gvColorHandle);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertex); 
    glDisableVertexAttribArray(gvPositionHandle);
    glDisableVertexAttribArray(gvColorHandle);
    
    eglSwapBuffers(displ, surface); 
}

void hw_step() {
    rotation_angle += rotation_increment;
    return;
}
