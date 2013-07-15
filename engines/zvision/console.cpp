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

#include "common/scummsys.h"

#include "gui/debugger.h"
#include "common/file.h"
#include "common/bufferedstream.h"
#include "audio/mixer.h"

#include "zvision/console.h"
#include "zvision/zvision.h"
#include "zvision/zork_avi_decoder.h"
#include "zvision/zork_raw.h"

namespace ZVision {

Console::Console(ZVision *engine) : GUI::Debugger(), _engine(engine) {
	DCmd_Register("loadimage", WRAP_METHOD(Console, cmdLoadImage));
	DCmd_Register("loadvideo", WRAP_METHOD(Console, cmdLoadVideo));
	DCmd_Register("loadsound", WRAP_METHOD(Console, cmdLoadSound));
}

bool Console::cmdLoadImage(int argc, const char **argv) {
	if (argc != 4) {
		DebugPrintf("Use loadimage <fileName> <x> <y> to load an image to the screen");
		return true;
	}
	_engine->renderImageToScreen(argv[1], atoi(argv[2]), atoi(argv[3]));

	return true;
}

bool Console::cmdLoadVideo(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Use loadvideo <fileName> to load a video to the screen");
		return true;
	}

	Video::VideoDecoder *videoDecoder = new ZorkAVIDecoder();
	if (videoDecoder && videoDecoder->loadFile(argv[1])) {
		_engine->startVideo(videoDecoder);
	}

	return true;
}

bool Console::cmdLoadSound(int argc, const char **argv) {
	if (argc != 4) {
		DebugPrintf("Use loadsound <fileName> <rate> <stereo: 0 or 1> to load a video to the screen");
		return true;
	}

	Common::File *file = new Common::File();
	if (!file->open(argv[1])) {
		DebugPrintf("File does not exist");
		return true;
	}

	Audio::AudioStream *soundStream = makeRawZorkStream(wrapBufferedSeekableReadStream(file, 2048, DisposeAfterUse::YES), atoi(argv[2]), atoi(argv[3]), DisposeAfterUse::YES);
	Audio::SoundHandle handle;
	_engine->getMixer()->playStream(Audio::Mixer::kPlainSoundType, &handle, soundStream, -1, 100, 0, DisposeAfterUse::YES, false, false);

	return true;
}

} // End of namespace ZVision
