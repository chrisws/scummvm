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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <FApp.h>
#include <FSysSystemTime.h>

#include "common/translation.h"
#include "base/main.h"

#include "backends/platform/bada/form.h"
#include "backends/platform/bada/system.h"

using namespace Tizen::Base::Collection;
using namespace Tizen::Base::Runtime;
using namespace Tizen::Ui::Controls;

// round down small Y touch values to 1 to allow the
// cursor to be positioned at the top of the screen
#define MIN_TOUCH_Y 20

// block for up to 2.5 seconds during shutdown to
// allow the game thread to exit gracefully.
#define EXIT_SLEEP_STEP 10
#define EXIT_SLEEP 250

//
// BadaAppForm
//
BadaAppForm::BadaAppForm() :
	_gestureMode(false),
	_osdMessage(NULL),
	_gameThread(NULL),
	_eventQueueLock(NULL),
	_state(kInitState),
	_buttonState(kLeftButton),
	_shortcut(kShowKeypad) {
}

result BadaAppForm::Construct() {
	BadaSystem *badaSystem = NULL;
	result r = Form::Construct(FORM_STYLE_NORMAL);
	if (!IsFailed(r)) {
		badaSystem = new BadaSystem(this);
		r = badaSystem != NULL ? E_SUCCESS : E_OUT_OF_MEMORY;
	}
	if (!IsFailed(r)) {
		r = badaSystem->Construct();
	}
	if (!IsFailed(r)) {
		_gameThread = new Thread();
		r = _gameThread != NULL ? E_SUCCESS : E_OUT_OF_MEMORY;
	}
	if (!IsFailed(r)) {
		r = _gameThread->Construct(*this);
	}
	if (!IsFailed(r)) {
		_eventQueueLock = new Mutex();
		r = _eventQueueLock != NULL ? E_SUCCESS : E_OUT_OF_MEMORY;
	}
	if (!IsFailed(r)) {
		r = _eventQueueLock->Create();
	}

	if (!IsFailed(r)) {
		g_system = badaSystem;
	} else {
		AppLog("Form startup failed");
		delete badaSystem;
		delete _gameThread;
		_gameThread = NULL;
	}
	return r;
}

BadaAppForm::~BadaAppForm() {
	logEntered();

	if (_gameThread && _state != kErrorState) {
		terminate();

		_gameThread->Stop();
		if (_state != kErrorState) {
			_gameThread->Join();
		}

		delete _gameThread;
		_gameThread = NULL;
	}

	delete _eventQueueLock;
	_eventQueueLock = NULL;

	logLeaving();
}

//
// abort the game thread
//
void BadaAppForm::terminate() {
	if (_state == kActiveState) {
		((BadaSystem *)g_system)->setMute(true);

		_eventQueueLock->Acquire();

		Common::Event e;
		e.type = Common::EVENT_QUIT;
		_eventQueue.push(e);
		_state = kClosingState;

		_eventQueueLock->Release();

		// block while thread ends
		AppLog("waiting for shutdown");
		for (int i = 0; i < EXIT_SLEEP_STEP && _state == kClosingState; i++) {
			Thread::Sleep(EXIT_SLEEP);
		}

		if (_state == kClosingState) {
			// failed to terminate - Join() will freeze
			_state = kErrorState;
		}
	}
}

void BadaAppForm::exitSystem() {
	_state = kErrorState;

	if (_gameThread) {
		_gameThread->Stop();
		delete _gameThread;
		_gameThread = NULL;
	}
}

result BadaAppForm::OnInitializing(void) {
	logEntered();

	AddOrientationEventListener(*this);
	AddTouchEventListener(*this);
	SetMultipointTouchEnabled(true);

	// set focus to enable receiving key events
	SetEnabled(true);
	SetFocusable(true);
	SetFocus();

	return E_SUCCESS;
}

result BadaAppForm::OnDraw(void) {
	logEntered();
	return E_SUCCESS;
}

void BadaAppForm::OnOrientationChanged(const Control &source, OrientationStatus orientationStatus) {
	logEntered();
	if (_state == kInitState) {
		_state = kActiveState;
		_gameThread->Start();
	}
}

Tizen::Base::Object *BadaAppForm::Run() {
	logEntered();

	scummvm_main(0, 0);
	if (_state == kActiveState) {
		Tizen::App::Application::GetInstance()->SendUserEvent(USER_MESSAGE_EXIT, NULL);
	}
	_state = kDoneState;
	return NULL;
}

bool BadaAppForm::pollEvent(Common::Event &event) {
	bool result = false;

	_eventQueueLock->Acquire();
	if (!_eventQueue.empty()) {
		event = _eventQueue.pop();
		result = true;
	}
	if (_osdMessage) {
		BadaSystem *system = (BadaSystem *)g_system;
		BadaGraphicsManager *graphics = system->getGraphics();
		if (graphics) {
			graphics->displayMessageOnOSD(_osdMessage);
			_osdMessage = NULL;
		}
	}
	_eventQueueLock->Release();

	return result;
}

