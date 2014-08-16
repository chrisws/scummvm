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

#ifndef ACCESS_ROOM_H
#define ACCESS_ROOM_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "access/data.h"

#define TILE_WIDTH 16
#define TILE_HEIGHT 16

namespace Access {

class Plotter {
public:
	Common::Array<Common::Rect> _walls;
	Common::Array<Common::Rect> _blocks;
	int _blockIn;
	int _delta;
public:
	Plotter();

	void load(Common::SeekableReadStream *stream, int wallCount, int blockCount);
};

class JetFrame {
public:
	int _wallCode;
	int _wallCodeOld;
	int _wallCode1;
	int _wallCode1Old;

	JetFrame() {
		_wallCode = _wallCodeOld = 0;
		_wallCode1 = _wallCode1Old = 0;
	}
};

class Room: public Manager {
private:
	void roomLoop();

	void loadPlayField(int fileNum, int subfile);

	void commandOff();
protected:
	void loadRoomData(const byte *roomData);
	void setupRoom();
	void setWallCodes();
	void buildScreen();

	/**
	* Free the playfield data
	*/
	void freePlayField();

	/**
	* Free tile data
	*/
	void freeTileData();

	/**
	* Switch to a given command mode
	*/
	void handleCommand(int commandId);

	/**
	 * Inner handler for switching to a given command mode
	 */
	void executeCommand(int commandId);

	virtual void loadRoom(int roomNumber) = 0;

	virtual void reloadRoom() = 0;

	virtual void reloadRoom1() = 0;

	virtual void setIconPalette() {}

	virtual void doCommands();

	virtual void roomMenu() = 0;

	virtual void mainAreaClick() = 0;
public:
	Plotter _plotter;
	Common::Array<JetFrame> _jetFrame;
	int _function;
	int _roomFlag;
	byte *_playField;
	int _matrixSize;
	int _playFieldWidth;
	int _playFieldHeight;
	byte *_tile;
	int _tileSize;
	int _selectCommand;
	bool _conFlag;
public:
	Room(AccessEngine *vm);

	virtual ~Room();

	void doRoom();

	/**
	 * Clear all the data used by the room
	 */
	void clearRoom();

	void buildColumn(int playX, int screenX);

	void buildRow(int playY, int screenY);

	void init4Quads();
};


class RoomInfo {
public:
	struct FileIdent {
		int _fileNum;
		int _subfile;
	};

	struct CellIdent : FileIdent {
		byte _cell;
	};
	
	struct SoundIdent : FileIdent {
		int _priority;
	};
public:
	int _roomFlag;
	int _estIndex;
	FileIdent _musicFile;
	int _scaleH1;
	int _scaleH2;
	int _scaleN1;
	FileIdent _playFieldFile;
	Common::Array<CellIdent> _cells;
	FileIdent _scriptFile;
	FileIdent _animFile;
	int _scaleI;
	int _scrollThreshold;
	FileIdent _paletteFile;
	int _startColor;
	int _numColors;
	Common::Array<uint32> _vidTable;
	Common::Array<SoundIdent> _sounds;
public:
	RoomInfo(const byte *data);
};

} // End of namespace Access

#endif /* ACCESS_ROOM_H */
