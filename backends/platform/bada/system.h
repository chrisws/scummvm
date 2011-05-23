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

#ifndef BADA_SYSTEM_H
#define BADA_SYSTEM_H

#include <FApp.h>
#include <FGraphics.h>
#include <FUi.h>
#include <FSystem.h>
#include <FBase.h>
#include <FIoFile.h>

#if defined(_DEBUG)
#define logEntered() AppLog("%s entered (%s %d)", \
                             __FUNCTION__, __FILE__, __LINE__);
#else
#define logEntered()
#endif

#if defined(_DEBUG)
#define logLeaving() AppLog("%s leaving (%s %d)", \
                             __FUNCTION__, __FILE__, __LINE__);
#else
#define logLeaving()
#endif

struct BadaAppForm : public Osp::Ui::Controls::Form,
                     public Osp::Base::Runtime::IRunnable {
  BadaAppForm() {}
  ~BadaAppForm();

  result Construct();
  Object* Run();
  result OnDraw(void);

  Osp::Base::Runtime::Thread* pThread;
};

BadaAppForm* systemStart(Osp::App::Application* app);
void systemStop(BadaAppForm* appForm);
void systemPostEvent();
void systemError(const char* format, ...);

#endif
