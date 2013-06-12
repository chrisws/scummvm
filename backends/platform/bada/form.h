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

#ifndef BADA_FORM_H
#define BADA_FORM_H

#include <FApp.h>
#include <FUi.h>
#include <FSystem.h>
#include <FBase.h>
#include <FUiITouchEventListener.h>
#include <FUiITextEventListener.h>

#include "config.h"
#include "common/scummsys.h"
#include "common/events.h"
#include "common/queue.h"
#include "common/mutex.h"
#include "engines/engine.h"

using namespace Tizen::Ui;
using namespace Tizen::Graphics;

//
// BadaAppForm
//
class BadaAppForm :
	public Controls::Form,
	public Tizen::Base::Runtime::IRunnable,
	public IOrientationEventListener,
	public ITouchEventListener,
	public IPropagatedKeyEventListener {

public:
	BadaAppForm();
	virtual ~BadaAppForm();

	result Construct();
	bool pollEvent(Common::Event &event);
	bool isClosing() { return _state == kClosingState; }
	void pushKey(Common::KeyCode keycode);
	void exitSystem();

private:
	Tizen::Base::Object *Run();
	result OnInitializing(void);
	result OnDraw(void);
	void OnOrientationChanged(const Control &source,
			OrientationStatus orientationStatus);
	void OnTouchDoublePressed(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchFocusIn(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchFocusOut(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchLongPressed(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchMoved(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchPressed(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	void OnTouchReleased(const Control &source,
			const Point &currentPosition,
			const TouchEventInfo &touchInfo);
	bool OnKeyPressed(Control &source, const KeyEventInfo &keyEventInfo);
	bool OnKeyReleased(Control &source, const KeyEventInfo &keyEventInfo);
	bool OnPreviewKeyPressed(Control &source, const KeyEventInfo &keyEventInfo);
	bool OnPreviewKeyReleased(Control &source, const KeyEventInfo &keyEventInfo);

	void pushEvent(Common::EventType type, const Point &currentPosition);
	void terminate();
	void setButtonShortcut();
	void setMessage(const char *message);
	void setShortcut();
	void setVolume(bool up, bool minMax);
	void showKeypad();
	void showLevel(int level);
	void invokeShortcut();
	int  getTouchCount();
	bool gameActive() { return _state == kActiveState && g_engine != NULL && !g_engine->isPaused(); }

	// event handling
	bool _gestureMode;
	const char *_osdMessage;
	Tizen::Base::Runtime::Thread *_gameThread;
	Tizen::Base::Runtime::Mutex *_eventQueueLock;
	Common::Queue<Common::Event> _eventQueue;
	enum { kInitState, kActiveState, kClosingState, kDoneState, kErrorState } _state;
	enum { kLeftButton, kRightButtonOnce, kRightButton, kMoveOnly } _buttonState;
	enum { kControlMouse, kEscapeKey, kGameMenu, kShowKeypad, kSetVolume } _shortcut;
};

#endif
