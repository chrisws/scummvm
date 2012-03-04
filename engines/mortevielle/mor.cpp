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

#include "common/endian.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/graphics.h"
#include "mortevielle/level15.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/parole2.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

// For ScummVM, we need to do check for file errors where we do the file access
const int ioresult = 0;

void testfi() {
	if (ioresult != 0) {
		// Theoritical message
		warning("IO Error");
		g_vm->quitGame();
	}
}

/**
 * Read the current system time
 */
int readclock() {
	TimeDate dateTime;
	g_system->getTimeAndDate(dateTime);

	int m = dateTime.tm_min * 60;
	int h = dateTime.tm_hour * 3600;
	return h + m + dateTime.tm_sec;
}

void modif(int &nu) {
	if (nu == 26)
		nu = 25;
	else if ((nu > 29) && (nu < 36))
		nu -= 4;
	else if ((nu > 69) && (nu < 78))
		nu -= 37;
	else if ((nu > 99) && (nu < 194))
		nu -= 59;
	else if ((nu > 996) && (nu < 1000))
		nu -= 862;
	else if ((nu > 1500) && (nu < 1507))
		nu -= 1363;
	else if ((nu > 1507) && (nu < 1513))
		nu -= 1364;
	else if ((nu > 1999) && (nu < 2002))
		nu -= 1851;
	else if (nu == 2010)
		nu = 151;
	else if ((nu > 2011) && (nu < 2025))
		nu -= 1860;
	else if (nu == 2026)
		nu = 165;
	else if ((nu > 2029) && (nu < 2037))
		nu -= 1864;
	else if ((nu > 3000) && (nu < 3005))
		nu -= 2828;
	else if (nu == 4100)
		nu = 177;
	else if (nu == 4150)
		nu = 178;
	else if ((nu > 4151) && (nu < 4156))
		nu -= 3973;
	else if (nu == 4157)
		nu = 183;
	else if ((nu == 4160) || (nu == 4161))
		nu -= 3976;
}


void dessine(int ad, int x, int y) {
	hideMouse();
	writepal(g_numpal);
	pictout(ad, 0, x, y);
	showMouse();
}

void dessine_rouleau() {
	writepal(89);
	if (g_currGraphicalDevice == MODE_HERCULES) {
		g_mem[0x7000 * 16 + 14] = 15;
	}
	hideMouse();
	pictout(0x73a2, 0, 0, 0);
	showMouse();
}


void text_color(int c) {
	g_color_txt = c;
}

/* NIVEAU 13 */


void text1(int x, int y, int nb, int m) {
	int co;

	if (g_res == 1)
		co = 10;
	else
		co = 6;
	Common::String tmpStr = deline(m);
	if ((y == 182) && ((int) tmpStr.size() * co > nb * 6))
		y = 176;
	displayStr(tmpStr, x, y, nb, 20, g_color_txt);
}

void initouv() {
	for (int cx = 1; cx <= 7; ++cx)
		g_touv[cx] = chr(0);
}

void ecrf1() {
	// Large drawing
	g_vm->_screenSurface.drawBox(0, 11, 512, 163, 15);
}

void clsf1() {
	hideMouse();
	g_vm->_screenSurface.fillRect(0, Common::Rect(0, 11, 514, 175));

	showMouse();
}

void clsf2() {
	hideMouse();
	if (g_f2_all) {
		g_vm->_screenSurface.fillRect(0, Common::Rect(1, 176, 633, 199));
		g_vm->_screenSurface.drawBox(0, 175, 634, 24, 15);
		g_f2_all = false;
	} else {
		g_vm->_screenSurface.fillRect(0, Common::Rect(1, 176, 633, 190));
		g_vm->_screenSurface.drawBox(0, 175, 634, 15, 15);
	}
	showMouse();
}

void ecrf2() {
	text_color(5);
}

void ecr2(Common::String str_) {
	// Some dead code was present in the original: removed
	g_vm->_screenSurface.putxy(8, 177);
	int tlig = 59 + (g_res - 1) * 36;

	if ((int)str_.size() < tlig)
		g_vm->_screenSurface.writeg(str_, 5);
	else if ((int)str_.size() < (tlig << 1)) {
		g_vm->_screenSurface.putxy(8, 176);
		g_vm->_screenSurface.writeg(copy(str_, 1, (tlig - 1)), 5);
		g_vm->_screenSurface.putxy(8, 182);
		g_vm->_screenSurface.writeg(copy(str_, tlig, tlig << 1), 5);
	} else {
		g_f2_all = true;
		clsf2();
		g_vm->_screenSurface.putxy(8, 176);
		g_vm->_screenSurface.writeg(copy(str_, 1, (tlig - 1)), 5);
		g_vm->_screenSurface.putxy(8, 182);
		g_vm->_screenSurface.writeg(copy(str_, tlig, ((tlig << 1) - 1)), 5);
		g_vm->_screenSurface.putxy(8, 190);
		g_vm->_screenSurface.writeg(copy(str_, tlig << 1, tlig * 3), 5);
	}
}

