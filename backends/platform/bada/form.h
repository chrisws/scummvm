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

//
// BadaAppForm
//
class BadaAppForm :
	public Tizen::Ui::Controls::Form,
	public Tizen::Base::Runtime::IRunnable,
	public Tizen::Ui::IOrientationEventListener,
	public Tizen::Ui::ITouchEventListener,
	public Tizen::Ui::IKeyEventListener {
public:
	BadaAppForm();
	~BadaAppForm();

	result Construct();
	bool pollEvent(Common::Event &event);
	bool isClosing() { return _state == kClosingState; }
	void pushKey(Common::KeyCode keycode);
	void exitSystem();

private:
	Tizen::Base::Object *Run();
	result OnInitializing(void);
	result OnDraw(void);
	void OnOrientationChanged(const Tizen::Ui::Control &source,
			Tizen::Ui::OrientationStatus orientationStatus);
	void OnTouchDoublePressed(const Tizen::Ui::Control &source,
			const Tizen::Graphics::Point &currentPosition,
			const Tizen::Ui::TouchEventInfo &touchInfo);
	void OnTouchFocusIn(const Tizen::Ui::Control &source,
			const Tizen::Graphics::Point &currentPosition,
			const Tizen::Ui::TouchEventInfo &touchInfo);
	void OnTouchFocusOut(const Tizen::Ui::Control &source,
			const Tizen::Graphics::Point &currentPosition,
			const Tizen::Ui::TouchEventInfo &touchInfo);
	void OnTouchLongPressed(const Tizen::Ui::Control &source,
			const Tizen::Graphics::Point &currentPosition,
			const Tizen::Ui::TouchEventInfo &touchInfo);
	void OnTouchMoved(const Tizen::Ui::Control &source,
			const Tizen::Graphics::Point &currentPosition,
			const Tizen::Ui::TouchEventInfo &touchInfo);
	void OnTouchPressed(const Tizen::Ui::Control &source,
			const Tizen::Graphics::Point &currentPosition,
			const Tizen::Ui::TouchEventInfo &touchInfo);
	void OnTouchReleased(const Tizen::Ui::Control &source,
			const Tizen::Graphics::Point &currentPosition,
			const Tizen::Ui::TouchEventInfo &touchInfo);
	void OnKeyLongPressed(const Tizen::Ui::Control &source,	Tizen::Ui::KeyCode keyCode);
	void OnKeyPressed(const Tizen::Ui::Control &source,	Tizen::Ui::KeyCode keyCode);
	void OnKeyReleased(const Tizen::Ui::Control &source, Tizen::Ui::KeyCode keyCode);

	void pushEvent(Common::EventType type, const Tizen::Graphics::Point &currentPosition);
	void terminate();
	void setButtonShortcut();
	void setMessage(const char *message);
	void setShortcut();
	void setVolume(bool up, bool minMax);
	void showKeypad();
	void showLevel(int level);
	void invokeShortcut();
	bool gameActive() { return _state == kActiveState && g_engine != NULL && !g_engine->isPaused(); }

	// event handling
	const char *_osdMessage;
	Tizen::Base::Runtime::Thread *_gameThread;
	Tizen::Base::Runtime::Mutex *_eventQueueLock;
	Common::Queue<Common::Event> _eventQueue;
	enum { kInitState, kActiveState, kClosingState, kDoneState, kErrorState } _state;
	enum { kLeftButton, kRightButtonOnce, kRightButton, kMoveOnly } _buttonState;
	enum { kControlMouse, kEscapeKey, kGameMenu, kShowKeypad, kSetVolume } _shortcut;
};

#endif
