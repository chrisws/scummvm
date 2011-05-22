/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <FBase.h>
#include <FAppApplication.h>

#include "system.h"
#include "graphics.h"

int getPowerOf2(int value) {
  int result = 1;
  
  while (result < value) {
    result <<= 1;
  }
  
  return result;
}

//
// BadaGraphicsManager
//
bool BadaGraphicsManager::construct(Osp::App::Application* app) {
  appForm = new BadaAppForm(app);
  return appForm != null;
}

bool BadaGraphicsManager::hasFeature(OSystem::Feature f) {
  logEntered();

  bool result = false;
  return result;
}

bool BadaGraphicsManager::isHotkey(const Common::Event &event) {
  logEntered();

  bool result = false;
  return result;
}

bool BadaGraphicsManager::loadEgl() {
  EGLint numConfigs = 1;
  EGLint eglConfigList[] = {
    EGL_RED_SIZE, 5,
    EGL_GREEN_SIZE, 6,
    EGL_BLUE_SIZE,  5,
    EGL_ALPHA_SIZE, 0,
    EGL_DEPTH_SIZE, 8,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
    EGL_NONE
  };

  EGLint eglContextList[] = {
    EGL_CONTEXT_CLIENT_VERSION, 1, 
    EGL_NONE
  };

  eglBindAPI(EGL_OPENGL_ES_API);

  if (eglDisplay) {
    unloadGFXMode();
  }

  eglDisplay = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
  if (EGL_NO_DISPLAY == eglDisplay) {
    systemError("eglGetDisplay() failed. [0x%x]\n", eglGetError());
    return false;
  }

  if (EGL_FALSE == eglInitialize(eglDisplay, null, null) || 
      EGL_SUCCESS != eglGetError()) {
    systemError("eglInitialize() failed. [0x%x]\n", eglGetError());
    return false;
  }

  if (EGL_FALSE == eglChooseConfig(eglDisplay, eglConfigList, 
                                   &eglConfig, 1, &numConfigs) ||
      EGL_SUCCESS != eglGetError()) {
    systemError("eglChooseConfig() failed. [0x%x]\n", eglGetError());
    return false;
  }

  if (!numConfigs) {
    systemError("eglChooseConfig() failed. Matching config does nott exist \n");
    return false;
  }

  eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, 
                                        (EGLNativeWindowType)appForm, null);

  if (EGL_NO_SURFACE == eglSurface ||
      EGL_SUCCESS != eglGetError()) {
    systemError("eglCreateWindowSurface() failed. EGL_NO_SURFACE [0x%x]\n", 
           eglGetError());
    return false;
  }

  eglContext = eglCreateContext(eglDisplay, eglConfig, 
                                  EGL_NO_CONTEXT, eglContextList);
  if (EGL_NO_CONTEXT == eglContext ||
      EGL_SUCCESS != eglGetError()) {
    systemError("eglCreateContext() failed. [0x%x]\n", eglGetError());
    return false;
  }

  if (false == eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) ||
      EGL_SUCCESS != eglGetError()) {
    systemError("eglMakeCurrent() failed. [0x%x]\n", eglGetError());
    return false;
  }

  return true;

}

bool BadaGraphicsManager::loadGFXMode() {
  logEntered();

  if (!loadEgl()) {
    unloadGFXMode();
    return false;
  }

  int x, y, width, height;
  // TODO
  //  Application::GetInstance()->GetAppFrame()->GetFrame()->GetBounds(x, y, width, height);

  EGLint surfaceType;

  eglGetConfigAttrib(eglDisplay, eglConfig, EGL_SURFACE_TYPE, &surfaceType);

  if ((surfaceType & EGL_PBUFFER_BIT) > 0) {
    EGLint pbuffer_attribs[] = {
      EGL_WIDTH,          getPowerOf2(width),
      EGL_HEIGHT,         getPowerOf2(height),
      EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
      EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGB,
      EGL_NONE
    };

    pBufferSurface = eglCreatePbufferSurface(eglDisplay, eglConfig, pbuffer_attribs);

    if (pBufferSurface != EGL_NO_SURFACE) {
      // TODO apply dimensions to scummvm model
      //eglQuerySurface(eglDisplay, pBufferSurface, EGL_WIDTH, &__pbuffer_width);
      //eglQuerySurface(eglDisplay, pBufferSurface, EGL_HEIGHT, &__pbuffer_height);

      glGenTextures(1, &pBufferTexture);
      glBindTexture(GL_TEXTURE_2D, pBufferTexture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
  }

  return true;
}

bool BadaGraphicsManager::notifyEvent(const Common::Event &event) {
  logEntered();

  bool result = false;
  return result;
}

void BadaGraphicsManager::internUpdateScreen() {
  logEntered();

}

void BadaGraphicsManager::setFeatureState(OSystem::Feature f, bool enable) {
  logEntered();

}

void BadaGraphicsManager::setInternalMousePosition(int x, int y) {
  logEntered();

}

void BadaGraphicsManager::unloadGFXMode() {
  logEntered();

	if (pBufferTexture) {
		glDeleteTextures(1, &pBufferTexture);
		pBufferTexture = 0;
	}

  if (EGL_NO_DISPLAY != eglDisplay) {
    eglMakeCurrent(eglDisplay, null, null, null);

    if (pBufferSurface != EGL_NO_SURFACE) {
      eglDestroySurface(eglDisplay, pBufferSurface);
      pBufferSurface = EGL_NO_SURFACE;
    }

    if (eglContext != EGL_NO_CONTEXT) {
      eglDestroyContext(eglDisplay, eglContext);
      eglContext = EGL_NO_CONTEXT;
    }

    if (eglSurface != EGL_NO_SURFACE) {
      eglDestroySurface(eglDisplay, eglSurface);
      eglSurface = EGL_NO_SURFACE;
    }

    eglTerminate(eglDisplay);
    eglDisplay = EGL_NO_DISPLAY;
  }
  
  eglConfig = null;
}

void BadaGraphicsManager::updateScreen() {
  logEntered();

}