void clsf3() {
	hideMouse();
	g_vm->_screenSurface.fillRect(0, Common::Rect(1, 192, 633, 199));
	g_vm->_screenSurface.drawBox(0, 191, 634, 8, 15);
	showMouse();
}

void ecr3(Common::String text) {
	clsf3();
	g_vm->_screenSurface.putxy(8, 192);
	g_vm->_screenSurface.writeg(text, 5);
}

void ecrf6() {
	text_color(5);
	g_vm->_screenSurface.drawBox(62, 33, 363, 80, 15);
}

void ecrf7() {
	text_color(4);
}

void clsf10() {
	int co, cod;
	Common::String st;

	hideMouse();
	if (g_res == 1) {
		co = 634;
		cod = 534;
	} else {
		co = 600;
		cod = 544;
	}
	g_vm->_screenSurface.fillRect(15, Common::Rect(cod, 93, co, 98));
	if (g_s._conf < 33)
		st = g_vm->getEngineString(S_COOL);
	else if (g_s._conf < 66)
		st = g_vm->getEngineString(S_LOURDE);
	else if (g_s._conf > 65)
		st = g_vm->getEngineString(S_MALSAINE);
	
	co = 580 - (g_vm->_screenSurface.getStringWidth(st) / 2);
	g_vm->_screenSurface.putxy(co, 92);
	g_vm->_screenSurface.writeg(st, 4);

	if (g_res == 1)
		co = 620;
	else
		co = 584;

	g_vm->_screenSurface.fillRect(15, Common::Rect(560, 24, co, 86));
	/* rempli(69,12,32,5,255);*/
	showMouse();
}

void stop() {
	hirs();
	g_currGraphicalDevice = MODE_AMSTRAD1512;
	hirs();
	g_vm->quitGame();
}

void paint_rect(int x, int y, int dx, int dy) {
	int co;

	if (g_currGraphicalDevice == MODE_CGA)
		co = 3;
	else
		co = 11;
	g_vm->_screenSurface.fillRect(co, Common::Rect(x, y, x + dx, y + dy));
}

void calch(int &j, int &h, int &m) {
	int nh = readclock();
	int th = g_jh + ((nh - g_mh) / g_t);
	m = ((th % 2) + g_vm__) * 30;
	h = ((uint)th >> 1) + g_vh;
	if (m == 60) {
		m = 0;
		h = h + 1;
	}
	j = (h / 24) + g_vj;
	h = h - ((j - g_vj) * 24);
}

void conv(int x, int &y) {
	int cx = 1;
	y = 128;
	while (cx < x) {
		y = (uint)y >> 1;
		++cx;
	}
}

/* NIVEAU 12 */
void modobj(int m) {
	Common::String strp = Common::String(' ');

	if (m != 500)
		strp = deline(m - 501 + kInventoryStringIndex);

	g_vm->_menu.menut(g_vm->_menu._inventoryMenu[8], strp);
	g_vm->_menu.disableMenuItem(g_vm->_menu._inventoryMenu[8]);
}

void repon(int f, int m) {
	Common::String str_;
	Common::String str1;
	Common::String tmpStr;

	if ((m > 499) && (m < 563)) {
		tmpStr = deline(m - 501 + kInventoryStringIndex);

		if ((int) tmpStr.size() > ((58 + (g_res - 1) * 37) << 1))
			g_f2_all = true;
		else
			g_f2_all = false;

		clsf2();
		displayStr(tmpStr, 8, 176, 85, 3, 5);
	} else {
		modif(m);
		if (f == 8)
			f = 2;
		if (f == 1)
			f = 6;
		if (f == 2) {
			clsf2();
			ecrf2();
			text1(8, 182, 103, m);
			if ((m == 68) || (m == 69))
				g_s._teauto[40] = '*';
			if ((m == 104) && (g_caff == 14)) {
				g_s._teauto[36] = '*';
				if (g_s._teauto[39] == '*') {
					g_s._pourc[3] = '*';
					g_s._teauto[38] = '*';
				}
			}
		}
		if ((f == 6) || (f == 9)) {
			int i;
			if (f == 6)
				i = 4;
			else
				i = 5;

			tmpStr = deline(m);
			displayStr(tmpStr, 80, 40, 60, 25, i);

			if (m == 180)
				g_s._pourc[6] = '*';

			if (m == 179)
				g_s._pourc[10] = '*';
		}
		if (f == 7) {         /* messint */
			ecrf7();
			tmpStr = deline(m);

			int xSmallStr, xLargeStr, dx;
			if (g_res == 1) {
				xSmallStr = 252 - tmpStr.size() * 5;
				xLargeStr = 100;
				dx = 80;
			} else {
				xSmallStr = 252 - tmpStr.size() * 3;
				xLargeStr = 144;
				dx = 50;
			}

			if (tmpStr.size() < 40)
				displayStr(tmpStr, xSmallStr, 86, dx, 3, 5);
			else
				displayStr(tmpStr, xLargeStr, 86, dx, 3, 5);
		}
	}
}

