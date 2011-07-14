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

#include <FAppApplication.h>

#include "form.h"
#include "system.h"

using namespace Osp::App;
using namespace Osp::Base;
using namespace Osp::Base::Runtime;
using namespace Osp::Graphics;
using namespace Osp::Ui;
using namespace Osp::Ui::Controls;

#define SETUP_TIMEOUT 4000
#define SETUP_KEYPAD  0
#define SETUP_MOUSE   1
#define SETUP_VOLUME  2

//
// BadaAppForm
//
BadaAppForm::BadaAppForm() : 
  gameThread(0), 
  state(InitState),
  setupTimer(0),
  leftButton(true) {
  eventQueueLock = new Mutex();
  eventQueueLock->Create();
}

result BadaAppForm::Construct() {
  result r = Form::Construct(Controls::FORM_STYLE_NORMAL);
  if (IsFailed(r)) {
    return r;
  }

  BadaSystem* badaSystem = null;
  gameThread = null;

  badaSystem = new BadaSystem(this);
  r = badaSystem != null ? E_SUCCESS : E_OUT_OF_MEMORY;
  
  if (!IsFailed(r)) {
    r = badaSystem->Construct();
  }

  if (!IsFailed(r)) {
    gameThread = new Thread();
    r = gameThread != null ? E_SUCCESS : E_OUT_OF_MEMORY;
  }

  if (!IsFailed(r)) {
    r = gameThread->Construct(*this);
  }

  if (IsFailed(r)) {
    if (badaSystem != null) {
      delete badaSystem;
    }
    if (gameThread != null) {
      delete gameThread;
      gameThread = null;
    }
  }
  else {
    g_system = badaSystem;
  }

  return r;
}

BadaAppForm::~BadaAppForm() {
  logEntered();

  if (gameThread) {
    terminate();
    gameThread->Stop();
    gameThread->Join();
    delete gameThread;
    gameThread = null;
  }

  if (eventQueueLock) {
    delete eventQueueLock;
    eventQueueLock = null;
  }

  logLeaving();
}

//
// abort the game thread
//
void BadaAppForm::terminate() {
  if (state == ActiveState) {
    eventQueueLock->Acquire();

    Common::Event e;
    e.type = Common::EVENT_QUIT;
    eventQueue.push(e);
    state = ClosingState;

    eventQueueLock->Release();
    
    // block while thread ends
    AppLog("waiting for shutdown");
    for (int i = 0; i < 10 && state == ClosingState; i++) {
      Thread::Sleep(250);
    }
  }
}

result BadaAppForm::OnInitializing(void) {
  logEntered();

  SetOrientation(ORIENTATION_LANDSCAPE);
  AddOrientationEventListener(*this);
  AddTouchEventListener(*this);
  AddKeyEventListener(*this);

  // set focus to enable receiving key events
  SetFocusable(true);
  SetFocus();

  return E_SUCCESS;
}

result BadaAppForm::OnDraw(void) {
  logEntered();

  if (g_system) {
    BadaSystem* system = (BadaSystem*) g_system;
    BadaGraphicsManager* graphics = system->getGraphics();
    if (graphics && graphics->isReady()) {
      g_system->updateScreen();
    }
  }

  return E_SUCCESS;
}

bool BadaAppForm::pollEvent(Common::Event& event) {
  bool result = false;

  eventQueueLock->Acquire();
  if (!eventQueue.empty()) {
    event = eventQueue.pop();
    result = true;
  }
  eventQueueLock->Release();

  return result;
}

void BadaAppForm::pushEvent(Common::EventType type,
                            const Point& currentPosition) {
  Common::Event e;
  e.type = type;
  e.mouse.x = currentPosition.x;
  e.mouse.y = currentPosition.y;
  g_system->warpMouse(currentPosition.x, currentPosition.y);

  eventQueueLock->Acquire();
  eventQueue.push(e);
  eventQueueLock->Release();
}

void BadaAppForm::pushKey(Common::KeyCode keycode) {
  Common::Event e;
  e.type = Common::EVENT_KEYDOWN;
  e.kbd.keycode = keycode;

  eventQueueLock->Acquire();
  eventQueue.push(e);
  eventQueueLock->Release();
}

