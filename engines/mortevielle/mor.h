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

#ifndef MORTEVIELLE_MOR_H
#define MORTEVIELLE_MOR_H

#include "common/str.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

const int kTime1 = 410;
const int kTime2 = 250;

static const int _actionMenu[12] = { OPCODE_NONE,
		OPCODE_SHIDE, OPCODE_ATTACH, OPCODE_FORCE, OPCODE_SLEEP, 
		OPCODE_ENTER, OPCODE_CLOSE,  OPCODE_KNOCK, OPCODE_EAT,
		OPCODE_PLACE, OPCODE_OPEN,   OPCODE_LEAVE
};

/* NIVEAU 10 */
extern void phaz(int &rand, int &p, int cf);
extern int t11(int roomId);
extern void writetp(Common::String s, int t);
extern void aniof(int ouf, int num);
/* NIVEAU 9 */
extern void dessin(int ad);
extern void fenat(char ans);
/* NIVEAU 8 */
extern void afdes();
extern void tkey1(bool d);
/* NIVEAU 7 */
extern void tlu(int af, int ob);
extern void affrep();
/* NIVEAU 6 */
extern void tsort();
extern void st4(int ob);
extern void modinv();
extern void mennor();
extern void premtet();
/* NIVEAU 5 */
extern void ajchai();
extern void ajjer(int ob);
extern void quelquun();
extern void tsuiv();
extern void tfleche();
extern void tcoord(int sx);
extern void st7(int ob);
extern void treg(int ob);
extern void avpoing(int &ob);
extern void rechai(int &ch);
extern int t23coul();
extern void st13(int ob);

extern void sauvecr(int y, int dy);
extern void charecr(int y, int dy);
} // End of namespace Mortevielle
#endif