void t5(int cx) {
	if (cx == 10)
		g_blo = false;

	if (cx != 1) {
		g_bh1 = false;
		g_bf1 = false;
	}

	if (cx != 2)
		g_bh2 = false;

	if (cx != 4) {
		g_bh4 = false;
		g_bf4 = false;
	}

	if (cx != 5)
		g_bh5 = false;
	if (cx != 6)
		g_bh6 = false;
	if (cx != 8)
		g_bh8 = false;
	if (cx != 3)
		g_bt3 = false;
	if (cx != 7)
		g_bt7 = false;
	if (cx != 9)
		g_bh9 = false;
}

void affper(int per) {
	int cx;

	for (cx = 1; cx <= 8; ++cx)
		g_vm->_menu.disableMenuItem(g_vm->_menu._discussMenu[cx]);
	clsf10();
	if ((per & 128) == 128) {
		g_vm->_screenSurface.putxy(560, 24);
		g_vm->_screenSurface.writeg("LEO", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[1]);
	}
	if ((per & 64) == 64) {
		g_vm->_screenSurface.putxy(560, 32);
		g_vm->_screenSurface.writeg("PAT", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[2]);
	}
	if ((per & 32) == 32) {
		g_vm->_screenSurface.putxy(560, 40);
		g_vm->_screenSurface.writeg("GUY", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[3]);
	}
	if ((per & 16) == 16) {
		g_vm->_screenSurface.putxy(560, 48);
		g_vm->_screenSurface.writeg("EVA", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[4]);
	}
	if ((per & 8) == 8) {
		g_vm->_screenSurface.putxy(560, 56);
		g_vm->_screenSurface.writeg("BOB", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[5]);
	}
	if ((per & 4) == 4) {
		g_vm->_screenSurface.putxy(560, 64);
		g_vm->_screenSurface.writeg("LUC", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[6]);
	}
	if ((per & 2) == 2) {
		g_vm->_screenSurface.putxy(560, 72);
		g_vm->_screenSurface.writeg("IDA", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[7]);
	}
	if ((per & 1) == 1) {
		g_vm->_screenSurface.putxy(560, 80);
		g_vm->_screenSurface.writeg("MAX", 4);
		g_vm->_menu.enableMenuItem(g_vm->_menu._discussMenu[8]);
	}
	g_ipers = per;
}

void choix(int min, int max, int &per) {
	bool i;
	int cz;

	int rand = getRandomNumber(min, max);
	if (rand > 4) {
		rand = 8 - rand;
		i = true;
	} else
		i = false;

	int cx = 0;
	per = 0;
	while (cx < rand) {
		int cy = getRandomNumber(1, 8);
		conv(cy, cz);
		if ((per & cz) != cz) {
			++cx;
			per |= cz;
		}
	}
	if (i)
		per = 255 - per;
}

void cpl1(int &p) {
	int j, h, m;

	calch(j, h, m);
	// The original uses an || instead of an &&, resulting 
	// in an always true condition. Based on the other tests, 
	// and on other scenes, we use an && instead.
	// if ((h > 7) || (h < 11))
	if ((h > 7) && (h < 11))
		p = 25;
	else if ((h > 10) && (h < 14))
		p = 35;
	else if ((h > 13) && (h < 16))
		p = 50;
	else if ((h > 15) && (h < 18))
		p = 5;
	else if ((h > 17) && (h < 22))
		p = 35;
	else if ((h > 21) && (h < 24))
		p = 50;
	else if ((h >= 0) && (h < 8))
		p = 70;

	g_vm->_menu.mdn();
}

