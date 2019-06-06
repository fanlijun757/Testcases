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
//#include "test.h"
//#include "test_monkey.h"
#include "test_sphere.h"
//#include "test_Torus2.h"
//#include "test_dragon.h"
//#include "test_jeep.h"
//#include "test_ori.h"
//#include "Index_ad.h"
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


GLuint gProgram;
GLuint gProgram1;
GLint gvPositionHandle;
GLint gYuvTexSamplerHandle;
float * data_adjacency;



static const char gVertexShader[] = 
	"#version 320 es\n"
	"\n"
	"layout(location = 1) in vec4 vPosition;\n"
	"out vec4 PosL;\n"
    "void main() {\n"
    "  PosL = vPosition;\n"
    "}\n";
/*
static const char gGeoShader[] = 
"#version 320 es\n"
"\n"
"layout (triangles_adjacency) in;\
layout (triangle_strip, max_vertices = 18) out;\
uniform highp mat4 mvp;\
precision highp float;\
uniform vec3 light;\
in vec4 PosL[];\
void main()\
{\
        vec4 v0 = PosL[0];\
		vec4 v1 = PosL[1];\
		vec4 v2 = PosL[2];\
        vec4 v3 = PosL[3];\
		vec4 v4 = PosL[4];\
		vec4 v5 = PosL[5];\
		vec3 nor1 = cross(vec3(v4.xyz - v0.xyz), vec3(v2.xyz-v0.xyz));\
		vec3 nor2 = cross(vec3(v0.xyz - v1.xyz), vec3(v2.xyz-v1.xyz));\
		vec3 nor3 = cross(vec3(v4.xyz - v2.xyz), vec3(v3.xyz-v2.xyz));\
		vec3 nor4 = cross(vec3(v5.xyz - v0.xyz), vec3(v4.xyz-v0.xyz));\
		if(dot(nor1, light) > 0.0 ) {\
			if(dot(nor2, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(0.01), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(10000.0), 0.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(0.01), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(10000.0), 0.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(10000.0), 0.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(0.01), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
			if(dot(nor3, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(0.01), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(10000.0), 0.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(0.01), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(10000.0), 0.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(10000.0), 0.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(0.01), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
			if(dot(nor4, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(0.01), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(10000.0), 0.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(0.01), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(10000.0), 0.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(10000.0), 0.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(0.01), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
		}\
}\n";
*/

