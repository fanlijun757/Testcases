/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sched.h>
#include <sys/resource.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <utils/Timers.h>

#include <WindowSurface.h>
#include <ui/GraphicBuffer.h>
#include <EGLUtils.h>
#include "KnightModel.h"
#include "matrix.h"

using namespace android;

static void checkEglError(const char* op, EGLBoolean returnVal = EGL_TRUE) {
    if (returnVal != EGL_TRUE) {
        fprintf(stderr, "%s() returned %d\n", op, returnVal);
    }

    for (EGLint error = eglGetError(); error != EGL_SUCCESS; error
            = eglGetError()) {
        fprintf(stderr, "after %s() eglError %s (0x%x)\n", op, EGLUtils::strerror(error),
                error);
    }
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        fprintf(stderr, "after %s() glError (0x%x)\n", op, error);
    }
}

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
            } else {
                fprintf(stderr, "Guessing at GL_INFO_LOG_LENGTH size\n");
                char* buf = (char*) malloc(0x1000);
                if (buf) {
                    glGetShaderInfoLog(shader, 0x1000, NULL, buf);
                    fprintf(stderr, "Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
            }
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pGeoSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint geoShader = loadShader(GL_GEOMETRY_SHADER, pGeoSource);
    if (!geoShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, geoShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
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


GLuint createProgram_ori(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }


    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
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


GLuint gProgram_depth;
GLuint gProgram_shadow;
GLuint gProgram_show_depth;
GLint gvPositionHandle;
GLint gYuvTexSamplerHandle;

static const char gVertexShader_shadow[] =
	"#version 320 es\n"
	"\n"
	"layout(location = 0) in vec3 vPosition;\n"
	"layout(location = 1) in vec3 vNormal;\n"
	"uniform mat4 mvp;\n"
	"uniform mat4 LightMvp;\n"
	"uniform float Light_X;\n"
	"uniform float Light_Y;\n"
	"uniform float Light_Z;\n"
	"out vec4 worldPos;\n"
	"out vec3 vsNormal;\n"
  "void main() {\n"
  "  gl_Position = mvp*vec4(vPosition, 1.0);\n"
  "  worldPos = LightMvp*vec4(vPosition, 1.0);\n"
  "  vsNormal  = vNormal;"
  "}\n";


static const char gFragmentShader_shadow[] =
	"#version 320 es\n"
	"\n"
  "precision highp float;\n"
  "uniform sampler2D  depthTex;\n"
  "out vec4 color;\n"
  "in  vec4 worldPos;\n"
  "in  vec3 vsNormal;\n"
  "void main() {\n"
  "      vec3 depthTexCoord = worldPos.xyz/worldPos.w;\n"
  "      depthTexCoord.xyz = depthTexCoord.xyz*vec3(0.5) + vec3(0.5);\n"
  "      float depth = texture(depthTex, depthTexCoord.xy).x;\n"
  "      float z = depthTexCoord.z;\n"
  "      vec4 test;\n"
  "      if(z > (depth)) {\n"
  "          test = vec4(0.0, 0.0, 0.0, 1.0);\n"
  "      } else {"
  "          test = vec4(1.0, 1.0, 1.0, 1.0);\n"
  "     }\n"
  "     color = vec4(vsNormal, 1.0) * test + vec4(0.0, 0.0, 0.0, 1.0);\n"
  "}\n";

static const char gVertexShader_depth[] =
  "#version 320 es\n"
  "\n"
	"layout(location = 0) in vec3 vPosition;\n"
	"uniform mat4 mvp;\n"
  "void main() {\n"
  "  gl_Position = mvp*vec4(vPosition, 1.0);\n"
  "}\n";

static const char gFragmentShader_depth[] =
  "#version 320 es\n"
  "\n"
  "precision highp float;\n"
  "uniform vec4 color;\n"
  "out vec4 color_out;"
  "void main() {\n"
  "  color_out = vec4(0.0, 0.0, 1.0, 1.0);\n"
  "}\n";
  
static const char gVertexShader_show_depth[] =
  "#version 320 es\n"
  "\n"
	"layout(location = 0) in vec4 vPosition;\n"
	"out vec2 texCoord;"
  "void main() {\n"
  "  gl_Position = vec4(vPosition.x, vPosition.y, 0.0, 1.0);\n"
  "  texCoord = vec2(vPosition.z, vPosition.w);\n"
  "}\n";

static const char gFragmentShader_show_depth[] =
  "#version 320 es\n"
  "\n"
  "precision highp float;\n"
  "uniform sampler2D depthTex;\n"
  "in vec2 texCoord;"
  "out vec4 color_out;"
  "void main() {\n"
  "  float depth = texture(depthTex, texCoord).r;\n"
  "  color_out = vec4((1.0-depth)*5.0, (1.0-depth)*5.0, (1.0-depth)*5.0, 1.0);\n"
  "}\n";

const GLfloat gTriangleVertices[] = {
    -0.5f, 0.0,  0.5f, 1.0, 1.0, 1.0,
     0.5f, 0.0, -0.5f, 1.0, 1.0, 1.0,
    -0.5f, 0.0, -0.5f, 1.0, 1.0, 1.0,
    -0.5f, 0.0,  0.5f, 1.0, 1.0, 1.0,
     0.5f, 0.0,  0.5f, 1.0, 1.0, 1.0,
     0.5f, 0.0, -0.5f, 1.0, 1.0, 1.0,
};

const GLfloat gRectangle[] = {
     0.5f, 0.5,  0.0, 0.0,
     0.9f, 0.9,  1.0, 1.0,
     0.5f, 0.9,  0.0, 1.0,
     0.5f, 0.5,  0.0, 0.0,
     0.9f, 0.5,  1.0, 0.0,
     0.9f, 0.9,  1.0, 1.0,
};


float r[16];
float s[16];
float t[16];
float m[16];
float v[16];
float p[16];
float mv[16];
float mvp[16];
float temp[16];
float LightV[16];
float LightP[16];
float lightMvp[16];
#define PI 3.1415926
GLuint depthTex = 0;
GLuint colorTex = 0;
GLuint depthFBO = 0;
float Light_X =  -5;
float Light_Y =  5;
float Light_Z = 2;
float maxY = 0.0;
float minY = 0.0;
float maxZ = 0.0;
float minZ = 0.0;
float rotate = 0;

EGLint screen_w, screen_h;

bool setupGraphics(int w, int h) {
    gProgram_depth = createProgram_ori(gVertexShader_depth, gFragmentShader_depth);
    if (!gProgram_depth) {
        return false;
    }

    gProgram_shadow = createProgram_ori(gVertexShader_shadow, gFragmentShader_shadow);
    if (!gProgram_shadow) {
        return false;
    }
    
    gProgram_show_depth = createProgram_ori(gVertexShader_show_depth, gFragmentShader_show_depth);
    if (!gProgram_show_depth) {
        return false;
    }
    
    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    checkGlError("e");
    
    glGenFramebuffers(1, &depthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);

		glGenTextures(1, &depthTex);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		checkGlError("d");
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
		checkGlError("a");
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		checkGlError("b");

		glGenTextures(1, &colorTex);
		glBindTexture(GL_TEXTURE_2D, colorTex);
		checkGlError("d");
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		checkGlError("a");
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
		checkGlError("c");
		printf("colorTex=%d depthTex=%d depthFBO=%d\n", colorTex, depthTex, depthFBO);

		int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		checkGlError("11");
		if(status != GL_FRAMEBUFFER_COMPLETE) {
			printf("Fuck!\n");
			exit(1);
		}

		for(int i=0; i<knight_numVertices; i++) {
				if(minY > knight_vertices[i].position[1]){
					minY = knight_vertices[i].position[1];	
				}
				if(maxY < knight_vertices[i].position[1]){
					maxY = knight_vertices[i].position[1];	
				}
		}
		
		printf("maxY=%f minY=%f\n", maxY, minY);
		
		for(int i=0; i<knight_numVertices; i++) {
				if(minZ > knight_vertices[i].position[2]){
					minZ = knight_vertices[i].position[2];	
				}
				if(maxZ < knight_vertices[i].position[2]){
					maxZ = knight_vertices[i].position[2];	
				}
		}
		
		printf("maxY=%f minY=%f\n", maxZ, minZ);


    return true;
}


void drawDepth() {
		setLookAt(LightV, Light_X, Light_Y, Light_Z, 0, 0, 0, 1, 0, 0);
		perspective_matrix(PI/6, 1, 0.9, 100.0, LightP);

		rotate_matrix(rotate, 0, 1, 0, r);
		setScaling(s, 1, 1, 1);
		setTranslate(t, 0, minY*-1, 0);
		multiply_matrix(s, r, temp);
		multiply_matrix(t, temp, m);
		multiply_matrix(LightV, m, mv);
		multiply_matrix(LightP, mv, lightMvp);

		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTex, 0);
		glBindTexture(GL_TEXTURE_2D, colorTex);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);
		
		
		glViewport(0, 0, 1024, 1024);

		glClearDepthf(1.0f);
		glClearColor(1.0, 0.0, 0.0, 1.0);
		glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );


		glUseProgram(gProgram_depth);
		glUniformMatrix4fv( glGetUniformLocation (gProgram_depth, "mvp"), 1, GL_FALSE, lightMvp);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 48, knight_vertices);
		glDrawElements(GL_TRIANGLES, knight_numIndices, GL_UNSIGNED_INT, knight_indices);


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		checkGlError("drawDepth");
}