void cpl2(int &p) {
	int j, h, m;

	calch(j, h, m);
	if ((h > 7) && (h < 11))
		p = -2;
	if (h == 11)
		p = 100;
	if ((h > 11) && (h < 23))
		p = 10;
	if (h == 23)
		p = 20;
	if ((h >= 0) && (h < 8))
		p = 50;
}

void cpl3(int &p) {
	int j, h, m;

	calch(j, h, m);
	if (((h > 8) && (h < 10)) || ((h > 19) && (h < 24)))
		p = 34;
	if (((h > 9) && (h < 20)) || ((h >= 0) && (h < 9)))
		p = 0;
}

void cpl5(int &p) {
	int j, h, m;

	calch(j, h, m);
	if ((h > 6) && (h < 10))
		p = 0;
	if (h == 10)
		p = 100;
	if ((h > 10) && (h < 24))
		p = 15;
	if ((h >= 0) && (h < 7))
		p = 50;
}

void cpl6(int &p) {
	int j, h, m;

	calch(j, h, m);
	if (((h > 7) && (h < 13)) || ((h > 17) && (h < 20)))
		p = -2;
	if (((h > 12) && (h < 17)) || ((h > 19) && (h < 24)))
		p = 35;
	if (h == 17)
		p = 100;
	if ((h >= 0) && (h < 8))
		p = 60;
}

/**
 * Shows the you are alone message in the status area on the right hand side of the screen
 */
void person() {
	for (int cf = 1; cf <= 8; ++cf)
		g_vm->_menu.disableMenuItem(g_vm->_menu._discussMenu[cf]);

	Common::String sYou = g_vm->getEngineString(S_YOU);
	Common::String sAre = g_vm->getEngineString(S_ARE);
	Common::String sAlone = g_vm->getEngineString(S_ALONE);

	clsf10();
	g_vm->_screenSurface.putxy(580 - (g_vm->_screenSurface.getStringWidth(sYou) / 2), 30);
	g_vm->_screenSurface.writeg(sYou, 4);
	g_vm->_screenSurface.putxy(580 - (g_vm->_screenSurface.getStringWidth(sAre) / 2), 50);
	g_vm->_screenSurface.writeg(sAre, 4);
	g_vm->_screenSurface.putxy(580 - (g_vm->_screenSurface.getStringWidth(sAlone) / 2), 70);
	g_vm->_screenSurface.writeg(sAlone, 4);

	g_ipers = 0;
}

int chlm() {
	int retval = getRandomNumber(1, 2);
	if (retval == 2)
		retval = 128;
	
	return retval;
}

/**
 * Engine function - Draw Clock
 * @remarks	Originally called 'pendule'
 */
void drawClock() {
	const int cv[2][12] = {
		{  5,  8, 10,  8,  5,  0, -5, -8, -10, -8, -5,  0 },
		{ -5, -3,  0,  3,  5,  6,  5,  3,   0, -3, -5, -6 }
	};
	const int x = 580;
	const int y = 123;
	const int rg = 9;
	int h, co;

	hideMouse();
	
	paint_rect(570, 118, 20, 10);
	paint_rect(578, 114, 6, 18);
	if ((g_currGraphicalDevice == MODE_CGA) || (g_currGraphicalDevice == MODE_HERCULES))
		co = 0;
	else
		co = 1;

	if (g_minute == 0)
		g_vm->_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)x >> 1) * g_res, (y - rg), co);
	else 
		g_vm->_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)x >> 1) * g_res, (y + rg), co);

	h = g_hour;
	if (h > 12)
		h -= 12;
	if (h == 0)
		h = 12;

	g_vm->_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)(x + cv[0][h - 1]) >> 1) * g_res, y + cv[1][h - 1], co);
	showMouse();
	g_vm->_screenSurface.putxy(568, 154);

	if (g_hour > 11)
		g_vm->_screenSurface.writeg("PM ", 1);
	else
		g_vm->_screenSurface.writeg("AM ", 1);

	g_vm->_screenSurface.putxy(550, 160);
	if ((g_day >= 0) && (g_day <= 8)) {
		Common::String tmp = g_vm->getEngineString(S_DAY);
		tmp.insertChar((char)(g_day + 49), 0);
		g_vm->_screenSurface.writeg(tmp, 1);
	}
}

/*************
 * NIVEAU 11 *
 *************/

void debloc(int l) {
	g_num = 0;
	g_x = 0;
	g_y = 0;
	if ((l != 26) && (l != 15))
		t5(l);
	g_mpers = g_ipers;
}