void BadaAppForm::OnOrientationChanged(const Control& source, 
                                       OrientationStatus orientationStatus) {
  logEntered();
  if (state == InitState) {
    state = ActiveState;
    gameThread->Start();
  }
}

Object* BadaAppForm::Run(void) {
  scummvm_main(0, 0);

  AppLog("scummvm_main completed");

  if (state == ActiveState) {
    Application::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT, null);
  }
  state = DoneState;
  return null;
}

bool BadaAppForm::isEscapeMode() {
  bool result = false;
  if (setupTimer) {
    uint32 nextTimer = g_system->getMillis();
    if (setupTimer + SETUP_TIMEOUT < nextTimer) {
      setupTimer = 0;
    }
  }
  return setupTimer != 0;
}

void BadaAppForm::OnTouchDoublePressed(const Control& source, 
                                       const Point& currentPosition, 
                                       const TouchEventInfo& touchInfo) {
  setupTimer = g_system->getMillis();
  g_system->displayMessageOnOSD("ALT Keys Active");
}

void BadaAppForm::OnTouchFocusIn(const Control& source, 
                                 const Point& currentPosition, 
                                 const TouchEventInfo& touchInfo) {
}

void BadaAppForm::OnTouchFocusOut(const Control& source, 
                                  const Point& currentPosition, 
                                  const TouchEventInfo& touchInfo) {
}

void BadaAppForm::OnTouchLongPressed(const Control& source, 
                                     const Point& currentPosition, 
                                     const TouchEventInfo& touchInfo) {
  pushKey(Common::KEYCODE_RETURN);
}

void BadaAppForm::OnTouchMoved(const Control& source, 
                               const Point& currentPosition, 
                               const TouchEventInfo& touchInfo) {
  pushEvent(Common::EVENT_MOUSEMOVE, currentPosition);
}

void BadaAppForm::OnTouchPressed(const Control& source, 
                                 const Point& currentPosition, 
                                 const TouchEventInfo& touchInfo) {
  pushEvent(leftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
            currentPosition);
}

void BadaAppForm::OnTouchReleased(const Control& source, 
                                  const Point& currentPosition, 
                                  const TouchEventInfo& touchInfo) {
  if (isEscapeMode() && touchInfo.IsFlicked()) {
    pushKey(Common::KEYCODE_F7);
  }
  else {
    pushEvent(leftButton ? Common::EVENT_LBUTTONUP : Common::EVENT_RBUTTONUP,
              currentPosition);
  }
}

void BadaAppForm::OnKeyLongPressed(const Control& source, KeyCode keyCode) {
  logEntered();
  switch (keyCode) {
  case KEY_SWITCH:
    pushKey(Common::KEYCODE_F7);
    return;

  case KEY_CAMERA:
    pushKey(Common::KEYCODE_ESCAPE);
    return;

  default:
    break;
  }
}

void BadaAppForm::OnKeyPressed(const Control& source, KeyCode keyCode) {
  if (isEscapeMode()) {
    setupTimer = g_system->getMillis();
  }

  switch (keyCode) {
  case KEY_SIDE_UP:
    if (isEscapeMode()) {
      g_system->displayMessageOnOSD("Volume Up");
      ((BadaSystem*) g_system)->setVolume(true);
    }
    else {
      g_system->displayMessageOnOSD("Left Button Active");
      leftButton = true;
    }
    return;

  case KEY_SIDE_DOWN:
    if (isEscapeMode()) {
      g_system->displayMessageOnOSD("Volume Down");
      ((BadaSystem*) g_system)->setVolume(false);
    }
    else {
      g_system->displayMessageOnOSD("Right Button Active");
      leftButton = false;
    }
    return;

  case KEY_CAMERA:
    if (isEscapeMode()) {
      pushKey(Common::KEYCODE_F5);
    }
    else {
      pushKey(Common::KEYCODE_F7);
    }
    return;

  default:
    break;
  }
}

void BadaAppForm::OnKeyReleased(const Control& source, KeyCode keyCode) {
  logEntered();
}

//
// end of form.cpp 
//
