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

#ifndef BADA_APPLICATION_H
#define BADA_APPLICATION_H

#include <FBase.h>
#include <FApp.h>
#include <FGraphics.h>
#include <FUi.h>
#include <FSystem.h>

#include "backends/platform/bada/system.h"
#include "backends/platform/bada/form.h"

using namespace Tizen::App;
using namespace Tizen::System;
using namespace Tizen::Ui;
using namespace Tizen::Ui::Controls;
using namespace Tizen::Base::Collection;

class BadaScummVM :
	public UiApp,
	public IScreenEventListener {

public:
	BadaScummVM();
	~BadaScummVM();

	static UiApp *createInstance(void);

	virtual bool OnAppInitializing(AppRegistry &appRegistry);
	virtual bool OnAppInitialized(void);
	virtual bool OnAppWillTerminate(void);
	virtual bool OnAppTerminating(AppRegistry &appRegistry, bool forcedTermination = false);
	virtual void OnLowMemory(void);
	virtual void OnBatteryLevelChanged(BatteryLevel batteryLevel);
	virtual void OnUserEventReceivedN(RequestId requestId, IList *pArgs);
	virtual void OnForeground(void);
	virtual void OnBackground(void);
	virtual void OnScreenOn(void);
	virtual void OnScreenOff(void);
	virtual void OnScreenBrightnessChanged(int brightness);

private:
	void pauseGame(bool pause);
	BadaAppForm *_appForm;
};

#endif