void cpl10(int &p, int &h) {
	int j, m;

	calch(j, h, m);
	if (((h > 7) && (h < 11)) || ((h > 11) && (h < 14)) || ((h > 18) && (h < 21)))
		p = 100;
	if ((h == 11) || ((h > 20) && (h < 24)))
		p = 45;
	if (((h > 13) && (h < 17)) || (h == 18))
		p = 35;
	if (h == 17)
		p = 60;
	if ((h >= 0) && (h < 8))
		p = 5;
}

void cpl11(int &p, int &h) {
	int j, m;

	calch(j, h, m);
	if (((h > 8) && (h < 12)) || ((h > 20) && (h < 24)))
		p = 25;
	if (((h > 11) && (h < 14)) || ((h > 18) && (h < 21)))
		p = 5;
	if ((h > 13) && (h < 17))
		p = 55;
	if ((h > 16) && (h < 19))
		p = 45;
	if ((h >= 0) && (h < 9))
		p = 0;
}

void cpl12(int &p) {
	int j, h, m;

	calch(j, h, m);
	if (((h > 8) && (h < 15)) || ((h > 16) && (h < 22)))
		p = 55;
	if (((h > 14) && (h < 17)) || ((h > 21) && (h < 24)))
		p = 25;
	if ((h >= 0) && (h < 5))
		p = 0;
	if ((h > 4) && (h < 9))
		p = 15;
}

void cpl13(int &p) {
	p = 0;
}

void cpl15(int &p) {
	int j, h, m;

	calch(j, h, m);
	if ((h > 7) && (h < 12))
		p = 25;
	else if ((h > 11) && (h < 14))
		p = 0;
	else if ((h > 13) && (h < 18))
		p = 10;
	else if ((h > 17) && (h < 20))
		p = 55;
	else if ((h > 19) && (h < 22))
		p = 5;
	else if ((h > 21) && (h < 24))
		p = 15;
	else if ((h >= 0) && (h < 8))
		p = -15;
}

void cpl20(int &p, int &h) {
	int j, m;

	calch(j, h, m);
	if (h == 10)
		p = 65;
	else if ((h > 10) && (h < 21))
		p = 5;
	else if ((h > 20) && (h < 24))
		p = -15;
	else if ((h >= 0) && (h < 5))
		p = -300;
	else if ((h > 4) && (h < 10))
		p = -5;
}

void quelq1(int l) {
	int per;

	per = getRandomNumber(1, 2);
	if (l == 1) {
		if (per == 1)
			g_bh1 = true;
		else
			g_bf1 = true;
	} else if (l == 4) {
		if (per == 1)
			g_bh4 = true;
		else
			g_bf4 = true;
	}

	g_ipers = 10;
}

void quelq2() {
	if (g_li == 2)
		g_bh2 = true;
	else
		g_bh9 = true;

	g_ipers = 10;
}

void quelq5() {
	g_bh5 = true;
	g_ipers = 10;
}

void quelq6(int l) {
	if (l == 6)
		g_bh6 = true;
	else if (l == 8)
		g_bh8 = true;

	g_ipers = 10;
}

void quelq10(int h, int &per) {
	if ((h >= 0) && (h < 8))
		per = chlm();
	else {
		int min = 0, max = 0;
		if ((h > 7) && (h < 10)) {
			min = 5;
			max = 7;
		} else if ((h > 9) && (h < 12)) {
			min = 1;
			max = 4;
		} else if (((h > 11) && (h < 15)) || ((h > 18) && (h < 21))) {
			min = 6;
			max = 8;
		} else if (((h > 14) && (h < 19)) || ((h > 20) && (h < 24))) {
			min = 1;
			max = 5;
		}
		choix(min, max, per);
	}
	affper(per);
}

void quelq11(int h, int &per) {
	if ((h >= 0) && (h < 8))
		per = chlm();
	else {
		int min = 0, max = 0;
		if (((h > 7) && (h < 10)) || ((h > 20) && (h < 24))) {
			min = 1;
			max = 3;
		} else if (((h > 9) && (h < 12)) || ((h > 13) && (h < 19))) {
			min = 1;
			max = 4;
		} else if (((h > 11) && (h < 14)) || ((h > 18) && (h < 21))) {
			min = 1;
			max = 2;
		}
		choix(min, max, per);
	}
	affper(per);
}

void quelq12(int &per) {
	per = chlm();
	affper(per);
}

