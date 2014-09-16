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
#include "access/access.h"
#include "access/amazon/amazon_game.h"
#include "access/amazon/amazon_resources.h"
#include "access/amazon/amazon_scripts.h"

namespace Access {

namespace Amazon {

AmazonScripts::AmazonScripts(AccessEngine *vm) : Scripts(vm) {
	_game = (AmazonEngine *)_vm;
}

void AmazonScripts::cLoop() {
	searchForSequence();
	_vm->_images.clear();
	_vm->_buffer2.copyFrom(_vm->_buffer1);
	_vm->_oldRects.clear();
	_vm->_scripts->executeScript();
	_vm->plotList1();
	_vm->copyBlocks();
}

void AmazonScripts::mWhile1() {
	_vm->_screen->setDisplayScan();
	_vm->_screen->fadeOut();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 0);
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();
	
	Resource *spriteData = _vm->_files->loadFile(14, 6);
	_vm->_objectsTable[0] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2100;

	do {
		cLoop();
		_sequence = 2100;
	} while (_vm->_flags[52] == 1);

	_vm->_screen->copyFrom(_vm->_buffer1);
	_vm->_buffer1.copyFrom(_vm->_buffer2);

	_game->establish(-1, 14);

	spriteData = _vm->_files->loadFile(14, 7);
	_vm->_objectsTable[1] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_sound->playSound(0);
	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 1);
	_vm->_screen->setPalette();
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2200;

	_vm->_sound->queueSound(0, 14, 15);
	_vm->_sound->_soundPriority[0] = 1;

	do {
		cLoop();
		_sequence = 2200;
	} while (_vm->_flags[52] == 2);

	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 2);
	_vm->_screen->setPalette();
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->freeCells();

	spriteData = _vm->_files->loadFile(14, 8);
	_vm->_objectsTable[2] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2300;
	_vm->_sound->playSound(0);

	do {
		cLoop();
		_sequence = 2300;
	} while (_vm->_flags[52] == 3);

	_vm->freeCells();
	spriteData = _vm->_files->loadFile(14, 9);
	_vm->_objectsTable[3] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 3);
	_vm->_screen->setPalette();
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2400;

	do {
		cLoop();
		_sequence = 2400;
	} while (_vm->_flags[52] == 4);
}

void AmazonScripts::mWhile2() {
	_vm->_screen->setDisplayScan();
	_vm->_screen->fadeOut();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 0);
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_screen->forceFadeIn();

	Resource *spriteData = _vm->_files->loadFile(14, 6);
	_vm->_objectsTable[0] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2100;

	do {
		cLoop();
		_sequence = 2100;
	} while (_vm->_flags[52] == 1);

	_vm->_screen->fadeOut();
	_vm->freeCells();
	spriteData = _vm->_files->loadFile(14, 9);
	_vm->_objectsTable[3] = new SpriteResource(_vm, spriteData);
	delete spriteData;

	_vm->_screen->setDisplayScan();
	_vm->_events->hideCursor();

	_vm->_files->loadScreen(14, 3);
	_vm->_screen->setPalette();
	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);
	_vm->_events->showCursor();

	_vm->_screen->setIconPalette();
	_vm->_images.clear();
	_vm->_oldRects.clear();
	_sequence = 2400;

	do {
		cLoop();
		_sequence = 2400;
	} while (_vm->_flags[52] == 4);
}

void AmazonScripts::mWhile(int param1) {
	switch(param1) {
	case 1:
		mWhile1();
		break;
	case 2:
		warning("TODO FLY");
		break;
	case 3:
		warning("TODO FALL");
		break;
	case 4:
		warning("TODO JWALK");
		break;
	case 5:
		warning("TODO DOOPEN");
		break;
	case 6:
		warning("TODO DOWNRIVER");
		break;
	case 7:
		mWhile2();
		break;
	case 8:
		warning("TODO JWALK2");
		break;
	default:
		break;
	}
}

void AmazonScripts::loadBackground(int param1, int param2) {
	_vm->_files->_loadPalFlag = false;
	_vm->_files->loadScreen(param1, param2);

	_vm->_buffer2.copyFrom(*_vm->_screen);
	_vm->_buffer1.copyFrom(*_vm->_screen);

	_vm->_screen->forceFadeOut();
	
}

void AmazonScripts::setInactive() {
	_game->_rawInactiveX = _vm->_player->_rawPlayer.x;
	_game->_rawInactiveY = _vm->_player->_rawPlayer.y;
	_game->_charSegSwitch = false;

	mWhile(_game->_rawInactiveY);
}

void AmazonScripts::boatWalls(int param1, int param2) {
	if (param1 == 1)
		_vm->_room->_plotter._walls[42] = Common::Rect(96, 27, 87, 42);
	else {
		_vm->_room->_plotter._walls[39].bottom = _vm->_room->_plotter._walls[41].bottom = 106;
		_vm->_room->_plotter._walls[40].left = 94;
	}
}

