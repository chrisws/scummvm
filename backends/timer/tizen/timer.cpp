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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(TIZEN)

#include "backends/timer/tizen/timer.h"
#include "dlog/dlog.h"

Eina_Bool timerCallback(void *data) {
	TimerSlot *slot = (TimerSlot *)data;
	slot->timerExpired();
	return ECORE_CALLBACK_RENEW;
}

//
// TimerSlot - an event driven thread
//
TimerSlot::TimerSlot(Common::TimerManager::TimerProc callback, uint32 interval, void *refCon) :
	_timer(0),
	_callback(callback),
	_interval(interval),
	_refCon(refCon) {
}

TimerSlot::~TimerSlot() {
	if (_timer) {
		LOGD("timer stopped");
		ecore_timer_del(_timer);
		_timer = NULL;
	}
}

bool TimerSlot::start() {
	_timer = ecore_timer_add(_interval, timerCallback, this);
	if (_timer != NULL) {
		LOGD("started timer %d", _interval);
	}
	return _timer != NULL;
}

void TimerSlot::timerExpired() {
	_callback(_refCon);
}

//
// TizenTimerManager
//
TizenTimerManager::TizenTimerManager() {
}

TizenTimerManager::~TizenTimerManager() {
	for (Common::List<TimerSlot *>::iterator it = _timers.begin(); it != _timers.end(); ) {
		TimerSlot *slot = (*it);
		delete slot;
		it = _timers.erase(it);
	}
}

bool TizenTimerManager::installTimerProc(TimerProc proc, int32 interval, void *refCon, const Common::String &id) {
	TimerSlot *slot = new TimerSlot(proc, interval / 1000, refCon);
	if (!slot || !slot->start()) {
		delete slot;
		LOGD("Failed to start timer thread");
		return false;
	}

	_timers.push_back(slot);
	return true;
}

void TizenTimerManager::removeTimerProc(TimerProc proc) {
	for (Common::List<TimerSlot *>::iterator it = _timers.begin(); it != _timers.end(); ++it) {
		TimerSlot *slot = (*it);
		if (slot->_callback == proc) {
			delete slot;
			it = _timers.erase(it);
		}
	}
}

#endif