void quelq15(int &per) {
	int cx;
	bool test;

	per = 0;

	do {
		cx = getRandomNumber(1, 8);
		test = (((cx == 1) && (g_bh2 || g_bh9)) ||
		        ((cx == 2) && g_bh8) ||
		        ((cx == 3) && g_bh4) ||
		        ((cx == 4) && g_bf4) ||
		        ((cx == 5) && g_bh6) ||
		        ((cx == 6) && g_bh1) ||
		        ((cx == 7) && g_bf1) ||
		        ((cx == 8) && g_bh5));
	} while (test);

	conv(cx, per);
	affper(per);
}

void quelq20(int h, int &per) {
	if (((h >= 0) && (h < 10)) || ((h > 18) && (h < 24)))
		per = chlm();
	else {
		int min = 0, max = 0;
		if ((h > 9) && (h < 12)) {
			min = 3;
			max = 7;
		} else if ((h > 11) && (h < 18)) {
			min = 1;
			max = 2;
		} else if (h == 18) {
			min = 2;
			max = 4;
		}
		choix(min, max, per);
	}
	affper(per);
}


void frap() {
	int j, h, m;

	calch(j, h, m);
	if ((h >= 0) && (h < 8))
		g_crep = 190;
	else {
		if (getRandomNumber(1, 100) > 70)
			g_crep = 190;
		else
			g_crep = 147;
	}
}

void nouvp(int l, int &p) {
	p = 0;
	if (l == 1) {
		if (g_bh1)
			p = 4;
		if (g_bf1)
			p = 2;
	} else if (((l == 2) && (g_bh2)) || ((l == 9) && (g_bh9)))
		p = 128;
	else if (l == 4) {
		if (g_bh4)
			p = 32;
		if (g_bf4)
			p = 16;
	} else if ((l == 5) && (g_bh5))
		p = 1;
	else if ((l == 6) && (g_bh6))
		p = 8;
	else if ((l == 8) && (g_bh8))
		p = 64;
	else if (((l == 3) && (g_bt3)) || ((l == 7) && (g_bt7)))
		p = 9;

	if (p != 9)
		affper(p);
}



void tip(int ip, int &cx) {
	if (ip == 128)
		cx = 1;
	else if (ip == 64)
		cx = 2;
	else if (ip == 32)
		cx = 3;
	else if (ip == 16)
		cx = 4;
	else if (ip == 8)
		cx = 5;
	else if (ip == 4)
		cx = 6;
	else if (ip == 2)
		cx = 7;
	else if (ip == 1)
		cx = 8;
}


void ecfren(int &p, int &rand, int cf, int l) {
	if (l == 0)
		person();
	p = -500;
	rand = 0;
	if (((l == 1) && (!g_bh1) && (!g_bf1)) || ((l == 4) && (!g_bh4) && (!g_bf4)))
		cpl1(p);
	if ((l == 2) && (!g_bh2) && (!g_bh9))
		cpl2(p);
	if (((l == 3) && (!g_bt3)) || ((l == 7) && (!g_bt7)))
		cpl3(p);
	if ((l == 5) && (!g_bh5))
		cpl5(p);
	if (((l == 6) && (!g_bh6)) || ((l == 8) && (!g_bh8)))
		cpl6(p);
	if ((l == 9) && (!g_bh9) && (!g_bh2))
		p = 10;
	if (((l == 2) && (g_bh9)) || ((l == 9) && (g_bh2)))
		p = -400;
	if (p != -500) {
		p = p + cf;
		rand = getRandomNumber(1, 100);
	}
}

void becfren(int l) {
	if ((l == 1) || (l == 4)) {
		int rand = getRandomNumber(1, 2);
		if (l == 1) {
			if (rand == 1)
				g_bh1 = true;
			else
				g_bf1 = true;
		} else { // l == 4
			if (rand == 1)
				g_bh4 = true;
			else
				g_bf4 = true;
		}
	} else if (l == 2)
		g_bh2 = true;
	else if (l == 3)
		g_bt3 = true;
	else if (l == 5)
		g_bh5 = true;
	else if (l == 6)
		g_bh6 = true;
	else if (l == 7)
		g_bt7 = true;
	else if (l == 8)
		g_bh8 = true;
	else if (l == 9)
		g_bh9 = true;
}

/* NIVEAU 10 */
void init_nbrepm() {
	const byte ipm[9] = { 0, 4, 5, 6, 7, 5, 6, 5, 8 };

	for (int idx = 0; idx < 9; ++idx)
		g_nbrepm[idx] = ipm[idx];
}

void phaz(int &rand, int &p, int cf) {
	p += cf;
	rand = getRandomNumber(1, 100);
}