static const char gGeoShader1[] = 
"#version 320 es\n"
"\n"
"layout (triangles_adjacency) in;\
layout (triangle_strip, max_vertices = 18) out;\
uniform highp mat4 mvp;\
precision highp float;\
uniform vec3 light;\
in vec4 PosL[];\
float offset = 0.0;\
float Infinite_W = 0.1;\
void main()\
{\
    vec4 v0 = PosL[0];\
		vec4 v1 = PosL[1];\
		vec4 v2 = PosL[2];\
    vec4 v3 = PosL[3];\
		vec4 v4 = PosL[4];\
		vec4 v5 = PosL[5];\
		vec3 nor1 = cross(vec3(v4.xyz - v0.xyz), vec3(v2.xyz-v0.xyz));\
		vec3 nor2 = cross(vec3(v0.xyz - v1.xyz), vec3(v2.xyz-v1.xyz));\
		vec3 nor3 = cross(vec3(v4.xyz - v2.xyz), vec3(v3.xyz-v2.xyz));\
		vec3 nor4 = cross(vec3(v5.xyz - v0.xyz), vec3(v4.xyz-v0.xyz));\
		if(dot(nor1, light) > 0.0 ) {\
			if(dot(nor2, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(light, Infinite_W);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
		        gl_Position = mvp*vec4(light, Infinite_W);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(light, Infinite_W);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
			if(dot(nor3, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(light, Infinite_W);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
		        gl_Position = mvp*vec4(light, Infinite_W);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(light, Infinite_W);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
			if(dot(nor4, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(light, Infinite_W);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
		        gl_Position = mvp*vec4(light, Infinite_W);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(light, Infinite_W);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
		}\
}\n";

#if 1
static const char gGeoShader[] = 
"#version 320 es\n"
"\n"
"layout (triangles_adjacency) in;\
layout (triangle_strip, max_vertices = 18) out;\
uniform highp mat4 mvp;\
precision highp float;\
uniform vec3 light;\
in vec4 PosL[];\
float offset = 0.0;\
float Infinite_W = 1000.0;\
void main()\
{\
    vec4 v0 = PosL[0];\
		vec4 v1 = PosL[1];\
		vec4 v2 = PosL[2];\
    vec4 v3 = PosL[3];\
		vec4 v4 = PosL[4];\
		vec4 v5 = PosL[5];\
		vec3 nor1 = cross(vec3(v2.xyz-v0.xyz), vec3(v4.xyz-v0.xyz));\
		vec3 nor2 = cross(vec3(v2.xyz-v1.xyz), vec3(v0.xyz-v1.xyz));\
		vec3 nor3 = cross(vec3(v3.xyz-v2.xyz), vec3(v4.xyz-v2.xyz));\
		vec3 nor4 = cross(vec3(v4.xyz-v0.xyz), vec3(v5.xyz-v0.xyz));\
		if(dot(nor1, light) > 0.0 ) {\
			if(dot(nor2, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(Infinite_W), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(Infinite_W), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(Infinite_W), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
			if(dot(nor3, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(Infinite_W), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(Infinite_W), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(Infinite_W), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
			if(dot(nor4, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(Infinite_W), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(Infinite_W), 1.0);;\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(Infinite_W), 1.0);;\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
		}\
}\n";
#else

static const char gGeoShader[] = 
"#version 320 es\n"
"\n"
"layout (triangles_adjacency) in;\
layout (line_strip, max_vertices = 18) out;\
uniform highp mat4 mvp;\
precision highp float;\
uniform vec3 light;\
in vec4 PosL[];\
float offset = 0.0;\
float Infinite_W = 1000.0;\
void main()\
{\
    vec4 v0 = PosL[0];\
		vec4 v1 = PosL[1];\
		vec4 v2 = PosL[2];\
    vec4 v3 = PosL[3];\
		vec4 v4 = PosL[4];\
		vec4 v5 = PosL[5];\
		vec3 nor1 = cross(vec3(v2.xyz-v0.xyz), vec3(v4.xyz-v0.xyz));\
		vec3 nor2 = cross(vec3(v2.xyz-v1.xyz), vec3(v0.xyz-v1.xyz));\
		vec3 nor3 = cross(vec3(v3.xyz-v2.xyz), vec3(v4.xyz-v2.xyz));\
		vec3 nor4 = cross(vec3(v4.xyz-v0.xyz), vec3(v5.xyz-v0.xyz));\
		if(dot(nor1, light) > 0.0 ) {\
			if(dot(nor2, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
			if(dot(nor3, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[2].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
			if(dot(nor4, light) <= 0.0) {\
		        gl_Position = mvp*vec4(PosL[4].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        gl_Position = mvp*vec4(PosL[0].xyz + light*vec3(offset), 1.0);\
		        EmitVertex();\
		        EndPrimitive();\
			}\
		}\
}\n";

#endif

static const char gFragmentShader[] = 
	"#version 320 es\n"
	"\n"
    "precision highp float;\n"
    "out vec4 color;\n"
    "void main() {\n"
    "  if(gl_FrontFacing)\n"
    "      color = vec4(1.0);\n"
    "  else\n"
    "      color = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

static const char gVertexShader_ori[] = 
	"attribute vec4 vPosition;\n"
	"uniform mat4 mvp;\n"
    "void main() {\n"
    "  gl_Position = mvp*vPosition;\n"
    "}\n";
    
static const char gFragmentShader_ori[] = 
    "precision highp float;\n"
    "uniform vec4 color;\n"
    "void main() {\n"
    "  gl_FragColor = color;\n"
    "}\n";



bool setupGraphics(int w, int h) {
    gProgram = createProgram(gVertexShader, gGeoShader, gFragmentShader);
    if (!gProgram) {
        return false;
    }

    gProgram1 = createProgram_ori(gVertexShader_ori, gFragmentShader_ori);
    if (!gProgram1) {
        return false;
    }
    
    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    
    data_adjacency = (float*)malloc(sizeof(float)*3*ver_num*2);
    {
    	int find_num = 0;
    	for(int i=0; i<ver_num; i++) {
    		int offset = ((i%3)==2)?-2:1;
    		int backup = ((i%3)==0)?2:((i%3==1)?-1:-1);
    		float x1 = ori[i*3+0];
    		float y1 = ori[i*3+1];
    		float z1 = ori[i*3+2];
    		float x2 = ori[(i+offset)*3+0];
    		float y2 = ori[(i+offset)*3+1];
    		float z2 = ori[(i+offset)*3+2];
     		float x3 = ori[(i+backup)*3+0];
    		float y3 = ori[(i+backup)*3+1];
    		float z3 = ori[(i+backup)*3+2];
    		int find_one = false;
    		
    		data_adjacency[i*2*3+0] = x1;
    		data_adjacency[i*2*3+1] = y1;
    		data_adjacency[i*2*3+2] = z1;
    		
    		for(int j=0; j<ver_num; j+=3) {
  				bool find_a = false;
  				bool find_b = false;
  				bool find_c = false;
  				if(i/3==j/3) {
  					continue;
  				}
  				
	    		float x_a = ori[j*3+0];
	    		float y_a = ori[j*3+1];
	    		float z_a = ori[j*3+2];
	    		
	    		float x_b = ori[(j+1)*3+0];
	    		float y_b = ori[(j+1)*3+1];
	    		float z_b = ori[(j+1)*3+2];
	    		
	    		float x_c = ori[(j+2)*3+0];
	    		float y_c = ori[(j+2)*3+1];
	    		float z_c = ori[(j+2)*3+2];
	    		if( (x_a==x1 && y_a==y1 && z_a == z1) ||
	    			(x_a==x2 && y_a==y2 && z_a == z2)) {
	    			find_a = true;
	    		}
	    		if( (x_b==x1 && y_b==y1 && z_b == z1) ||
	    			(x_b==x2 && y_b==y2 && z_b == z2)) {
	    			find_b = true;
	    		}
	    		if( (x_c==x1 && y_c==y1 && z_c == z1) ||
	    			(x_c==x2 && y_c==y2 && z_c == z2)) {
	    			find_c = true;
	    		}
	    		
	    		if((find_a + find_b + find_c) == 2) {
	    			find_num++;
	    			find_one = true;
	    		} else if((find_a + find_b + find_c) == 3) {
	    			printf("Fuck!\n");
	    			assert(1);
	    		}

	    		if(find_a && find_b) {
		    		data_adjacency[(i*2+1)*3+0] = x_c;
		    		data_adjacency[(i*2+1)*3+1] = y_c;
		    		data_adjacency[(i*2+1)*3+2] = z_c;
		    		break;	    			
	    		}
	    		if(find_a && find_c) {
		    		data_adjacency[(i*2+1)*3+0] = x_b;
		    		data_adjacency[(i*2+1)*3+1] = y_b;
		    		data_adjacency[(i*2+1)*3+2] = z_b;    
		    		break;	  	    			
	    		}
	    		if(find_c && find_b) {
		    		data_adjacency[(i*2+1)*3+0] = x_a;
		    		data_adjacency[(i*2+1)*3+1] = y_a;
		    		data_adjacency[(i*2+1)*3+2] = z_a;   
		    		break;	   	    			
	    		}
    		}
    		
    		if(find_one == false) {
	    		data_adjacency[(i*2+1)*3+0] = x3;
	    		data_adjacency[(i*2+1)*3+1] = y3;
	    		data_adjacency[(i*2+1)*3+2] = z3;     		
    		}
    	}
    	printf("find_num=%d\n", find_num);
    }
    return true;
}

const GLfloat gTriangleVertices[] = {
    -0.5f, 0.0,  0.5f,
     0.5f, 0.0, -0.5f,
    -0.5f, 0.0, -0.5f,
    -0.5f, 0.0,  0.5f,
     0.5f, 0.0,  0.5f,
     0.5f, 0.0, -0.5f,
};

const GLfloat gFullScareenVertices[] = {
    -1.0f,  1.0f, 0.0, 
    -1.0f, -1.0f, 0.0, 
     1.0f, -1.0f, 0.0, 
     1.0f,  1.0f, 0.0,
};
float r[16];
float s[16];
float t[16];
float m[16];
float v[16];
float p[16];
float mv[16];
float mvp[16];
#define PI 3.1415926
static float rotate = 100;

void drawSence() {
		glUseProgram(gProgram1);

		multiply_matrix(t, s, m);
		multiply_matrix(v, m, mv);
		multiply_matrix(p, mv, mvp);

		checkGlError("glUseProgram");

		glVertexAttribPointer(glGetAttribLocation (gProgram1, "vPosition"), 3, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
		checkGlError("glVertexAttribPointer");
		glEnableVertexAttribArray(glGetAttribLocation (gProgram1, "vPosition"));
		checkGlError("glEnableVertexAttribArray");

		glUniform4f( glGetUniformLocation (gProgram1, "color"), 0.5, 0.5, 0.5, 1.0);
		glUniformMatrix4fv( glGetUniformLocation (gProgram1, "mvp"), 1, GL_FALSE, mvp);

		//Draw the floor
		glDrawArrays(GL_TRIANGLES, 0, 6);
		checkGlError("glDrawArrays1");		


		rotate_matrix(rotate, 0, 1, 0, m);
		multiply_matrix(v, m, mv);
		multiply_matrix(p, mv, mvp);
		glVertexAttribPointer(glGetAttribLocation (gProgram1, "vPosition"), 3, GL_FLOAT, GL_FALSE, 0, ori);
		glEnableVertexAttribArray(glGetAttribLocation (gProgram1, "vPosition"));
		checkGlError("glEnableVertexAttribArray");
		
		glUniform4f( glGetUniformLocation (gProgram1, "color"), 1.0, 0.5, 0.5, 1.0);
				
		glUniformMatrix4fv( glGetUniformLocation (gProgram1, "mvp"), 1, GL_FALSE, mvp);

		//Draw the object
		glDrawArrays(GL_TRIANGLES, 0, ver_num);
		checkGlError("glDrawArrays0");
}


void drawSenceShadow() {
		glUseProgram(gProgram1);

#if 1
		multiply_matrix(t, s, m);
		multiply_matrix(v, m, mv);
		multiply_matrix(p, mv, mvp);

		checkGlError("glUseProgram");

		glVertexAttribPointer(glGetAttribLocation (gProgram1, "vPosition"), 3, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
		checkGlError("glVertexAttribPointer");
		glEnableVertexAttribArray(glGetAttribLocation (gProgram1, "vPosition"));
		checkGlError("glEnableVertexAttribArray");

		glUniform4f( glGetUniformLocation (gProgram1, "color"), 0.25, 0.25, 0.25, 1.0);
		glUniformMatrix4fv( glGetUniformLocation (gProgram1, "mvp"), 1, GL_FALSE, mvp);

		//Draw the floor

		glDrawArrays(GL_TRIANGLES, 0, 6);
		checkGlError("glDrawArrays1");		


		rotate_matrix(rotate, 0, 1, 0, m);
		multiply_matrix(v, m, mv);
		multiply_matrix(p, mv, mvp);
		glVertexAttribPointer(glGetAttribLocation (gProgram1, "vPosition"), 3, GL_FLOAT, GL_FALSE, 0, ori);
		glEnableVertexAttribArray(glGetAttribLocation (gProgram1, "vPosition"));
		checkGlError("glEnableVertexAttribArray");

		glUniform4f( glGetUniformLocation (gProgram1, "color"), 0.5, 0.25, 0.25, 1.0);

				
		glUniformMatrix4fv( glGetUniformLocation (gProgram1, "mvp"), 1, GL_FALSE, mvp);

		//Draw the object
		glDrawArrays(GL_TRIANGLES, 0, ver_num);
		checkGlError("glDrawArrays0");
#else
		{
				glDisable(GL_DEPTH_TEST);
				setIdentity(mvp);
				glVertexAttribPointer(glGetAttribLocation (gProgram1, "vPosition"), 3, GL_FLOAT, GL_FALSE, 0, gFullScareenVertices);
				glEnableVertexAttribArray(glGetAttribLocation (gProgram1, "vPosition"));
				checkGlError("glEnableVertexAttribArray");
				
				glUniform4f( glGetUniformLocation (gProgram1, "color"), 0.5, 0.25, 0.25, 1.0);
				glUniformMatrix4fv( glGetUniformLocation (gProgram1, "mvp"), 1, GL_FALSE, mvp);

				//Draw the object
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);			
			
		}
#endif
}
/*
void drawStencil() {
 	//Update the stencil buffer
 	rotate_matrix(rotate, 0, 1, 0, m);
	multiply_matrix(v, m, mv);
	multiply_matrix(p, mv, mvp);
	   
    glUseProgram(gProgram);
    glUniformMatrix4fv( glGetUniformLocation (gProgram, "mvp"), 1, GL_FALSE, mvp);
    glUniform3f( glGetUniformLocation (gProgram, "light"), 0.0, -1.0, 1.0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, vex_data);
    glEnableVertexAttribArray(1);
    
    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xff);
    
    glDisable(GL_CULL_FACE);
    
	glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
	glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);       

    glDrawElements(GL_TRIANGLES_ADJACENCY, index_num, GL_UNSIGNED_INT, index_data);
    checkGlError("glDrawArrays2");  	
}*/
void drawStencil() {
		//Update the stencil buffer
		rotate_matrix(rotate, 0, 1, 0, m);
		multiply_matrix(v, m, mv);
		multiply_matrix(p, mv, mvp);

		glUseProgram(gProgram);
		glUniformMatrix4fv( glGetUniformLocation (gProgram, "mvp"), 1, GL_FALSE, mvp);
		glUniform3f( glGetUniformLocation (gProgram, "light"), 0.0, -1.0, 0.0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, data_adjacency);
		glEnableVertexAttribArray(1);

		glDepthMask(GL_FALSE);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 0, 0xff);

		glDisable(GL_CULL_FACE);
		//glEnable(GL_CULL_FACE);//FAKE
		//glCullFace(GL_FRONT);
		//glDisable(GL_DEPTH_TEST);//FAKE

		glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
		glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);       

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0.0f, -1.0f);

		glDrawArrays(GL_TRIANGLES_ADJACENCY, 0, ver_num*2);
		glDisable(GL_POLYGON_OFFSET_FILL);
		checkGlError("glDrawArrays2");  	
}


void renderFrame() {
		//rotate +=0.1;
		rotate_matrix(rotate, 0, 1, 0, r);
		setTranslate(t, 0, -3, 0);
		setScaling(s, 20.0, 20.0, 20.0);
		setLookAt(v, 0, 20, 30, 0, 0, 0, 0, 1, 0);
		perspective_matrix(PI/6, 1, 0.9, 100000.0, p);

		{
			(void)gTriangleVertices;
			(void)m;
			(void)mv;
			(void)gFullScareenVertices;
			(void)gGeoShader1;
		}

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
		glDepthMask(GL_TRUE);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearStencil(0);
		glClearDepthf(1.0);
		glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);//FAKE
		drawSence();

		
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);//FAKE
		drawStencil();

		//Draw the shadow
		glDepthMask(GL_FALSE);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);  	
		glEnable(GL_CULL_FACE);

		//glDisable(GL_STENCIL_TEST);
		//drawSence(false);

		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_NOTEQUAL, 0x0, 0xFF);
		drawSenceShadow();
}

int main(int /*argc*/, char** /*argv*/) {
    EGLBoolean returnValue;
    EGLConfig myConfig = {0};

    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    EGLint s_configAttribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
            EGL_DEPTH_SIZE, 24,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE };
    EGLint majorVersion;
    EGLint minorVersion;
    EGLContext context;
    EGLSurface surface;
    EGLint w, h;

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
    eglQuerySurface(dpy, surface, EGL_WIDTH, &w);
    checkEglError("eglQuerySurface");
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &h);
    checkEglError("eglQuerySurface");

    fprintf(stderr, "Window dimensions: %d x %d\n", w, h);

    if(!setupGraphics(w, h)) {
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
