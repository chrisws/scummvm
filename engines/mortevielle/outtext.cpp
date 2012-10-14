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

#include "common/file.h"
#include "common/str.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/graphics.h"
#include "mortevielle/mortevielle.h"

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

	g_vm->_screenSurface.putxy(x, y);
	if (g_vm->_resolutionScaler == 1)
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
			g_vm->_screenSurface.drawString(s, typ);
			s = "";
			++p;
			xc = x;
			yc += 6;
			g_vm->_screenSurface.putxy(xc, yc);
			break;
		case ' ':
			s += ' ';
			xc += tab;
			++p;
			if (nextWord(p, inputStr.c_str(), tab) + xc > xf) {
				g_vm->_screenSurface.drawString(s, typ);
				s = "";
				xc = x;
				yc += 6;
				if (yc > yf) {
					while (!g_vm->keyPressed())
						;
					i = y;
					do {
						j = x;
						do {
							g_vm->_screenSurface.putxy(j, i);
							g_vm->_screenSurface.drawString(" ", 0);
							j += 6;
						} while (j <= xf);
						i += 6;
					} while (i <= yf);
					yc = y;
				}
				g_vm->_screenSurface.putxy(xc, yc);
			}
			break;
		case '$':
			stringParsed = true;
			g_vm->_screenSurface.drawString(s, typ);
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

	int skipBlock = 0;
	while (skipSize > 127) {
		++skipBlock;
		skipSize -= 128;
	}
	if (skipBlock != 0)
		f.seek(skipBlock * 0x80);

	int remainingSkipSize = abs(skipSize);
	int totalLength = length + remainingSkipSize;
	int memIndx = 0x6000 * 16;
	while (totalLength > 0) {
		f.read(&g_vm->_mem[memIndx], 128);
		totalLength -= 128;
		memIndx += 128;
	}
	f.close();

	for (int i = remainingSkipSize; i <= length + remainingSkipSize; ++i)
		g_vm->_mem[0x7000 * 16 + i - remainingSkipSize] = g_vm->_mem[0x6000 * 16 + i];
}

/**
 * Load ANI file
 * @remarks	Originally called 'charani'
 */
void TextHandler::loadAniFile(Common::String filename, int32 skipSize, int length) {
	Common::File f;
	if (!f.open(filename))
		error("Missing file - %s", filename.c_str());

	int skipBlock = 0;
	while (skipSize > 127) {
		skipSize = skipSize - 128;
		++skipBlock;
	}
	if (skipBlock != 0)
		f.seek(skipBlock * 0x80);

	int remainingSkipSize = abs(skipSize);
	int fullLength = length + remainingSkipSize;
	int memIndx = 0x6000 * 16;
	while (fullLength > 0) {
		f.read(&g_vm->_mem[memIndx], 128);
		fullLength -= 128;
		memIndx += 128;
	}
	f.close();

	for (int i = remainingSkipSize; i <= length + remainingSkipSize; ++i)
		g_vm->_mem[kAdrAni * 16 + i - remainingSkipSize] = g_vm->_mem[0x6000 * 16 + i];
}