void inzon() {
	copcha();

	g_s._ipre  = false;
	g_s._selectedObjectId = 0;
	g_s._icave = 0;
	g_s._iboul = 0;
	g_s._ibag  = 0;
	g_s._ipuit = 0;
	g_s._ivier = 0;
	g_s._iloic = 136;
	g_s._icryp = 141;
	g_s._conf  = getRandomNumber(4, 10);
	g_s._mlieu = MANOR_FRONT;

	for (int cx = 2; cx <= 6; ++cx)
		g_s._sjer[cx] = chr(0);

	g_s._sjer[1] = chr(113);
	g_s._heure = chr(20);

	for (int cx = 1; cx <= 10; ++cx)
		g_s._pourc[cx] = ' ';

	for (int cx = 1; cx <= 6; ++cx)
		g_s._teauto[cx] = '*';

	for (int cx = 7; cx <= 9; ++cx)
		g_s._teauto[cx] = ' ';

	for (int cx = 10; cx <= 28; ++cx)
		g_s._teauto[cx] = '*';

	for (int cx = 29; cx <= 42; ++cx)
		g_s._teauto[cx] = ' ';

	g_s._teauto[33] = '*';

	for (int cx = 1; cx <= 8; ++cx)
		g_nbrep[cx] = 0;

	init_nbrepm();
}

void dprog() {
	g_li = 21;
	g_jh = 0;
	if (!g_s._ipre)
		g_blo = true;
	g_t = ti1;
	g_mh = readclock();
}

void pl1(int cf) {
	if (((g_li == 1) && (!g_bh1) && (!g_bf1)) || ((g_li == 4) && (!g_bh4) && (!g_bf4))) {
		int p, rand;
		cpl1(p);
		phaz(rand, p, cf);

		if (rand > p)
			person();
		else
			quelq1(g_li);
	}
}

void pl2(int cf) {
	if (!g_bh2) {
		int p, rand;
		cpl2(p);
		phaz(rand, p, cf);

		if (rand > p)
			person();
		else
			quelq2();
	}
}

void pl5(int cf) {
	if (!g_bh5) {
		int p, rand;
		cpl5(p);
		phaz(rand, p, cf);

		if (rand > p)
			person();
		else
			quelq5();
	}
}

void pl6(int cf) {
	if (((g_li == 6) && (!g_bh6)) || ((g_li == 8) && (!g_bh8))) {
		int p, rand;
		cpl6(p);
		phaz(rand, p, cf);

		if (rand > p)
			person();
		else
			quelq6(g_li);
	}
}

void pl9(int cf) {
	if (!g_bh9) {
		cf = -10;
		int p, rand;
		phaz(rand, p, cf);

		if (rand > p)
			person();
		else
			quelq2();
	}
}

void pl10(int cf) {
	int p, h, rand;
	cpl10(p, h);
	phaz(rand, p, cf);

	if (rand > p)
		person();
	else
		quelq10(h, p);
}

void pl11(int cf) {
	int p, h, rand;

	cpl11(p, h);
	phaz(rand, p, cf);
	if (rand > p)
		person();
	else
		quelq11(h, p);
}

void pl12(int cf) {
	int p, rand;

	cpl12(p);
	phaz(rand, p, cf);
	if (rand > p)
		person();
	else
		quelq12(p);
}

void pl13(int cf) {
	int p, rand;

	cpl13(p);
	phaz(rand, p, cf);
	if (rand > p)
		person();
	else
		quelq12(p);
}

void pl15(int cf) {
	int p, rand;

	cpl15(p);
	phaz(rand, p, cf);
	if (rand > p)
		person();
	else
		quelq15(p);
}

void pl20(int cf) {
	int p, h, rand;

	cpl20(p, h);
	phaz(rand, p, cf);
	if (rand > p)
		person();
	else
		quelq20(h, p);
}

void t11(int l11, int &a) {
	int p, rand;

	ecfren(p, rand, g_s._conf, l11);
	g_li = l11;
	if ((l11 > 0) && (l11 < 10)) {
		if (p != -500) {
			if (rand > p) {
				person();
				a = 0;
			} else {
				becfren(g_li);
				nouvp(g_li, a);
			}
		} else
			nouvp(g_li, a);
	}

	if (l11 > 9) {
		if ((l11 > 15) && (l11 != 20) && (l11 != 26))
			person();
		else {
			int h = 0;
			if (l11 == 10)
				cpl10(p, h);
			if (l11 == 11)
				cpl11(p, h);
			if (l11 == 12)
				cpl12(p);
			if ((l11 == 13) || (l11 == 14))
				cpl13(p);
			if ((l11 == 15) || (l11 == 26))
				cpl15(p);
			if (l11 == 20)
				cpl20(p, h);
			p += g_s._conf;
			rand = getRandomNumber(1, 100);
			if (rand > p) {
				person();
				a = 0;
			} else {
				if (l11 == 10)
					quelq10(h, p);
				if (l11 == 11)
					quelq11(h, p);
				if ((l11 == 12) || (l11 == 13) || (l11 == 14))
					quelq12(p);
				if ((l11 == 15) || (l11 == 26))
					quelq15(p);
				if (l11 == 20)
					quelq20(h, p);
				a = p;
			}
		}
	}
}