void AmazonScripts::plotInactive() {
	if (_game->_charSegSwitch) {
		_game->_currentCharFlag = true;
		SpriteResource *tmp = _vm->_inactive._spritesPtr;
		_vm->_inactive._spritesPtr = _vm->_player->_playerSprites;
		_vm->_player->_playerSprites = tmp;
		_game->_charSegSwitch = false;
	} else if (_game->_jasMayaFlag != _game->_currentCharFlag) {
		if (_vm->_player->_playerOff) {
			_game->_jasMayaFlag = _game->_currentCharFlag;
		} else {
			_game->_currentCharFlag = _game->_jasMayaFlag;
			int tmpX = _game->_rawInactiveX;
			int tmpY = _game->_rawInactiveY;
			_game->_rawInactiveX = _vm->_player->_rawPlayer.x;
			_game->_rawInactiveY = _vm->_player->_rawPlayer.y;
			_vm->_player->_rawPlayer.x = tmpX;
			_vm->_player->_rawPlayer.y = tmpY;
			_game->_inactiveYOff = _vm->_player->_playerOffset.y;
			_vm->_player->calcManScale();
		}
	}

	if (_vm->_player->_roomNumber == 44) {
		warning("CHECKME: Only sets useless(?) flags 155 and 160");
	}

	_vm->_inactive._flags &= 0xFD;
	_vm->_inactive._flags &= 0xF7;
	_vm->_inactive._position.x = _game->_rawInactiveX;
	_vm->_inactive._position.y = _game->_rawInactiveY - _game->_inactiveYOff;
	_vm->_inactive._offsetY = _game->_inactiveYOff;
	_vm->_inactive._frameNumber = 0;

	_vm->_images.addToList(&_vm->_inactive);

}

void AmazonScripts::executeSpecial(int commandIndex, int param1, int param2) {
	switch (commandIndex) {
	case 1:
		_vm->establish(param1, param2);
		break;
	case 2:
		loadBackground(param1, param2);
		break;
	case 3:
		warning("TODO DOCAST");
		break;
	case 4:
		setInactive();
		break;
	case 6:
		mWhile(param1);
		break;
	case 9:
		warning("TODO GUARD");
		break;
	case 10:
		warning("TODO NEWMUSIC");
		break;
	case 11:
		plotInactive();
		break;
	case 13:
		warning("TODO RIVER");
		break;
	case 14:
		warning("TODO ANT");
		break;
	case 15:
		boatWalls(param1, param2);
		break;
	default:
		warning("Unexpected Special code %d - Skipped", commandIndex);
	}
}

typedef void(AmazonScripts::*AmazonScriptMethodPtr)();

void AmazonScripts::executeCommand(int commandIndex) {
	static const AmazonScriptMethodPtr COMMAND_LIST[] = {
		&AmazonScripts::cmdHelp, &AmazonScripts::CMDCYCLEBACK,
		&AmazonScripts::CMDCHAPTER, &AmazonScripts::cmdSetHelp,
		&AmazonScripts::cmdCenterPanel, &AmazonScripts::cmdMainPanel,
		&AmazonScripts::CMDRETFLASH
	};

	if (commandIndex >= 73)
		(this->*COMMAND_LIST[commandIndex - 73])();
	else
		Scripts::executeCommand(commandIndex);
}

void AmazonScripts::cmdHelp() {
	Common::String helpMessage = readString();

	if (_game->_helpLevel == 0) {
		_game->_timers.saveTimers();
		_game->_useItem = 0;

		if (_game->_noHints) {
			printString(NO_HELP_MESSAGE);
			return;
		} else if (_game->_hintLevel == 0) {
			printString(NO_HINTS_MESSAGE);
			return;
		}
	}

	int level = _game->_hintLevel - 1;
	if (level < _game->_helpLevel)
		_game->_moreHelp = 0;

	_game->drawHelp();
	error("TODO: more cmdHelp");
}

void AmazonScripts::CMDCYCLEBACK() { 
	error("TODO CMDCYCLEBACK"); 
}
void AmazonScripts::CMDCHAPTER() { 
	error("TODO CMDCHAPTER"); 
}

void AmazonScripts::cmdSetHelp() {
	int arrayId = (_data->readUint16LE() && 0xFF) - 1;
	int helpId = _data->readUint16LE() && 0xFF;

	byte *help = _vm->_helpTbl[arrayId];
	help[helpId] = 1;

	if (_vm->_useItem == 0) {
		_sequence = 11000;
		searchForSequence();
	}
}

void AmazonScripts::cmdCenterPanel() {
	if (_vm->_screen->_vesaMode) {
		_vm->_screen->clearScreen();
		_vm->_screen->setPanel(3);
	}
}

void AmazonScripts::cmdMainPanel() {
	if (_vm->_screen->_vesaMode) {
		_vm->_room->init4Quads();
		_vm->_screen->setPanel(0);
	}
}

void AmazonScripts::CMDRETFLASH() { 
	error("TODO CMDRETFLASH"); 
}

} // End of namespace Amazon

} // End of namespace Access