void drawShowDepth() {
	  glBindFramebuffer(GL_FRAMEBUFFER, 0);
	  glViewport(0, 0, screen_w, screen_h);
	  	  
		glUseProgram(gProgram_show_depth);
	  
		checkGlError("3");
	  glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		
		glUniform1i(glGetUniformLocation (gProgram_show_depth, "depthTex"), 0);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, gRectangle);
		glDrawArrays(GL_TRIANGLES, 0, 6);
}

void drawSence() {
		glUseProgram(gProgram_shadow);

		setLookAt(v, 0, 3, 5, 0, 0, 0, 0, 1, 0);
		perspective_matrix(PI/6, 1, 0.9, 100.0, p);

		rotate_matrix(rotate, 0, 1, 0, r);
		setScaling(s, 1, 1, 1);
		setTranslate(t, 0, minY*-1, 0);
		multiply_matrix(s, r, temp);
		multiply_matrix(t, temp, m);
		multiply_matrix(v, m, mv);
		multiply_matrix(p, mv, mvp);
		multiply_matrix(LightV, m, mv);
		multiply_matrix(LightP, mv, lightMvp);

	  glBindFramebuffer(GL_FRAMEBUFFER, 0);
	  glViewport(0, 0, screen_w, screen_h);
		checkGlError("3");
	  glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthTex);
		checkGlError("33");
		glUniform1i(glGetUniformLocation (gProgram_shadow, "depthTex"), 0);
		
		
		glUniformMatrix4fv( glGetUniformLocation (gProgram_shadow, "LightMvp"), 1, GL_FALSE, lightMvp);
		
		glUniformMatrix4fv( glGetUniformLocation (gProgram_shadow, "mvp"), 1, GL_FALSE, mvp);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 48, knight_vertices);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 48, (void*)((char*)knight_vertices+12));
		glDrawElements(GL_TRIANGLES, knight_numIndices, GL_UNSIGNED_INT, knight_indices);

		setScaling(s, 10, 10, 10);
		setIdentity(t);
		multiply_matrix(t, s, m);
		multiply_matrix(v, m, mv);
		multiply_matrix(p, mv, mvp);
		
		multiply_matrix(LightV, m, mv);
		multiply_matrix(LightP, mv, lightMvp);
		
		glUniformMatrix4fv( glGetUniformLocation (gProgram_shadow, "LightMvp"), 1, GL_FALSE, lightMvp);
		glUniformMatrix4fv( glGetUniformLocation (gProgram_shadow, "mvp"), 1, GL_FALSE, mvp);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, gTriangleVertices);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void*)((char*)gTriangleVertices+12));
		glDrawArrays(GL_TRIANGLES, 0, 6);
			checkGlError("6");
}