void cavegre() {
	g_s._conf += 2;
	if (g_s._conf > 69)
		g_s._conf += (g_s._conf / 10);
	clsf3();
	ecrf2();
	ecr3(g_vm->getEngineString(S_SOMEONE_ENTERS));
	int rand = (getRandomNumber(0, 4)) - 2;
	parole(2, rand, 1);

	// The original was doing here a useless loop.
	// It has been removed

	clsf3();
	person();
}

void writetp(Common::String s, int t) {
	if (g_res == 2)
		g_vm->_screenSurface.writeg(s, t);
	else
		g_vm->_screenSurface.writeg(copy(s, 1, 25), t);
}

/**
 * Shows the waiting message when changing scenes.
 * @remarks	Because modern computesr are so much quicker. There's no point in showing
 * a waiting message between scenes.
 */
void messint(int nu) {
	// Method is deprecated
	warning("DEPRECATED: messint");
}

void aniof(int ouf, int num) {
	if ((g_caff == 7) && ((num == 4) || (num == 5)))
		return;
	
	if ((g_caff == 10) && (num == 7))
		num = 6;

	if (g_caff == 12) {
		if (num == 3)
			num = 4;
		else if (num == 4)
			num = 3;
	}

	int ad = adani;
	int offset = animof(ouf, num);

	GfxSurface surface;
	surface.decode(&g_mem[ad * 16 + offset]);
	g_vm->_screenSurface.drawPicture(surface, 0, 12);

	ecrf1();
}

void musique(int so) {
	if (so == 0) {
		/* musik(0) */
		;
	} else if ((g_prebru == 0) && (!g_s._ipre)) {
		parole(10, 1, 1);
		++g_prebru;
	} else {
		bool i = false;
		if ((g_s._mlieu == MOUNTAIN) || (g_s._mlieu == MANOR_FRONT) || (g_s._mlieu == MANOR_BACK)) {
			if (getRandomNumber(1, 3) == 2) {
				parole(9, getRandomNumber(2, 4), 1);
				i = true;
			}
		}
		if (g_s._mlieu == CHAPEL) {
			if (getRandomNumber(1, 2) == 1) {
				parole(8, 1, 1);
				i = true;
			}
		}

		if (g_s._mlieu == WELL) {
			if (getRandomNumber(1, 2) == 2) {
				parole(12, 1, 1);
				i = true;
			}
		}

		if (g_s._mlieu == 23) {
			parole(13, 1, 1);
			i = true;
		}
		if (!i)
			parole(getRandomNumber(1, 17), 1, 2);
	}
}

/* NIVEAU 9 */
void dessin(int ad) {
	if (ad != 0)
		dessine(g_ades, ((ad % 160) * 2), (ad / 160));
	else {
		clsf1();
		if (g_caff > 99) {
			dessine(g_ades, 60, 33);
			g_vm->_screenSurface.drawBox(118, 32, 291, 121, 15);         // Medium box
		} else if (g_caff > 69) {
			dessine(g_ades, 112, 48);           // Heads
			g_vm->_screenSurface.drawBox(222, 47, 155, 91, 15);
		} else {
			dessine(g_ades, 0, 12);
			ecrf1();
			if ((g_caff < 30) || (g_caff > 32)) {
				for (int cx = 1; cx <= 6; ++cx) {
					if (ord(g_touv[cx]) != 0)
						aniof(1, ord(g_touv[cx]));
				}

				if (g_caff == 13) {
					if (g_s._iboul == 141)
						aniof(1, 7);

					if (g_s._ibag == 159)
						aniof(1, 6);
				}
				if ((g_caff == 14) && (g_s._icave == 151))
					aniof(1, 2);

				if ((g_caff == 17) && (g_s._ivier == 143))
					aniof(1, 1);

				if ((g_caff == 24) && (g_s._ipuit != 0))
					aniof(1, 1);
			}
			
			if (g_caff < 26)
				musique(1);
		}
	}
}

} // End of namespace Mortevielle