void BadaAppForm::pushEvent(Common::EventType type, const Point &currentPosition) {
	BadaSystem *system = (BadaSystem *)g_system;
	BadaGraphicsManager *graphics = system->getGraphics();
	if (graphics) {
		// graphics could be NULL at startup or when
		// displaying the system error screen
		Common::Event e;
		e.type = type;
		e.mouse.x = currentPosition.x;
		e.mouse.y = currentPosition.y > MIN_TOUCH_Y ? currentPosition.y : 1;

		bool moved = graphics->moveMouse(e.mouse.x, e.mouse.y);

		_eventQueueLock->Acquire();

		if (moved && type != Common::EVENT_MOUSEMOVE) {
			Common::Event moveEvent;
			moveEvent.type = Common::EVENT_MOUSEMOVE;
			moveEvent.mouse = e.mouse;
			_eventQueue.push(moveEvent);
		}

		_eventQueue.push(e);
		_eventQueueLock->Release();
	}
}

void BadaAppForm::pushKey(Common::KeyCode keycode) {
	if (_eventQueueLock) {
		Common::Event e;
		e.synthetic = false;
		e.kbd.keycode = keycode;
		e.kbd.ascii = keycode;
		e.kbd.flags = 0;

		_eventQueueLock->Acquire();
		e.type = Common::EVENT_KEYDOWN;
		_eventQueue.push(e);
		e.type = Common::EVENT_KEYUP;
		_eventQueue.push(e);
		_eventQueueLock->Release();
	}
}

void BadaAppForm::setButtonShortcut() {
	switch (_buttonState) {
	case kLeftButton:
		setMessage(_s("Right Click Once"));
		_buttonState = kRightButtonOnce;
		break;
	case kRightButtonOnce:
		setMessage(_s("Right Click"));
		_buttonState = kRightButton;
		break;
	case kRightButton:
		setMessage(_s("Move Only"));
		_buttonState = kMoveOnly;
		break;
	case kMoveOnly:
		setMessage(_s("Left Click"));
		_buttonState = kLeftButton;
		break;
	}
}

void BadaAppForm::setMessage(const char *message) {
	if (_eventQueueLock) {
		_eventQueueLock->Acquire();
		_osdMessage = message;
		_eventQueueLock->Release();
	}
}

void BadaAppForm::setShortcut() {
	logEntered();
	// cycle to the next shortcut
	switch (_shortcut) {
	case kControlMouse:
		setMessage(_s("Escape Key"));
		_shortcut = kEscapeKey;
		break;

	case kEscapeKey:
		setMessage(_s("Game Menu"));
		_shortcut = kGameMenu;
		break;

	case kGameMenu:
		setMessage(_s("Show Keypad"));
		_shortcut = kShowKeypad;
		break;

	case kShowKeypad:
		setMessage(_s("Control Mouse"));
		_shortcut = kControlMouse;
		break;
	}
}

void BadaAppForm::invokeShortcut() {
	logEntered();
	switch (_shortcut) {
	case kControlMouse:
		setButtonShortcut();
		break;
		
	case kEscapeKey:
		pushKey(Common::KEYCODE_ESCAPE);
		break;
		
	case kGameMenu:
		_buttonState = kLeftButton;
		pushKey(Common::KEYCODE_F5);
		break;
		
	case kShowKeypad:
		showKeypad();
		break;
  }
}

void BadaAppForm::showKeypad() {
	// display the soft keyboard
	_buttonState = kLeftButton;
	pushKey(Common::KEYCODE_F7);
}

int BadaAppForm::getTouchCount() {
	Tizen::Ui::TouchEventManager *touch = Tizen::Ui::TouchEventManager::GetInstance();
	IListT<TouchEventInfo *> *touchList = touch->GetTouchInfoListN();
	int touchCount = touchList->GetCount();
	touchList->RemoveAll();
	delete touchList;
	return touchCount;
}

void BadaAppForm::OnTouchDoublePressed(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
	if (_buttonState != kMoveOnly) {
		pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
							currentPosition);
		pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
							currentPosition);
	}
}

void BadaAppForm::OnTouchFocusIn(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
}

void BadaAppForm::OnTouchFocusOut(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
}

void BadaAppForm::OnTouchLongPressed(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
	logEntered();
	if (_buttonState != kLeftButton) {
		pushKey(Common::KEYCODE_RETURN);
	}
}

void BadaAppForm::OnTouchMoved(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
	if (!_gestureMode) {
	  pushEvent(Common::EVENT_MOUSEMOVE, currentPosition);
	}
}

void BadaAppForm::OnTouchPressed(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
	if (getTouchCount() > 1) {
		_gestureMode = true;
	} else if (_buttonState != kMoveOnly) {
		pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONDOWN : Common::EVENT_RBUTTONDOWN,
							currentPosition);
	}
}

void BadaAppForm::OnTouchReleased(const Control &source,
		const Point &currentPosition, const TouchEventInfo &touchInfo) {
	if (_gestureMode) {
		int touchCount = getTouchCount();
		if (touchCount == 1) {
			setShortcut();
		} else {
			if (touchCount == 2) {
				invokeShortcut();
			}
			_gestureMode = false;
		}
	} else if (_buttonState != kMoveOnly) {
		pushEvent(_buttonState == kLeftButton ? Common::EVENT_LBUTTONUP : Common::EVENT_RBUTTONUP,
							currentPosition);
		if (_buttonState == kRightButtonOnce) {
			_buttonState = kLeftButton;
		}
		// flick to skip dialog
		if (touchInfo.IsFlicked()) {
			pushKey(Common::KEYCODE_PERIOD);
		}
	}
}