void renderFrame() {
		{
			(void)m;
			(void)mv;
		}
		
		rotate = 170;
		
		//printf("rotate=%f\n", rotate);

		glEnable(GL_DEPTH_TEST);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );

		drawDepth();
		checkGlError("1");
		drawSence();
		drawShowDepth();
		checkGlError("2");
}

int main(int /*argc*/, char** /*argv*/) {
    EGLBoolean returnValue;
    EGLConfig myConfig = {0};

    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    EGLint s_configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE };
    EGLint majorVersion;
    EGLint minorVersion;
    EGLContext context;
    EGLSurface surface;

    EGLDisplay dpy;

    checkEglError("<init>");
    dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    checkEglError("eglGetDisplay");
    if (dpy == EGL_NO_DISPLAY) {
        printf("eglGetDisplay returned EGL_NO_DISPLAY.\n");
        return 0;
    }

    returnValue = eglInitialize(dpy, &majorVersion, &minorVersion);
    checkEglError("eglInitialize", returnValue);
    fprintf(stderr, "EGL version %d.%d\n", majorVersion, minorVersion);
    if (returnValue != EGL_TRUE) {
        printf("eglInitialize failed\n");
        return 0;
    }

    WindowSurface windowSurface;
    EGLNativeWindowType window = windowSurface.getSurface();
    returnValue = EGLUtils::selectConfigForNativeWindow(dpy, s_configAttribs, window, &myConfig);
    if (returnValue) {
        printf("EGLUtils::selectConfigForNativeWindow() returned %d", returnValue);
        return 1;
    }

    checkEglError("EGLUtils::selectConfigForNativeWindow");


    surface = eglCreateWindowSurface(dpy, myConfig, window, NULL);
    checkEglError("eglCreateWindowSurface");
    if (surface == EGL_NO_SURFACE) {
        printf("gelCreateWindowSurface failed.\n");
        return 1;
    }

    context = eglCreateContext(dpy, myConfig, EGL_NO_CONTEXT, context_attribs);
    checkEglError("eglCreateContext");
    if (context == EGL_NO_CONTEXT) {
        printf("eglCreateContext failed\n");
        return 1;
    }
    returnValue = eglMakeCurrent(dpy, surface, surface, context);
    checkEglError("eglMakeCurrent", returnValue);
    if (returnValue != EGL_TRUE) {
        return 1;
    }
    eglQuerySurface(dpy, surface, EGL_WIDTH, &screen_w);
    checkEglError("eglQuerySurface");
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &screen_h);
    checkEglError("eglQuerySurface");

    fprintf(stderr, "Window dimensions: %d x %d\n", screen_w, screen_h);

    if(!setupGraphics(screen_w, screen_h)) {
        fprintf(stderr, "Could not set up graphics.\n");
        return 1;
    }

    for (;;) {
        renderFrame();
        eglSwapBuffers(dpy, surface);
        checkEglError("eglSwapBuffers");
    }

    return 0;
}