void TextHandler::taffich() {
	static const byte _rang[16] = {15, 14, 11, 7, 13, 12, 10, 6, 9, 5, 3, 1, 2, 4, 8, 0};

	byte tran1[] = { 121, 121, 138, 139, 120 };
	byte tran2[] = { 150, 150, 152, 152, 100, 110, 159, 100, 100 };

	int cx, handle, npal;
	int32 lgt;
	int alllum[16];

	int a = g_vm->_caff;
	if ((a >= 153) && (a <= 161))
		a = tran2[a - 153];
	else if ((a >= 136) && (a <= 140))
		a = tran1[a - 136];
	int b = a;
	if (g_vm->_maff == a)
		return;

	switch (a) {
	case 16:
		g_vm->_coreVar._pctHintFound[9] = '*';
		g_vm->_coreVar._availableQuestion[42] = '*';
		break;
	case 20:
		g_vm->_coreVar._availableQuestion[39] = '*';
		if (g_vm->_coreVar._availableQuestion[36] == '*') {
			g_vm->_coreVar._pctHintFound[3] = '*';
			g_vm->_coreVar._availableQuestion[38] = '*';
		}
		break;
	case 24:
		g_vm->_coreVar._availableQuestion[37] = '*';
		break;
	case 30:
		g_vm->_coreVar._availableQuestion[9] = '*';
		break;
	case 31: // Coat of arms
		g_vm->_coreVar._pctHintFound[4] = '*';
		g_vm->_coreVar._availableQuestion[35] = '*';
		break;
	case 118:
		g_vm->_coreVar._availableQuestion[41] = '*';
		break;
	case 143:
		g_vm->_coreVar._pctHintFound[1] = '*';
		break;
	case 150:
		g_vm->_coreVar._availableQuestion[34] = '*';
		break;
	case 151:
		g_vm->_coreVar._pctHintFound[2] = '*';
		break;
	default:
		break;
	}

	g_vm->_destinationOk = true;
	g_vm->_mouse.hideMouse();
	lgt = 0;
	Common::String filename;

	if ((a != 50) && (a != 51)) {
		g_vm->_maff = a;
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
			lgt += g_vm->_fxxBuffer[cx];
		handle = g_vm->_fxxBuffer[a];

		filename = "DXX.mor";
	} else {
		if (g_vm->getLanguage() == Common::DE_DEU)
			filename = "DZZALL";
		else
			filename = "DZZ.mor";

		handle = g_vm->_fxxBuffer[87];
		if (a == 51) {
			lgt = handle;
			handle = g_vm->_fxxBuffer[88];
		}
		g_vm->_maff = a;
		npal = a + 37;
	}
	loadDesFile(filename, lgt, handle);
	if (g_vm->_currGraphicalDevice == MODE_HERCULES) {
		for (int i = 0; i <= 15; ++i) {
			int palh = READ_LE_UINT16(&g_vm->_mem[(0x7000 * 16) + ((i + 1) << 1)]);
			alllum[i] = (palh & 15) + (((uint)palh >> 12) & 15) + (((uint)palh >> 8) & 15);
		}
		for (int i = 0; i <= 15; ++i) {
			int k = 0;
			for (int j = 0; j <= 15; ++j)
				if (alllum[j] > alllum[k])
					k = j;
			g_vm->_mem[(0x7000 * 16) + 2 + (k << 1)] = _rang[i];
			alllum[k] = -1;
		}
	}
	g_vm->_numpal = npal;
	g_vm->setPal(npal);

	if ((b < 15) || (b == 16) || (b == 17) || (b == 24) || (b == 26) || (b == 50)) {
		lgt = 0;
		if ((b < 15) || (b == 16) || (b == 17) || (b == 24) || (b == 26)) {
			if (b == 26)
				b = 18;
			else if (b == 24)
				b = 17;
			else if (b > 15)
				--b;
			for (cx = 0; cx <= (b - 1); ++cx)
				lgt += g_vm->_fxxBuffer[cx + 89];
			handle = g_vm->_fxxBuffer[b + 89];
			filename = "AXX.mor";
		} else if (b == 50) {
			filename = "AZZ.mor";
			handle = 1260;
		}
		loadAniFile(filename, lgt, handle);
	}
	g_vm->_mouse.showMouse();
	if ((a < 27) && ((g_vm->_maff < 27) || (g_vm->_coreVar._currPlace == LANDING)) && (g_vm->_msg[4] != OPCODE_ENTER)) {
		if ((a == 13) || (a == 14))
			g_vm->displayAloneText();
		else if (!g_vm->_blo)
			g_vm->getPresence(g_vm->_coreVar._currPlace);
		g_vm->_savedBitIndex =  0;
	}
}

} // End of namespace Mortevielle
