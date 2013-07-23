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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/graphics.h"

#include "common/file.h"
#include "common/str.h"

namespace Mortevielle {

/**
 * Next word
 * @remarks	Originally called 'l_motsuiv'
 */
int TextHandler::nextWord(int p, const char *ch, int &tab) {
	int c = p;

	while ((ch[p] != ' ') && (ch[p] != '$') && (ch[p] != '@'))
		++p;

	return tab * (p - c);
}

/**
 * Engine function - Display Text
 * @remarks	Originally called 'afftex'
 */
void TextHandler::displayStr(Common::String inputStr, int x, int y, int dx, int dy, int typ) {
	int tab;
	Common::String s;
	int i, j;

	// Safeguard: add $ just in case
	inputStr += '$';

	_vm->_screenSurface.putxy(x, y);
	if (_vm->_resolutionScaler == 1)
		tab = 10;
	else
		tab = 6;
	dx *= 6;
	dy *= 6;
	int xc = x;
	int yc = y;
	int xf = x + dx;
	int yf = y + dy;
	int p = 0;
	bool stringParsed = (inputStr[p] == '$');
	s = "";
	while (!stringParsed) {
		switch (inputStr[p]) {
		case '@':
			_vm->_screenSurface.drawString(s, typ);
			s = "";
			++p;
			xc = x;
			yc += 6;
			_vm->_screenSurface.putxy(xc, yc);
			break;
		case ' ':
			s += ' ';
			xc += tab;
			++p;
			if (nextWord(p, inputStr.c_str(), tab) + xc > xf) {
				_vm->_screenSurface.drawString(s, typ);
				s = "";
				xc = x;
				yc += 6;
				if (yc > yf) {
					while (!_vm->keyPressed())
						;
					i = y;
					do {
						j = x;
						do {
							_vm->_screenSurface.putxy(j, i);
							_vm->_screenSurface.drawString(" ", 0);
							j += 6;
						} while (j <= xf);
						i += 6;
					} while (i <= yf);
					yc = y;
				}
				_vm->_screenSurface.putxy(xc, yc);
			}
			break;
		case '$':
			stringParsed = true;
			_vm->_screenSurface.drawString(s, typ);
			break;
		default:
			s += inputStr[p];
			++p;
			xc += tab;
			break;
		}
	}
}

/**
 * Load DES file
 * @remarks	Originally called 'chardes'
 */
void TextHandler::loadDesFile(Common::String filename, int32 skipSize, int length) {
	Common::File f;
	if (!f.open(filename))
		error("Missing file %s", filename.c_str());

	assert(skipSize + length <= f.size());
	f.seek(skipSize);
	f.read(&_vm->_mem[(kAdrPictureComp * 16)], length);
	f.close();
}

/**
 * Load ANI file
 * @remarks	Originally called 'charani'
 */
void TextHandler::loadAniFile(Common::String filename, int32 skipSize, int length) {
	Common::File f;
	if (!f.open(filename))
		error("Missing file - %s", filename.c_str());

	assert(skipSize + length <= f.size());
	f.seek(skipSize);
	f.read(&_vm->_mem[(kAdrAni * 16)], length);
	f.close();
}

void TextHandler::taffich() {
	static const byte rang[16] = {15, 14, 11, 7, 13, 12, 10, 6, 9, 5, 3, 1, 2, 4, 8, 0};

	static const byte tran1[] = { 121, 121, 138, 139, 120 };
	static const byte tran2[] = { 150, 150, 152, 152, 100, 110, 159, 100, 100 };

	int cx, drawingSize, npal;
	int32 drawingStartPos;
	int alllum[16];

	int a = _vm->_caff;
	if ((a >= 153) && (a <= 161))
		a = tran2[a - 153];
	else if ((a >= 136) && (a <= 140))
		a = tran1[a - 136];
	int b = a;
	if (_vm->_maff == a)
		return;

	switch (a) {
	case 16:
		_vm->_coreVar._pctHintFound[9] = '*';
		_vm->_coreVar._availableQuestion[42] = '*';
		break;
	case 20:
		_vm->_coreVar._availableQuestion[39] = '*';
		if (_vm->_coreVar._availableQuestion[36] == '*') {
			_vm->_coreVar._pctHintFound[3] = '*';
			_vm->_coreVar._availableQuestion[38] = '*';
		}
		break;
	case 24:
		_vm->_coreVar._availableQuestion[37] = '*';
		break;
	case 30:
		_vm->_coreVar._availableQuestion[9] = '*';
		break;
	case 31: // Coat of arms
		_vm->_coreVar._pctHintFound[4] = '*';
		_vm->_coreVar._availableQuestion[35] = '*';
		break;
	case 118:
		_vm->_coreVar._availableQuestion[41] = '*';
		break;
	case 143:
		_vm->_coreVar._pctHintFound[1] = '*';
		break;
	case 150:
		_vm->_coreVar._availableQuestion[34] = '*';
		break;
	case 151:
		_vm->_coreVar._pctHintFound[2] = '*';
		break;
	default:
		break;
	}

	_vm->_destinationOk = true;
	_vm->_mouse.hideMouse();
	drawingStartPos = 0;
	Common::String filename;

	if ((a != 50) && (a != 51)) {
		_vm->_maff = a;
		if (a == 159)
			a = 86;
		else if (a > 140)
			a -= 67;
		else if (a > 137)
			a -= 66;
		else if (a > 99)
			a -= 64;
		else if (a > 69)
			a -= 42;
		else if (a > 29)
			a -= 5;
		else if (a == 26)
			a = 24;
		else if (a > 18)
			--a;
		npal = a;

		for (cx = 0; cx <= (a - 1); ++cx)
			drawingStartPos += _vm->_drawingSizeArr[cx];
		drawingSize = _vm->_drawingSizeArr[a];

		filename = "DXX.mor";
	} else {
		if (_vm->getLanguage() == Common::DE_DEU)
			filename = "DZZALL";
		else
			filename = "DZZ.mor";

		if (a == 50) {
			drawingStartPos = 0;
			drawingSize = _vm->_drawingSizeArr[87];
		} else { // a == 51
			drawingStartPos = _vm->_drawingSizeArr[87];
			drawingSize = _vm->_drawingSizeArr[88];
		}
		_vm->_maff = a;
		npal = a + 37;
	}
	loadDesFile(filename, drawingStartPos, drawingSize);
	if (_vm->_currGraphicalDevice == MODE_HERCULES) {
		for (int i = 0; i <= 15; ++i) {
			int palh = READ_LE_UINT16(&_vm->_mem[(kAdrPictureComp * 16) + 2 + (i << 1)]);
			alllum[i] = (palh & 15) + (((uint)palh >> 12) & 15) + (((uint)palh >> 8) & 15);
		}
		for (int i = 0; i <= 15; ++i) {
			int k = 0;
			for (int j = 0; j <= 15; ++j) {
				if (alllum[j] > alllum[k])
					k = j;
			}
			_vm->_mem[(kAdrPictureComp * 16) + 2 + (k << 1)] = rang[i];
			alllum[k] = -1;
		}
	}
	_vm->_numpal = npal;
	_vm->setPal(npal);

	if ((b < 15) || (b == 16) || (b == 17) || (b == 24) || (b == 26) || (b == 50)) {
		drawingStartPos = 0;
		if ((b < 15) || (b == 16) || (b == 17) || (b == 24) || (b == 26)) {
			if (b == 26)
				b = 18;
			else if (b == 24)
				b = 17;
			else if (b > 15)
				--b;
			for (cx = 0; cx <= (b - 1); ++cx)
				drawingStartPos += _vm->_drawingSizeArr[cx + 89];
			drawingSize = _vm->_drawingSizeArr[b + 89];
			filename = "AXX.mor";
		} else { // b == 50
			// CHECKME: the size of AZZ.mor is 1280 for the DOS version
			//          and 1260 for the Amiga version. Maybe the 20 bytes 
			//          are a filler (to get 10 blocks of 128 bytes), 
			//          or the size should be variable.
			drawingSize = 1260;
			filename = "AZZ.mor";
		}
		loadAniFile(filename, drawingStartPos, drawingSize);
	}
	_vm->_mouse.showMouse();
	if ((a < COAT_ARMS) && ((_vm->_maff < COAT_ARMS) || (_vm->_coreVar._currPlace == LANDING)) && (_vm->_currAction != OPCODE_ENTER)) {
		if ((a == ATTIC) || (a == CELLAR))
			_vm->displayAloneText();
		else if (!_vm->_blo)
			_vm->getPresence(_vm->_coreVar._currPlace);
		_vm->_savedBitIndex =  0;
	}
}

void TextHandler::setParent(MortevielleEngine *vm) {
	_vm = vm;
}

} // End of namespace Mortevielle
