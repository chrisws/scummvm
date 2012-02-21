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
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/file.h"
#include "common/str.h"
#include "mortevielle/level15.h"
#include "mortevielle/mor.h"
#include "mortevielle/mouse.h"
#include "mortevielle/taffich.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

void chardes(Common::String filename, int32 skipSize, int length) {
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
		f.read(&mem[memIndx], 128);
		testfi();
		totalLength -= 128;
		memIndx += 128;
	}
	f.close();

	for (int i = remainingSkipSize; i <= length + remainingSkipSize; ++i) 
		mem[0x7000 * 16 + i - remainingSkipSize] = mem[0x6000 * 16 + i];
}

void charani(Common::String filename, int32 skipSize, int length) {
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
		f.read(&mem[memIndx], 128);
		testfi();
		fullLength -= 128;
		memIndx += 128;
	}
	f.close();

	for (int i = remainingSkipSize; i <= length + remainingSkipSize; ++i)
		mem[0x7314 * 16 + i - remainingSkipSize] = mem[0x6000 * 16 + i];
}

void taffich() {
	byte tran1[] = { 121, 121, 138, 139, 120 };	// array<136, 140, byte>
	byte tran2[] = { 150, 150, 152, 152, 100, 110, 159, 100, 100 };	// array<153, 161, byte>

	int i, m, cx, handle, npal;
	int32 lgt;
	int palh, k, j;
	int alllum[16];


	int a = caff;
	if ((a >= 153) && (a <= 161))
		a = tran2[a - 153];
	else if ((a >= 136) && (a <= 140))
		a = tran1[a - 136];
	int b = a;
	if (_maff == a)
		return;

	switch (a) {
	case 16:
		s.pourc[9] = '*';
		s.teauto[42] = '*';
		break;
	case 20:
		s.teauto[39] = '*';
		if (s.teauto[36] == '*') {
			s.pourc[3] = '*';
			s.teauto[38] = '*';
		}
		break;
	case 24:
		s.teauto[37] = '*';
		break;
	case 30:
		s.teauto[9] = '*';
		break;
	case 31:
		s.pourc[4] = '*';
		s.teauto[35] = '*';
		break;
	case 118:
		s.teauto[41] = '*';
		break;
	case 143:
		s.pourc[1] = '*';
		break;
	case 150:
		s.teauto[34] = '*';
		break;
	case 151:
		s.pourc[2] = '*';
		break;
	default:
		break;
	}

	okdes = true;
	hideMouse();
	lgt = 0;
	Common::String filename;

	if ((a != 50) && (a != 51)) {
		m = a + 2000;
		if ((m > 2001) && (m < 2010))
			m = 2001;
		if (m == 2011)
			m = 2010;
		if (a == 32)
			m = 2034;
		if ((a == 17) && (_maff == 14))
			m = 2018;
		if (a > 99)
			if ((is == 1) || (is == 0))
				m = 2031;
			else
				m = 2032;
		if (((a > 69) && (a < 80)) || (a == 30) || (a == 31) || (a == 144) || (a == 147) || (a == 149))
			m = 2030;
		if (((a < 27) && (((_maff > 69) && (! s.ipre)) || (_maff > 99))) || ((_maff > 29) && (_maff < 33)))
			m = 2033;
		messint(m);
		_maff = a;
		if (a == 159)
			a = 86;
		else if (a > 140)
			a = a - 67;
		else if (a > 137)
			a = a - 66;
		else if (a > 99)
			a = a - 64;
		else if (a > 69)
			a = a - 42;
		else if (a > 29)
			a = a - 5;
		else if (a == 26)
			a = 24;
		else if (a > 18)
			a = a - 1;
		npal = a;

		for (cx = 0; cx <= (a - 1); ++cx)
			lgt = lgt + l[cx];
		handle = l[a];

		filename = "DXX.mor";
	} else {
		filename = "DZZ.mor";
		handle = l[87];
		if (a == 51) {
			lgt = handle;
			handle = l[88];
		}
		_maff = a;
		npal = a + 37;
	}
	chardes(filename, lgt, handle);
	if (_currGraphicalDevice == MODE_HERCULES) {
		for (i = 0; i <= 15; ++i) {
			palh = READ_LE_UINT16(&mem[0x7000 * 16 + (succ(int, i) << 1)]);
			alllum[i] = (palh & 15) + (((uint)palh >> 12) & 15) + (((uint)palh >> 8) & 15);
		}
		for (i = 0; i <= 15; ++i) {
			k = 0;
			for (j = 0; j <= 15; ++j)
				if (alllum[j] > alllum[k])
					k = j;
			mem[0x7000 * 16 + 2 + (k << 1)] = rang[i];
			alllum[k] = -1;
		}
	}
	numpal = npal;
	writepal(npal);

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
				lgt += l[cx + 89];
			handle = l[b + 89];
			filename = "AXX.mor";
		} else if (b == 50) {
			filename = "AZZ.mor";
			handle = 1260;
		}
		charani(filename, lgt, handle);
	}
	showMouse();
	if ((a < 27) && ((_maff < 27) || (s.mlieu == 15)) && (msg[4] != OPCODE_ENTER)) {
		if ((a == 13) || (a == 14))
			person();
		else if (! blo)
			t11(s.mlieu, cx);
		mpers =  0;
	}
}

} // End of namespace Mortevielle
