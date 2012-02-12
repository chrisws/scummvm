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
#include "mortevielle/parole.h"
#include "mortevielle/sound.h"
#include "mortevielle/mortevielle.h"

namespace Mortevielle {

void spfrac(int wor) {
	c3.rep = (uint)wor >> 12;
	if ((typlec == 0) && (c3.code != 9))
		if (((c3.code > 4) && (c3.val != 20) && ((c3.rep != 3) && (c3.rep != 6) && (c3.rep != 9)) ||
				((c3.code < 5) && ((c3.rep != 19) && (c3.rep != 22) && (c3.rep != 4) && (c3.rep != 9))))) {
			c3.rep = c3.rep + 1;
		}

	c3.freq = ((uint)wor >> 6) & 7;
	c3.acc = ((uint)wor >> 9) & 7;
}

void charg_car() {
	int wor, int_;

	wor = swap(READ_LE_UINT16(&mem[adword + ptr_word]));
	int_ = wor & 0x3f;

	if ((int_ >= 0) && (int_ <= 13)) {
		c3.val = int_;
		c3.code = 5;
	} else if ((int_ >= 14) && (int_ <= 21)) {
		c3.val = int_;
		c3.code = 6;
	} else if ((int_ >= 22) && (int_ <= 47)) {
		int_ = int_ - 22;
		c3.val = int_;
		c3.code = typcon[int_];
	} else if ((int_ >= 48) && (int_ <= 56)) {
		c3.val = int_ - 22;
		c3.code = 4;
	} else {
		switch (int_) {
		case 60 : {
			c3.val = 32;  /*  " "  */
			c3.code = 9;
		}
		break;
		case 61 : {
			c3.val = 46;  /*  "."  */
			c3.code = 9;
		}
		break;
		case 62 : {
			c3.val = 35;  /*  "#"  */
			c3.code = 9;
		}
		break;
		}
	}

	spfrac(wor);
	ptr_word = ptr_word + 2;
}


void entroct(byte o) {
	mem[adtroct * 16 + ptr_oct] = o;
	ptr_oct = ptr_oct + 1;
}

void veracf(byte b) {
	;
}

/* overlay */ void cctable(tablint &t) {
	int k;
	float tb[257];


	tb[0] = 0;
	for (k = 0; k <= 255; k ++) {
		tb[k + 1] = addfix + tb[k];
		t[255 - k] = abs((int)tb[k] + 1);
	}
}

/* overlay */ void regenbruit() {
	int i, j;

	i = offsetb3 + 8590;
	j = 0;
	do {
		t_cph[j] = READ_LE_UINT16(&mem[adbruit3 + i]);
		i = i + 2;
		j = j + 1;
	} while (!(i >= offsetb3 + 8790));
}

/* overlay */   void charge_son() {
	Common::File f;

	if (!f.open("sonmus.mor"))
		error("Missing file - sonmus.mor");
	
	f.read(&mem[0x7414 * 16 + 0], 273);

	g_vm->_soundManager.demus(&mem[0x7414 * 16], &mem[adson * 16], 273);
	f.close();
}

/* overlay */   void charge_phbruit() {
	Common::File f;

	if (!f.open("phbrui.mor"))
		error("Missing file - phbrui.mor");

	for (int i = 1; i <= 3; ++i)
		t_cph[i] = f.readSint16LE();

	f.close();
}

/* overlay */   void charge_bruit() {
	Common::File f;
	int i;

	if (!f.open("bruits"))               //Translation: "noise"
		error("Missing file - bruits");

	f.read(&mem[adbruit * 16 + 0], 250);
	for (i = 0; i <= 19013; i ++) mem[adbruit * 16 + 32000 + i] = mem[adbruit5 + i];
	f.read(&mem[adbruit1 * 16 + offsetb1], 149);

	f.close();
}

/* overlay */   void trait_car() {
	byte d3;
	int d2, i;

	switch (c2.code) {
	case 9 :
		if (c2.val != ord('#'))  for (i = 0; i <= c2.rep; i ++) entroct(c2.val);
		break;

	case 5:
	case 6 : {
		if (c2.code == 6)  d3 = tabdph[(c2.val - 14) << 1];
		else d3 = null;
		if (c1.code >= 5) {
			veracf(c2.acc);
			if (c1.code == 9) {
				entroct(4);
				if (d3 == null)  entroct(c2.val);
				else entroct(d3);
				entroct(22);
			}
		}
		switch (c2.rep) {
		case 0 : {
			entroct(0);
			entroct(c2.val);
			if (d3 == null)  if (c3.code == 9)  entroct(2);
				else entroct(4);
			else if (c3.code == 9)  entroct(0);
			else entroct(1);
		}
		break;
		case 4:
		case 5:
		case 6 : {
			if (c2.rep != 4) {
				i = c2.rep - 5;
				do {
					i = i - 1;
					entroct(0);
					if (d3 == null)  entroct(c2.val);
					else entroct(d3);
					entroct(3);
				} while (!(i < 0));
			}
			if (d3 == null) {
				entroct(4);
				entroct(c2.val);
				entroct(0);
			} else {
				entroct(0);
				entroct(c2.val);
				entroct(3);
			}
		}
		break;
		case 7:
		case 8:
		case 9 : {
			if (c2.rep != 7) {
				i = c2.rep - 8;
				do {
					i = i - 1;
					entroct(0);
					if (d3 == null)  entroct(c2.val);
					else entroct(d3);
					entroct(3);
				} while (!(i < 0));
			}
			if (d3 == null) {
				entroct(0);
				entroct(c2.val);
				entroct(2);
			} else {
				entroct(0);
				entroct(c2.val);
				entroct(0);
			}
		}
		break;
		case 1:
		case 2:
		case 3 : {
			if (c2.rep != 1) {
				i = c2.rep - 2;
				do {
					i = i - 1;
					entroct(0);
					if (d3 == null)  entroct(c2.val);
					else entroct(d3);
					entroct(3);
				} while (!(i < 0));
			}
			entroct(0);
			entroct(c2.val);
			if (c3.code == 9)  entroct(0);
			else entroct(1);
		}
		break;
		}     /*  case  c2.rep  */
	}
	break;

	case 2:
	case 3 : {
		d3 = c2.code + 5; /*  7 ou 8  => voyelle correspondante  */ //Translation: Corresponding vowel
		if (c1.code > 4) {
			veracf(c2.acc);
			if (c1.code == 9) {
				entroct(4);
				entroct(d3);
				entroct(22);
			}
		}
		i = c2.rep;
		if (i != 0) {
			do {
				i = i - 1;
				entroct(0);
				entroct(d3);
				entroct(3);
			} while (!(i <= 0));
		}
		veracf(c3.acc);
		if (c3.code == 6) {
			entroct(4);
			entroct(tabdph[(c3.val - 14) << 1]);
			entroct(c2.val);
		} else {
			entroct(4);
			if (c3.val == 4)  entroct(3);
			else entroct(c3.val);
			entroct(c2.val);
		}
	}
	break;
	case 0:
	case 1 : {
		veracf(c2.acc);
		switch (c3.code) {
		case 2 :
			d2 = 7;
			break;
		case 3 :
			d2 = 8;
			break;
		case 6 :
			d2 = tabdph[(c3.val - 14) << 1];
			break;
		case 5 :
			d2 = c3.val;
			break;
		default:
			d2 = 10;
		}       /*  case  c3.code  */
		d2 = d2 * 26 + c2.val;
		if (tnocon[d2] == 0)  d3 = 2;
		else d3 = 6;
		if (c2.rep >= 5) {
			c2.rep = c2.rep - 5;
			d3 = 8 - d3;       /*  echange 2 et 6  */    //Translation: swap 2 and 6
		}
		if (c2.code == 0) {
			i = c2.rep;
			if (i != 0) {
				do {
					i = i - 1;
					entroct(d3);
					entroct(c2.val);
					entroct(3);
				} while (!(i <= 0));
			}
			entroct(d3);
			entroct(c2.val);
			entroct(4);
		} else {
			entroct(d3);
			entroct(c2.val);
			entroct(3);
			i = c2.rep;
			if (i != 0) {
				do {
					i = i - 1;
					entroct(d3);
					entroct(c2.val);
					entroct(4);
				} while (!(i <= 0));
			}
		}
		if (c3.code == 9) {
			entroct(d3);
			entroct(c2.val);
			entroct(5);
		} else if ((c3.code != 0) && (c3.code != 1) && (c3.code != 4)) {
			veracf(c3.acc);
			switch (c3.code) {
			case 3 :
				d2 = 8;
				break;
			case 6 :
				d2 = tabdph[(c3.val - 14) << 1];
				break;
			case 5 :
				d2 = c3.val;
				break;
			default:
				d2 = 7;
			}     /*  case c3.code  */
			if (d2 == 4)  d2 = 3;
			if (intcon[c2.val] != 0)  c2.val = c2.val + 1;
			if ((c2.val == 17) || (c2.val == 18))  c2.val = 16;
			entroct(4);
			entroct(d2);
			entroct(c2.val);
		}
	}
	break;
	case 4 : {
		veracf(c2.acc);
		i = c2.rep;
		if (i != 0) {
			do {
				i = i - 1;
				entroct(2);
				entroct(c2.val);
				entroct(3);
			} while (!(i <= 0));
		}
		entroct(2);
		entroct(c2.val);
		entroct(4);
		if (c3.code == 9) {
			entroct(2);
			entroct(c2.val);
			entroct(5);
		} else if ((c3.code != 0) && (c3.code != 1) && (c3.code != 4)) {
			veracf(c3.acc);
			switch (c3.code) {
			case 3 :
				d2 = 8;
				break;
			case 6 :
				d2 = tabdph[(c3.val - 14) << 1];
				break;
			case 5 :
				d2 = c3.val;
				break;
			default:
				d2 = 7;
			}     /*  case c3.code  */
			if (d2 == 4)  d2 = 3;
			if (intcon[c2.val] != 0)  c2.val = c2.val + 1;
			entroct(4);
			entroct(d2);
			entroct(tabdbc[((c2.val - 26) << 1) + 1]);
		}
	}
	break;
	}     /* case c2.code  */
}

} // End of namespace Mortevielle
